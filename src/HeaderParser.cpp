
#include "HeaderParser.h"

#include <cctype>
#include <stack>
#include <memory>
#include "types/StructureType.h"
#include "types/EnumType.h"
#include "types/StaticArrayType.h"

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

HeaderParser::HeaderParser(TypeSet *outputTypeSet, const char *headerStart, size_t headerLength, bool parseNamesOnly) : typeSet(outputTypeSet), cur(headerStart), end(headerStart+headerLength), parseNamesOnly(parseNamesOnly) { }

const char *HeaderParser::currentChar() const {
    return cur;
}

void HeaderParser::parse() {
    while (skipWhitespaceAndComments(MULTI_LINE), cur < end)
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

void HeaderParser::parseSection() {
    if (cur < end && *cur == '#') // directive
        return skipDirective();
    if (matchKeyword("namespace"))
        return parseNamespace();
    if (matchKeyword("struct"))
        parseStruct();
    else if (matchKeyword("enum"))
        parseEnum();
    skipSection();
}

void HeaderParser::parseNamespace() {
    skipWhitespaceAndComments(MULTI_LINE);
    std::string namespaceName = readIdentifier();
    if (namespaceName.empty())
        throw Error::INVALID_NAMESPACE_SYNTAX;
    skipWhitespaceAndComments(MULTI_LINE);
    if (matchSymbol(';'))
        return;
    if (!matchSymbol('{'))
        throw Error::INVALID_NAMESPACE_SYNTAX;
    curNamespace.push_back(namespaceName);
    for (skipWhitespaceAndComments(MULTI_LINE); !matchSymbol('}'); skipWhitespaceAndComments(MULTI_LINE)) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        parseSection();
    }
    curNamespace.pop_back();
}

const Type *HeaderParser::parseStruct() {
    skipWhitespaceAndComments(MULTI_LINE);
    std::string structName = readNamespacedIdentifier();
    skipWhitespaceAndComments(MULTI_LINE);
    const char *possibleDeclarationEnd = cur;
    if (matchKeyword("final"))
        skipWhitespaceAndComments(MULTI_LINE);
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not a structure declaration/definition but a variable
        // Rewind cur because "final" is actually variable name
        cur = possibleDeclarationEnd;
        return findType(structName);
    }

    StructureType *structType = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    if (!structName.empty()) {
        std::string fullStructName;
        if (structName.size() >= 2 && structName[0] == ':' && structName[1] == ':')
            fullStructName = structName.substr(2);
        else {
            for (std::vector<std::string>::const_iterator it = curNamespace.begin(); it != curNamespace.end(); ++it)
                fullStructName += *it+"::";
            fullStructName += structName;
        }
        if (Type *type = typeSet->find(fullStructName)) {
            if (!(structType = dynamic_cast<StructureType *>(type)))
                throw Error::TYPE_REDEFINITION;
            if (forwardDeclaration)
                return type;
            if (structType->membersFinalized())
                throw Error::TYPE_REDEFINITION;
        } else {
            std::unique_ptr<StructureType> newType(new StructureType(fullStructName));
            structType = newType.get();
            typeSet->add(fullStructName, std::move(newType));
        }
    }
    if (forwardDeclaration)
        return structType;
    if (structName.empty() && !parseNamesOnly) {
        std::unique_ptr<StructureType> newType(new StructureType());
        structType = newType.get();
        typeSet->addAnonymous(std::move(newType));
    }

    if (matchSymbol(':')) {
        skipWhitespaceAndComments(MULTI_LINE);
        do {
            bool nonPublic = false;
            if (matchKeyword("virtual"))
                skipWhitespaceAndComments(MULTI_LINE);
            if (matchKeyword("public"))
                skipWhitespaceAndComments(MULTI_LINE);
            else if (matchKeyword("private") || matchKeyword("protected")) {
                nonPublic = true;
                skipWhitespaceAndComments(MULTI_LINE);
            }
            matchKeyword("virtual");
            if (const Type *baseType = parseType()) {
                if (!nonPublic && !parseNamesOnly && dynamic_cast<const StructureType *>(baseType)) {
                    if (StructureType *baseStructType = dynamic_cast<StructureType *>(typeSet->find(baseType->name().body())))
                        structType->inheritFrom(baseStructType);
                    // else internal error
                }
            } else {
                // Skip unrecognized type name - modified skipExpression()
                while (skipWhitespaceAndComments(MULTI_LINE), cur < end) {
                    if (*cur == '{' || *cur == ';' || *cur == ',')
                        break;
                    const char *prev = cur;
                    skipBlock(ANY_BRACES_INCLUDING_ANGLED);
                    skipDirective();
                    skipStringLiteral();
                    cur += cur == prev;
                }
            }
            skipWhitespaceAndComments(MULTI_LINE);
        } while (matchSymbol(','));
    }

    if (!matchSymbol('{'))
        throw Error::INVALID_STRUCTURE_SYNTAX;
    if (!structName.empty())
        curNamespace.push_back(structName);
    for (skipWhitespaceAndComments(MULTI_LINE); !matchSymbol('}'); skipWhitespaceAndComments(MULTI_LINE)) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        if (matchKeyword("public") || matchKeyword("protected") || matchKeyword("private")) {
            skipWhitespaceAndComments(MULTI_LINE);
            if (!matchSymbol(':'))
                throw Error::INVALID_STRUCTURE_SYNTAX;
            continue;
        }
        if (
            matchKeyword("void") || matchKeyword("union") || matchKeyword("class") ||
            matchKeyword("const") || matchKeyword("constexpr") || 
            matchKeyword("virtual") || matchKeyword("inline") || matchKeyword("extern") || matchKeyword("explicit") ||
            matchKeyword("typedef") || matchKeyword("operator") || matchKeyword("template") || matchKeyword("friend")
        ) {
            skipSection();
            continue;
        }
        if (matchKeyword("mutable")) // ignore
            skipWhitespaceAndComments(MULTI_LINE);
        bool staticMember = matchKeyword("static");
        if (staticMember)
            skipWhitespaceAndComments(MULTI_LINE);
        if (matchKeyword("mutable"))
            skipWhitespaceAndComments(MULTI_LINE);
        const Type *memberBaseType = nullptr;
        if (matchKeyword("struct")) {
            memberBaseType = parseStruct();
            skipWhitespaceAndComments(MULTI_LINE);
            if (matchSymbol(';')) {
                if (!parseNamesOnly && !staticMember && memberBaseType && memberBaseType->name().body().empty()) {
                    // Special case - non-variable anonymous structure is considered part of parent structure
                    if (!structType->absorb(dynamic_cast<const StructureType *>(memberBaseType)))
                        throw Error::DUPLICATE_STRUCT_MEMBER;
                }
                continue;
            }
        } else if (matchKeyword("enum")) {
            memberBaseType = parseEnum();
            skipWhitespaceAndComments(MULTI_LINE);
            if (matchSymbol(';'))
                continue;
        }
        if (parseNamesOnly || staticMember) {
            skipSection();
            continue;
        }
        if (!memberBaseType)
            memberBaseType = parseType();
        if (memberBaseType) {
            do {
                skipWhitespaceAndComments(MULTI_LINE);
                std::string memberName = readIdentifier();
                if (!memberName.empty()) {
                    std::stack<int> arrayDimensions;
                    skipWhitespaceAndComments(MULTI_LINE);
                    while (matchSymbol('[')) { // TODO
                        arrayDimensions.push(parseArrayLength());
                        skipWhitespaceAndComments(MULTI_LINE);
                        if (!matchSymbol(']'))
                            throw Error::INVALID_ARRAY_SYNTAX;
                        skipWhitespaceAndComments(MULTI_LINE);
                    }
                    const Type *memberType = memberBaseType;
                    while (!arrayDimensions.empty()) {
                        std::string arrayTypeName = memberBaseType->name().body()+'['+std::to_string(arrayDimensions.top())+']'+memberBaseType->name().suffix();
                        if (const Type *arrayType = typeSet->find(arrayTypeName))
                            memberType = arrayType;
                        else {
                            std::unique_ptr<StaticArrayType> newArrayType(new StaticArrayType(memberType, arrayDimensions.top()));
                            memberType = newArrayType.get();
                            typeSet->add(std::move(newArrayType));
                        }
                        arrayDimensions.pop();
                    }
                    if (matchSymbol('=')) {
                        skipExpression();
                        skipWhitespaceAndComments(MULTI_LINE);
                    }
                    if (cur < end && (*cur == ';' || *cur == ',') && !memberType->name().body().empty()) { // TODO anonymous structures / enums
                        if (!structType->addMember(memberName, memberType))
                            throw Error::DUPLICATE_STRUCT_MEMBER;
                    }
                }
            } while (matchSymbol(','));
            if (matchSymbol(';'))
                continue;
        }
        skipSection();
    }
    if (!structName.empty())
        curNamespace.pop_back();
    if (!parseNamesOnly)
        structType->finalizeMembers();
    return structType;
}

const Type *HeaderParser::parseEnum() {
    bool enumClass = false;
    skipWhitespaceAndComments(MULTI_LINE);
    if (matchKeyword("class") || matchKeyword("struct")) {
        enumClass = true;
        skipWhitespaceAndComments(MULTI_LINE);
    }
    std::string enumName = readNamespacedIdentifier();
    skipWhitespaceAndComments(MULTI_LINE);
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not an enum declaration/definition but a variable
        return findType(enumName);
    }

    EnumType *enumType = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    if (!enumName.empty()) {
        std::string fullEnumName;
        if (enumName.size() >= 2 && enumName[0] == ':' && enumName[1] == ':')
            fullEnumName = enumName.substr(2);
        else {
            for (std::vector<std::string>::const_iterator it = curNamespace.begin(); it != curNamespace.end(); ++it)
                fullEnumName += *it+"::";
            fullEnumName += enumName;
        }
        if (Type *type = typeSet->find(fullEnumName)) {
            if (!((enumType = dynamic_cast<EnumType *>(type)) && enumType->isEnumClass() == enumClass))
                throw Error::TYPE_REDEFINITION;
            if (forwardDeclaration)
                return type;
            if (enumType->isFinalized())
                throw Error::TYPE_REDEFINITION;
        } else {
            std::unique_ptr<EnumType> newType(new EnumType(fullEnumName, enumClass));
            enumType = newType.get();
            typeSet->add(fullEnumName, std::move(newType));
        }
    }
    if (forwardDeclaration)
        return enumType;
    if (enumName.empty() && !parseNamesOnly) {
        std::unique_ptr<EnumType> newType(new EnumType());
        enumType = newType.get();
        typeSet->addAnonymous(std::move(newType));
    }

    if (matchSymbol(':')) {
        while ((skipWhitespaceAndComments(MULTI_LINE), cur < end) && !(*cur == ';' || *cur == '{')) {
            const char *prev = cur;
            skipDirective();
            skipStringLiteral();
            cur += cur == prev;
        }
    }
    if (!matchSymbol('{'))
        throw Error::INVALID_ENUM_SYNTAX;
    for (skipWhitespaceAndComments(MULTI_LINE); !matchSymbol('}'); skipWhitespaceAndComments(MULTI_LINE)) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        std::string enumValue = readIdentifier();
        if (enumValue.empty())
            throw Error::INVALID_ENUM_SYNTAX;
        if (!parseNamesOnly)
            enumType->addValue(std::move(enumValue));
        skipWhitespaceAndComments(MULTI_LINE);
        if (matchSymbol('=')) {
            skipExpression();
            skipWhitespaceAndComments(MULTI_LINE);
        }
        matchSymbol(',');
    }
    if (!parseNamesOnly)
        enumType->finalize();
    return enumType;
}

const Type *HeaderParser::parseType() {
    const char *orig = cur;
    skipWhitespaceAndComments(MULTI_LINE);
    if (cur < end && ((isNonSymbol(*cur) && !isdigit(*cur)) || *cur == ':')) {
        std::string typeName = readNamespacedIdentifier();
        if (typeName.empty())
            return nullptr;
        const char *prev = cur;
        skipWhitespaceAndComments(MULTI_LINE);
        // Special case: multi-word fundamental types
        if (typeName == "signed" || typeName == "unsigned" || typeName == "short" || typeName == "long") {
            const char *kwStart = cur;
            if (matchKeyword("short") || matchKeyword("long")) { // Possible 3 word type?
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments(MULTI_LINE);
                kwStart = cur;
            }
            if (matchKeyword("short") || matchKeyword("long")) { // 4 word type??? (unsigned long long int)
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments(MULTI_LINE);
                kwStart = cur;
            }
            if (matchKeyword("int") || matchKeyword("char") || matchKeyword("double") || matchKeyword("float")) {
                typeName += " "+std::string(kwStart, cur);
                prev = cur;
            }
            cur = prev;
            if (Type *type = findType(typeName))
                return type;
        } else if (matchSymbol('<')) { // Template type
            skipWhitespaceAndComments(MULTI_LINE);
            const Type *elementType = nullptr;
            // regular container
            if (ContainerTemplate<> *containerTemplate = findContainerTemplate<>(typeName)) {
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == elementTypeIndex) {
                        if (!(elementType = parseType()))
                            throw Error::INVALID_TYPENAME_SYNTAX;
                    } else
                        skipTemplateArgument();
                    skipWhitespaceAndComments(MULTI_LINE);
                }
                if (elementType) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType))
                        return type;
                }
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
                    if (index == elementTypeIndex) {
                        if (!(elementType = parseType()))
                            throw Error::INVALID_TYPENAME_SYNTAX;
                    } else if (index == arrayLengthIndex)
                        arrayLength = parseArrayLength();
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments(MULTI_LINE);
                }
                if (elementType && arrayLength >= 0) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType, arrayLength))
                        return type;
                }
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
                    if (index == keyTypeIndex) {
                        if (!(keyType = parseType()))
                            throw Error::UNSUPPORTED_TYPE;
                    } else if (index == elementTypeIndex) {
                        if (!(elementType = parseType()))
                            throw Error::INVALID_TYPENAME_SYNTAX;
                    } else
                        skipTemplateArgument();
                    skipWhitespaceAndComments(MULTI_LINE);
                }
                if (keyType && elementType) {
                    if (const Type *type = typeSet->getContainerType(containerTemplate, elementType, keyType))
                        return type;
                }
            }
        } else {
            cur = prev;
            if (Type *type = findType(typeName))
                return type;
        }
    }
    cur = orig;
    return nullptr;
}

int HeaderParser::parseArrayLength() {
    skipWhitespaceAndComments(MULTI_LINE);
    // TODO what if #define ARRAY_SIZE 42 etc.
    if (cur < end && isdigit(*cur)) {
        std::string number;
        do {
            number.push_back(*cur++);
        } while (cur < end && isNonSymbol(*cur));
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
        skipWhitespaceAndComments(MULTI_LINE);
        rootNamespace = true;
    }
    if (cur < end && isNonSymbol(*cur) && !isdigit(*cur)) {
        std::string identifier;
        if (rootNamespace)
            identifier = "::";
        while (true) {
            std::string subIdentifier = readIdentifier();
            if (subIdentifier.empty())
                break;
            identifier += subIdentifier;
            const char *prev = cur;
            skipWhitespaceAndComments(MULTI_LINE);
            if (cur+1 < end && *cur == ':' && *(cur+1) == ':') {
                identifier += "::";
                cur += 2;
                skipWhitespaceAndComments(MULTI_LINE);
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
    if (cur < end && isNonSymbol(*cur) && !isdigit(*cur)) {
        std::string identifier;
        do {
            identifier.push_back(*cur++);
        } while (cur < end && isNonSymbol(*cur));
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

void HeaderParser::skipWhitespaceAndComments(SkipWhitespaceMode mode) {
    const char *prev;
    do {
        prev = cur;
        skipWhitespace(mode);
        skipComment();
    } while (cur != prev);
}

void HeaderParser::skipWhitespace(SkipWhitespaceMode mode) {
    while (cur < end) {
        switch (*cur) {
            case '\n':
                if (mode != MULTI_LINE)
                    return;
                // fallthrough
            case ' ': case '\t': case '\r':
                ++cur;
                break;
            default:
                return;
        }
    }
}

void HeaderParser::skipComment() {
    if (cur+1 < end && *cur == '/') {
        if (*(cur+1) == '/') { // single-line comment
            cur += 2;
            skipLine();
        } else if (*(cur+1) == '*') { // multi-line comment
            cur += 2;
            while (true) {
                if (cur+1 >= end)
                    throw Error::UNEXPECTED_EOF;
                if (*cur == '*' && *(cur+1) == '/') {
                    cur += 2;
                    break;
                }
                ++cur;
            }
        }
    }
}

void HeaderParser::skipStringLiteral() {
    if (cur < end) {
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
                if (cur+1 < end && cur[1] == '"') {
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
                                    return;
                            } while (cur < end && *cur++ == *it++);
                        }
                    }
                    throw Error::UNEXPECTED_EOF;
                }
                // fallthrough
            default:
                return;
        }
        ++cur;
        while (!matchSymbol(endSymbol)) {
            if (cur >= end)
                throw Error::UNEXPECTED_EOF;
            cur += *cur == '\\';
            ++cur;
        }
    }
}

void HeaderParser::skipExpression() {
    // WARNING! If the expression contains a template type constructor with multiple template arguments, this may fail, e.g.
    //     MyMap<Key, Elem>(5) is not really distinguishable from two expressions (MyMap < Key), (Elem > (5)) as if these were variables
    while (skipWhitespaceAndComments(MULTI_LINE), cur < end) {
        if (*cur == ';' || *cur == ',' || *cur == ')' || *cur == ']' || *cur == '}')
            return;
        const char *prev = cur;
        skipDirective();
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
            skipWhitespaceAndComments(MULTI_LINE);
            skipDirective();
            skipBlock(braceTypes);
            skipStringLiteral();
            cur += cur == prev;
        }
    }
}

void HeaderParser::skipSection() {
    while (skipWhitespaceAndComments(MULTI_LINE), cur < end) {
        if (matchSymbol(';'))
            return;
        if (*cur == '{') {
            skipBlock(ANY_BRACES_EXCEPT_ANGLED);
            return;
        }
        const char *prev = cur;
        skipDirective();
        skipBlock(ANY_BRACES_EXCEPT_ANGLED);
        skipStringLiteral();
        cur += cur == prev;
    }
}

void HeaderParser::skipTemplateArgument() {
    while (skipWhitespaceAndComments(MULTI_LINE), cur < end) {
        if (matchSymbol('>') || matchSymbol(',')) {
            --cur;
            return;
        }
        const char *prev = cur;
        skipDirective();
        skipBlock(ANY_BRACES_INCLUDING_ANGLED);
        skipStringLiteral();
        cur += cur == prev;
    }
}

void HeaderParser::skipDirective() {
    if (skipWhitespaceAndComments(SINGLE_LINE), matchSymbol('#'))
        skipLine();
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
            if (subCur < end && isNonSymbol(*subCur))
                return false;
            cur = subCur;
            return true;
        }
    } while (subCur < end && *subCur++ == *keyword++);
    return false;
}

bool HeaderParser::isNonSymbol(char c) {
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
    try {
        return HeaderParser(&typeSet, typeString.c_str(), typeString.size()).parseType();
    } catch (HeaderParser::Error::Type) {
        return nullptr;
    }
}
