
#include "HeaderParser.h"

#include <cstring>
#include <cctype>
#include <stack>
#include <memory>
#include "types/StructureType.h"
#include "types/EnumType.h"
#include "types/TypeAlias.h"
#include "types/StaticArrayType.h"
#include "Generator.h"

HeaderParser::Error::operator HeaderParser::Error::Type() const {
    return type;
}

HeaderParser::Error::operator bool() const {
    return type != OK;
}

const char *HeaderParser::Error::typeString() const {
    switch (type) {
        #define ERROR_TYPE_CASE(x) case x: return #x;
        case OK:
            return "OK";
        FOR_HEADER_PARSER_ERROR_TYPES(ERROR_TYPE_CASE)
    }
    return "";
}

HeaderParser::NamespaceBlockGuard::NamespaceBlockGuard(HeaderParser &parent, const std::string &namespacedName) : parent(parent), outerNamespaceLength(parent.curNamespace.size()), outerActualNamespaceDepth(parent.actualNamespaceDepth), outerUsingNamespacesCount(parent.usingNamespaces.size()) {
    if (namespacedName.empty())
        return;
    size_t pos = 0;
    if (namespacedName[0] == ':') {
        // Stash curNamespace if entering root namespace
        std::swap(outerNamespace, parent.curNamespace);
        pos += 2;
    }
    // Split namespacedName into namespaces and name
    while (pos < namespacedName.size()) {
        size_t start = pos;
        while (pos < namespacedName.size() && namespacedName[pos] != ':')
            ++pos;
        parent.curNamespace.push_back(namespacedName.substr(start, pos-start));
        pos += 2;
    }
}

HeaderParser::NamespaceBlockGuard::~NamespaceBlockGuard() {
    if (outerNamespace.empty())
        parent.curNamespace.resize(outerNamespaceLength);
    else
        parent.curNamespace = std::move(outerNamespace);
    parent.actualNamespaceDepth = outerActualNamespaceDepth;
    parent.usingNamespaces.resize(outerUsingNamespacesCount);
}

HeaderParser::HeaderParser(TypeSet *outputTypeSet, const char *headerStart, size_t headerLength, bool parseNamesOnly) : typeSet(outputTypeSet), cur(headerStart), end(headerStart+headerLength), actualNamespaceDepth(0), parseNamesOnly(parseNamesOnly) { }

const char *HeaderParser::currentChar() const {
    return cur;
}

void HeaderParser::parse() {
    while (skipWhitespaceAndComments(), cur < end)
        parseSection();
}

Type *HeaderParser::findType(const std::string &name) {
    if (name.empty())
        return nullptr;
    if (name.size() >= 2 && name[0] == ':' && name[1] == ':')
        return typeSet->find(name.substr(2));
    for (int i = (int) curNamespace.size(); i >= 0; --i) {
        std::string fullName;
        for (int j = 0; j < i; ++j)
            fullName += curNamespace[j]+"::";
        fullName += name;
        if (Type *type = typeSet->find(fullName))
            return type;
    }
    for (const std::string &ns : usingNamespaces)
        if (Type *type = typeSet->find(ns+"::"+name))
            return type;
    return nullptr;
}

std::string HeaderParser::fullTypeName(const std::string &baseName) const {
    if (baseName.size() >= 2 && baseName[0] == ':' && baseName[1] == ':')
        return baseName.substr(2);
    std::string prefix;
    for (const std::string &ns : curNamespace)
        prefix += ns+"::";
    return prefix+baseName;
}

void HeaderParser::parseSection() {
    if (matchKeyword("namespace"))
        return parseNamespace();
    if (matchKeyword("using"))
        return parseUsing();
    if (matchKeyword("typedef"))
        return parseTypedef();
    if (matchKeyword("struct"))
        parseStruct(false);
    else if (matchKeyword("class"))
        parseStruct(true);
    else if (matchKeyword("enum"))
        parseEnum();
    skipSection();
}

void HeaderParser::parseNamespace() {
    skipWhitespaceAndComments();
    std::string namespaceName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    if (matchSymbol(';'))
        return;
    if (!matchSymbol('{'))
        throw Error::INVALID_NAMESPACE_SYNTAX;
    NamespaceBlockGuard namespaceBlockGuard(*this, namespaceName);
    actualNamespaceDepth = curNamespace.size();
    for (skipWhitespaceAndComments(); !matchSymbol('}'); skipWhitespaceAndComments()) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        parseSection();
    }
}

static TypeAlias *createTypeAlias(TypeSet &typeSet, const std::string &fullAliasName) {
    TypeAlias *typeAlias = nullptr;
    if (Type *type = typeSet.find(fullAliasName)) {
        if (!(typeAlias = type->typeAlias()))
            throw HeaderParser::Error::TYPE_REDEFINITION;
    } else {
        std::unique_ptr<TypeAlias> newType(new TypeAlias(Generator::safeName(fullAliasName)));
        typeAlias = newType.get();
        typeSet.add(fullAliasName, std::move(newType));
    }
    return typeAlias;
}

void HeaderParser::parseUsing() {
    skipWhitespaceAndComments();
    if (matchKeyword("namespace")) {
        skipWhitespaceAndComments();
        std::string namespaceName = readNamespacedIdentifier();
        if (!namespaceName.empty()) {
            if (namespaceName.size() > 2 && namespaceName[0] == ':' && namespaceName[1] == ':')
                usingNamespaces.push_back(namespaceName.substr(2));
            else {
                for (int i = (int) actualNamespaceDepth; i >= 0; --i) {
                    std::string fullNamespaceName;
                    for (int j = 0; j < i; ++j)
                        fullNamespaceName += curNamespace[j]+"::";
                    fullNamespaceName += namespaceName;
                    usingNamespaces.push_back(std::move(fullNamespaceName));
                }
            }
        }
        return skipSection();
    }
    std::string aliasName = readNamespacedIdentifier();
    if (aliasName.empty())
        return skipSection();
    skipWhitespaceAndComments();
    if (matchSymbol(';')) {
        // using ns::type;
        size_t lastSepPos = aliasName.rfind("::");
        if (lastSepPos != std::string::npos && lastSepPos > 0) {
            if (const Type *aliasedType = findType(aliasName)) {
                TypeAlias *typeAlias = createTypeAlias(*typeSet, fullTypeName(aliasName.substr(lastSepPos+2)));
                if (!typeAlias->finalize(aliasedType))
                    throw Error::CYCLIC_TYPE_ALIAS;
            }
        }
        return;
    }
    if (matchSymbol('=')) {
        // using x = y;
        TypeAlias *typeAlias = createTypeAlias(*typeSet, fullTypeName(aliasName));
        skipWhitespaceAndComments();
        const Type *aliasedType = nullptr;
        if (matchKeyword("struct"))
            aliasedType = parseStruct(false);
        else if (matchKeyword("class")) {
            parseStruct(true);
            return skipSection();
        } else if (matchKeyword("enum"))
            aliasedType = parseEnum();
        else if (matchKeyword("const") || matchKeyword("volatile"))
            return skipSection();
        else if (!parseNamesOnly)
            aliasedType = tryParseType();
        if (aliasedType && (aliasedType = tryParseArrayTypeSuffix(aliasedType))) {
            skipWhitespaceAndComments();
            if (matchSymbol(';')) {
                if (!typeAlias->finalize(aliasedType))
                    throw Error::CYCLIC_TYPE_ALIAS;
                return;
            }
        }
    }
    skipSection();
}

void HeaderParser::parseTypedef() {
    skipWhitespaceAndComments();
    const Type *baseAliasedType = nullptr;
    Type *unnamedType = nullptr;
    if (matchKeyword("struct"))
        baseAliasedType = unnamedType = parseStruct(false);
    else if (matchKeyword("class")) {
        parseStruct(true);
        return skipSection();
    } else if (matchKeyword("enum"))
        baseAliasedType = unnamedType = parseEnum();
    else if (matchKeyword("const") || matchKeyword("volatile"))
        return skipSection();
    else {
        const char *orig = cur;
        try {
            baseAliasedType = parseType();
        } catch (Error::Type) {
            cur = orig;
            return skipSection();
        }
    }
    skipWhitespaceAndComments();
    if (matchKeyword("const") || matchKeyword("volatile"))
        return skipSection();
    do {
        skipWhitespaceAndComments();
        if (matchSymbol('*') || matchSymbol('&')) {
            skipExpression();
            continue;
        }
        std::string aliasName = readNamespacedIdentifier();
        if (!aliasName.empty()) {
            TypeAlias *typeAlias = nullptr;
            std::string fullAliasName = fullTypeName(aliasName);
            if (Type *type = typeSet->find(fullAliasName)) {
                if (!(typeAlias = type->typeAlias()) && type != baseAliasedType)
                    throw Error::TYPE_REDEFINITION;
            } else {
                std::unique_ptr<TypeAlias> newType(new TypeAlias(Generator::safeName(fullAliasName)));
                typeAlias = newType.get();
                typeSet->add(fullAliasName, std::move(newType));
            }
            if (const Type *aliasedType = tryParseArrayTypeSuffix(baseAliasedType)) {
                if (!typeAlias) { // typeAlias is null only if baseAliasedType == type (typedef struct X { } X)
                    if (aliasedType != baseAliasedType)
                        throw Error::TYPE_REDEFINITION; // redefinition of X from struct X to struct X[]
                } else if (!parseNamesOnly) {
                    if (aliasedType == baseAliasedType && unnamedType) {
                        unnamedType->rename(typeAlias->name());
                        unnamedType = nullptr;
                    }
                    if (!typeAlias->finalize(aliasedType))
                        throw Error::CYCLIC_TYPE_ALIAS;
                }
            }
        }
        skipWhitespaceAndComments();
    } while (matchSymbol(','));
    if (!matchSymbol(';'))
        skipSection();
}

Type *HeaderParser::parseStruct(bool isClass) {
    skipWhitespaceAndComments();
    std::string structName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    const char *possibleDeclarationEnd = cur;
    if (matchKeyword("final"))
        skipWhitespaceAndComments();
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not a structure declaration/definition but a variable
        // Rewind cur because "final" is actually variable name
        cur = possibleDeclarationEnd;
        return findType(structName);
    }

    StructureType *structType = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    if (!isClass) {
        if (!structName.empty()) {
            std::string fullStructName = fullTypeName(structName);
            if (Type *type = typeSet->find(fullStructName)) {
                if (!(structType = type->incompleteStructureType()))
                    throw Error::TYPE_REDEFINITION;
                if (forwardDeclaration)
                    return type;
            } else {
                std::unique_ptr<StructureType> newType(new StructureType(Generator::safeName(fullStructName)));
                structType = newType.get();
                typeSet->add(fullStructName, std::move(newType));
            }
        }
        if (forwardDeclaration)
            return structType;
        if (structName.empty() && !parseNamesOnly)
            structType = typeSet->newUnnamedStruct();
    } else if (forwardDeclaration)
        return nullptr;

    if (matchSymbol(':')) {
        skipWhitespaceAndComments();
        do {
            bool nonPublic = isClass;
            if (matchKeyword("virtual"))
                skipWhitespaceAndComments();
            if (matchKeyword("public")) {
                nonPublic = false;
                skipWhitespaceAndComments();
            } else if (matchKeyword("private") || matchKeyword("protected")) {
                nonPublic = true;
                skipWhitespaceAndComments();
            }
            matchKeyword("virtual");
            if (const Type *baseType = tryParseType()) {
                if (!nonPublic && !parseNamesOnly && !isClass)
                    structType->inheritFrom(baseType);
            } else {
                // Skip unrecognized type name - modified skipExpression()
                while (skipWhitespaceAndComments(), cur < end) {
                    if (*cur == '{' || *cur == ';' || *cur == ',')
                        break;
                    const char *prev = cur;
                    skipBlock(ANY_BRACES_INCLUDING_ANGLED);
                    skipStringLiteral();
                    cur += cur == prev;
                }
            }
            skipWhitespaceAndComments();
        } while (matchSymbol(','));
    }

    if (!matchSymbol('{'))
        throw Error::INVALID_STRUCTURE_SYNTAX;
    NamespaceBlockGuard namespaceBlockGuard(*this, structName);
    for (skipWhitespaceAndComments(); !matchSymbol('}'); skipWhitespaceAndComments()) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        if (matchKeyword("public") || matchKeyword("protected") || matchKeyword("private")) {
            skipWhitespaceAndComments();
            if (!matchSymbol(':'))
                throw Error::INVALID_STRUCTURE_SYNTAX;
            continue;
        }
        if (
            matchKeyword("void") || matchKeyword("union") ||
            matchKeyword("const") || matchKeyword("volatile") || matchKeyword("extern") ||
            matchKeyword("explicit") || matchKeyword("inline") || matchKeyword("constexpr") ||
            matchKeyword("virtual") || matchKeyword("friend") ||
            matchKeyword("template") || matchKeyword("operator")
        ) {
            skipSection();
            continue;
        }
        if (matchKeyword("mutable")) // ignore
            skipWhitespaceAndComments();
        bool staticMember = matchKeyword("static");
        if (staticMember)
            skipWhitespaceAndComments();
        if (matchKeyword("mutable"))
            skipWhitespaceAndComments();

        const Type *memberBaseType = nullptr;
        bool nestedType = true;
        if (matchKeyword("using")) {
            parseUsing();
            continue;
        } else if (matchKeyword("typedef")) {
            parseTypedef();
            continue;
        } else if (matchKeyword("struct")) {
            memberBaseType = parseStruct(false);
            skipWhitespaceAndComments();
            if (matchSymbol(';')) {
                if (!parseNamesOnly && !isClass && !staticMember && memberBaseType && memberBaseType->name().substance() == TypeName::VIRTUAL) {
                    // Special case - non-variable anonymous structure is considered part of parent structure
                    if (!structType->absorb(memberBaseType->structureType()))
                        throw Error::DUPLICATE_STRUCT_MEMBER;
                }
                continue;
            }
        } else if (matchKeyword("class")) {
            parseStruct(true);
            skipSection();
            continue;
        } else if (matchKeyword("enum")) {
            memberBaseType = parseEnum();
            skipWhitespaceAndComments();
            if (matchSymbol(';'))
                continue;
        } else
            nestedType = false;

        if (parseNamesOnly || isClass || staticMember) {
            skipSection();
            continue;
        }

        if (!nestedType)
            memberBaseType = tryParseType();
        skipWhitespaceAndComments();
        if (memberBaseType && !(matchKeyword("const") || matchKeyword("volatile") || matchKeyword("operator"))) {
            do {
                skipWhitespaceAndComments();
                if (matchSymbol('*') || matchSymbol('&')) {
                    skipExpression();
                    continue;
                }
                std::string memberName = readIdentifier();
                if (!memberName.empty()) {
                    const Type *memberType = tryParseArrayTypeSuffix(memberBaseType);
                    skipWhitespaceAndComments();
                    if (matchSymbol('=')) {
                        skipExpression();
                        skipWhitespaceAndComments();
                    }
                    if (memberType && cur < end && (*cur == ';' || *cur == ',')) {
                        if (!structType->addMember(memberType, memberName))
                            throw Error::DUPLICATE_STRUCT_MEMBER;
                    }
                }
            } while (matchSymbol(','));
            if (matchSymbol(';'))
                continue;
        }
        skipSection();
    }
    if (!parseNamesOnly && !isClass)
        structType->completeMembers();
    return structType;
}

Type *HeaderParser::parseEnum() {
    bool enumClass = false;
    skipWhitespaceAndComments();
    if (matchKeyword("class") || matchKeyword("struct")) {
        enumClass = true;
        skipWhitespaceAndComments();
    }
    std::string enumName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not an enum declaration/definition but a variable
        return findType(enumName);
    }

    EnumType *enumType = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    std::string enumNamespace;
    for (std::vector<std::string>::const_iterator it = curNamespace.begin(); it != curNamespace.end(); ++it)
        enumNamespace += *it+"::";
    if (!enumName.empty()) {
        std::string fullEnumName;
        if (enumName.size() >= 2 && enumName[0] == ':' && enumName[1] == ':') {
            fullEnumName = enumName.substr(2);
            enumNamespace.clear();
        } else
            fullEnumName = enumNamespace+enumName;
        // fullEnumName must equal fullTypeName(enumName)
        if (Type *type = typeSet->find(fullEnumName)) {
            if (!((enumType = type->incompleteEnumType()) && enumType->isEnumClass() == enumClass))
                throw Error::TYPE_REDEFINITION;
            if (forwardDeclaration)
                return type;
        } else {
            std::unique_ptr<EnumType> newType(new EnumType(enumClass, enumNamespace, Generator::safeName(fullEnumName)));
            enumType = newType.get();
            typeSet->add(fullEnumName, std::move(newType));
        }
    }
    if (forwardDeclaration)
        return enumType;
    if (enumName.empty() && !parseNamesOnly)
        enumType = typeSet->newUnnamedEnum(enumClass, enumNamespace);

    if (matchSymbol(':')) {
        while ((skipWhitespaceAndComments(), cur < end) && !(*cur == ';' || *cur == '{')) {
            const char *prev = cur;
            skipStringLiteral();
            cur += cur == prev;
        }
    }
    if (!matchSymbol('{'))
        throw Error::INVALID_ENUM_SYNTAX;
    for (skipWhitespaceAndComments(); !matchSymbol('}'); skipWhitespaceAndComments()) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        std::string enumValue = readIdentifier();
        if (enumValue.empty())
            throw Error::INVALID_ENUM_SYNTAX;
        if (!parseNamesOnly)
            enumType->addValue(std::move(enumValue));
        skipWhitespaceAndComments();
        if (matchSymbol('=')) {
            skipExpression();
            skipWhitespaceAndComments();
        }
        matchSymbol(',');
    }
    if (!parseNamesOnly)
        enumType->completeValues();
    return enumType;
}

const Type *HeaderParser::tryParseType() {
    const char *orig = cur;
    try {
        return parseType();
    } catch (Error::Type) {
        cur = orig;
        return nullptr;
    }
}

const Type *HeaderParser::parseType() {
    const char *orig = cur;
    skipWhitespaceAndComments();
    if (cur < end && ((isWordChar(*cur) && !isdigit(*cur)) || *cur == ':')) {
        std::string typeName = readNamespacedIdentifier();
        if (typeName.empty())
            return nullptr;
        const char *prev = cur;
        skipWhitespaceAndComments();
        // Special case: multi-word fundamental types
        if (typeName == "signed" || typeName == "unsigned" || typeName == "short" || typeName == "long") {
            const char *kwStart = cur;
            if (matchKeyword("short") || matchKeyword("long")) { // Possible 3 word type?
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments();
                kwStart = cur;
            }
            if (matchKeyword("short") || matchKeyword("long")) { // 4 word type??? (unsigned long long int)
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments();
                kwStart = cur;
            }
            if (matchKeyword("int") || matchKeyword("char") || matchKeyword("double") || matchKeyword("float")) {
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
            }
            cur = prev;
            return findType(typeName);
        } else if (matchSymbol('<')) { // Template type
            skipWhitespaceAndComments();
            const Type *elementType = nullptr;
            // regular container
            if (ContainerTemplate<> *containerTemplate = findContainerTemplate<>(typeName)) {
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == elementTypeIndex)
                        elementType = parseType();
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (elementType) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType))
                        return type;
                }
                return nullptr;
            }
            // static array container
            else if (ContainerTemplate<int> *containerTemplate = findContainerTemplate<int>(typeName)) {
                int arrayLength = -1;
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                int arrayLengthIndex = containerTemplate->templateArgIndex('N');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == elementTypeIndex)
                        elementType = parseType();
                    else if (index == arrayLengthIndex)
                        arrayLength = parseArrayLength();
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (elementType && arrayLength >= 0) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType, arrayLength))
                        return type;
                }
                return nullptr;
            }
            // object map container
            else if (ContainerTemplate<const Type *> *containerTemplate = findContainerTemplate<const Type *>(typeName)) {
                const Type *keyType = nullptr;
                int keyTypeIndex = containerTemplate->templateArgIndex('K');
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == keyTypeIndex)
                        keyType = parseType();
                    else if (index == elementTypeIndex)
                        elementType = parseType();
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (keyType && elementType) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType, keyType))
                        return type;
                }
                return nullptr;
            }
        } else {
            cur = prev;
            return findType(typeName);
        }
    }
    cur = orig;
    return nullptr;
}

const Type *HeaderParser::tryParseArrayTypeSuffix(const Type *elemType) {
    skipWhitespaceAndComments();
    if (elemType) {
        const char *orig = cur;
        try {
            std::stack<int> arrayDimensions;
            while (matchSymbol('[')) {
                arrayDimensions.push(parseArrayLength());
                skipWhitespaceAndComments();
                if (!matchSymbol(']'))
                    throw Error::INVALID_ARRAY_SYNTAX;
                skipWhitespaceAndComments();
            }
            const Type *type = elemType;
            while (!arrayDimensions.empty()) {
                std::string arrayTypeName = elemType->name().body()+'['+std::to_string(arrayDimensions.top())+']'+elemType->name().suffix();
                if (const Type *arrayType = typeSet->find(arrayTypeName))
                    type = arrayType;
                else {
                    std::unique_ptr<StaticArrayType> newArrayType(new StaticArrayType(type, arrayDimensions.top()));
                    type = newArrayType.get();
                    typeSet->add(std::move(newArrayType));
                }
                arrayDimensions.pop();
            }
            return type;
        } catch (Error::Type) {
            cur = orig;
        }
    }
    // Skip array dimensions
    while (matchSymbol('[')) {
        skipExpression();
        skipWhitespaceAndComments();
        if (!matchSymbol(']'))
            throw Error::INVALID_ARRAY_SYNTAX;
    }
    return nullptr;
}

int HeaderParser::parseArrayLength() {
    skipWhitespaceAndComments();
    // TODO what if #define ARRAY_SIZE 42 etc.
    if (cur < end && isdigit(*cur)) {
        std::string number;
        do {
            number.push_back(*cur++);
        } while (cur < end && isWordChar(*cur));
        int arrayLength = 0;
        if (sscanf(number.c_str(), "%i", &arrayLength) == 1)
            return arrayLength;
    }
    throw Error::INVALID_ARRAY_SYNTAX;
}

std::string HeaderParser::readNamespacedIdentifier() {
    const char *orig = cur;
    bool rootNamespace = false;
    if (cur+1 < end && *cur == ':' && *(cur+1) == ':') {
        cur += 2;
        skipWhitespaceAndComments();
        rootNamespace = true;
    }
    if (cur < end && isWordChar(*cur) && !isdigit(*cur)) {
        std::string identifier;
        if (rootNamespace)
            identifier = "::";
        while (true) {
            std::string subIdentifier = readIdentifier();
            if (subIdentifier.empty())
                break;
            identifier += subIdentifier;
            const char *prev = cur;
            skipWhitespaceAndComments();
            if (cur+1 < end && *cur == ':' && *(cur+1) == ':') {
                identifier += "::";
                cur += 2;
                skipWhitespaceAndComments();
            } else {
                cur = prev;
                return identifier;
            }
        }
    }
    cur = orig;
    return std::string();
}

std::string HeaderParser::readIdentifier() {
    if (cur < end && isWordChar(*cur) && !isdigit(*cur)) {
        std::string identifier;
        do {
            identifier.push_back(*cur++);
        } while (cur < end && isWordChar(*cur));
        return identifier;
    }
    return std::string();
}

void HeaderParser::skipLine() {
    bool escaped = false;
    while (cur < end) {
        switch (*cur++) {
            case '\n':
                if (!escaped)
                    return;
                // fallthrough
            default:
                escaped = false;
                // fallthrough
            case ' ': case '\t': case '\r':
                break;
            case '\\':
                escaped = true;
        }
    }
}

void HeaderParser::skipWhitespaceAndComments() {
    while (cur < end) {
        switch (*cur) {
            case ' ': case '\t': case '\n': case '\r':
                ++cur;
                continue;
            case '#':
                // Skip directive - TODO parse #define
                skipLine();
                // TODO does not correctly handle the following edge case:
                //     #define PI /*
                //         */ 3.14
                continue;
            case '/':
                if (skipComment())
                    continue;
                // fallthrough
            default:
                return;
        }
    }
}

bool HeaderParser::skipComment() {
    if (cur+1 < end && *cur == '/') {
        switch (*(cur+1)) {
            case '/': // Single-line comment
                skipLine();
                return true;
            case '*': // Multi-line comment
                ++cur;
                do {
                    ++cur;
                    if (cur+1 >= end)
                        throw Error::UNEXPECTED_EOF;
                } while (!(*cur == '*' && *(cur+1) == '/'));
                cur += 2;
                return true;
        }
    }
    return false;
}

bool HeaderParser::skipStringLiteral() {
    if (cur >= end)
        return false;
    char endSymbol = '\0';
    switch (*cur) {
        case '"':
            endSymbol = '"';
            break;
        case '\'':
            endSymbol = '\'';
            break;
        case 'R':
            // Raw string literal
            if (cur+1 < end && *(cur+1) == '"') {
                cur += 2;
                std::string delimiter;
                delimiter.reserve(17);
                while (cur < end && *cur != '(')
                    delimiter.push_back(*cur++);
                delimiter.push_back('"');
                if (cur >= end)
                    throw Error::UNEXPECTED_EOF;
                ++cur;
                while (cur < end) {
                    if (*cur++ == ')') {
                        std::string::const_iterator it = delimiter.begin();
                        do {
                            if (it == delimiter.end())
                                return true;
                        } while (cur < end && *cur++ == *it++);
                    }
                }
                throw Error::UNEXPECTED_EOF;
            }
            // fallthrough
        default:
            return false;
    }
    ++cur;
    while (!matchSymbol(endSymbol)) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        cur += *cur == '\\';
        ++cur;
    }
    return true;
}

void HeaderParser::skipExpression() {
    // WARNING! If the expression contains a template type constructor with multiple template arguments, this may fail, e.g.
    //     MyMap<Key, Elem>(5) is not really distinguishable from two expressions (MyMap < Key), (Elem > (5)) as if these were variables
    while (skipWhitespaceAndComments(), cur < end) {
        if (*cur == ';' || *cur == ',' || *cur == ')' || *cur == ']' || *cur == '}')
            return;
        const char *prev = cur;
        skipBlock(ANY_BRACES_EXCEPT_ANGLED);
        skipStringLiteral();
        cur += cur == prev;
    }
}

void HeaderParser::skipBlock(BraceTypes braceTypes) {
    if (cur < end) {
        char endSymbol = '\0';
        switch (*cur) {
            case '(':
                endSymbol = ')';
                break;
            case '[':
                endSymbol = ']';
                break;
            case '{':
                endSymbol = '}';
                break;
            case '<':
                if (braceTypes != ANY_BRACES_INCLUDING_ANGLED)
                    return;
                endSymbol = '>';
                break;
            default:
                return;
        }
        ++cur;
        while (!matchSymbol(endSymbol)) {
            if (cur >= end)
                throw Error::UNEXPECTED_EOF;
            const char *prev = cur;
            skipWhitespaceAndComments();
            skipBlock(braceTypes);
            skipStringLiteral();
            cur += cur == prev;
        }
    }
}

void HeaderParser::skipSection() {
    while (skipWhitespaceAndComments(), cur < end) {
        if (matchSymbol(';'))
            return;
        if (*cur == '{') {
            skipBlock(ANY_BRACES_EXCEPT_ANGLED);
            return;
        }
        const char *prev = cur;
        skipBlock(ANY_BRACES_EXCEPT_ANGLED);
        skipStringLiteral();
        cur += cur == prev;
    }
}

void HeaderParser::skipTemplateArgument() {
    while (skipWhitespaceAndComments(), cur < end) {
        if (matchSymbol('>') || matchSymbol(',')) {
            --cur;
            return;
        }
        const char *prev = cur;
        skipBlock(ANY_BRACES_INCLUDING_ANGLED);
        skipStringLiteral();
        cur += cur == prev;
    }
}

bool HeaderParser::matchSymbol(char s) {
    if (cur < end && *cur == s) {
        ++cur;
        return true;
    }
    return false;
}

bool HeaderParser::matchKeyword(const char *keyword) {
    const char *subCur = cur;
    do {
        if (!*keyword) {
            if (subCur < end && isWordChar(*subCur))
                return false;
            cur = subCur;
            return true;
        }
    } while (subCur < end && *subCur++ == *keyword++);
    return false;
}

bool HeaderParser::isWordChar(char c) {
    return isalnum(c) || c == '_' || c&0x80;
}

HeaderParser::Error preparseHeader(TypeSet &outputTypeSet, const std::string &headerString) {
    return parseHeader(outputTypeSet, headerString, true);
}

HeaderParser::Error parseHeader(TypeSet &outputTypeSet, const std::string &headerString, bool parseNamesOnly) {
    const char *headerStart = headerString.c_str();
    HeaderParser parser(&outputTypeSet, headerStart, headerString.size(), parseNamesOnly);
    HeaderParser::Error::Type parseErrorType = HeaderParser::Error::OK;
    try {
        parser.parse();
    } catch (HeaderParser::Error::Type errorType) {
        parseErrorType = errorType;
    }
    int position = int(parser.currentChar()-headerStart);
    return HeaderParser::Error(parseErrorType, position);
}

const Type *parseType(TypeSet &typeSet, const std::string &typeString) {
    return HeaderParser(&typeSet, typeString.c_str(), typeString.size()).tryParseType();
}
