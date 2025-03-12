
#include "HeaderParser.h"

#include <cstring>
#include <cctype>
#include <cassert>
#include <stack>
#include <memory>
#include "types/StructureType.h"
#include "types/EnumType.h"
#include "types/StaticArrayType.h"
#include "Generator.h"

const Type *parseType(TypeSet &typeSet, const std::string &typeString) {
    HeaderParser::Pass parserPass(HeaderParser::Pass::FINAL);
    return symbolType(HeaderParser(parserPass, typeSet, typeString.c_str(), typeString.size()).tryParseType());
}

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

HeaderParser::Pass::Pass(Stage initialStage) : stage(initialStage), cur(), prev() { }

void HeaderParser::Pass::operator++() {
    if (stage >= FINAL)
        stage = NONE;
    else if (cur.unresolvedNestedTypes == 0 && cur.unresolvedNamespacibleTypes == 0)
        stage = FINAL;
    else if (stage == INITIAL)
        stage = NAMES_ONLY;
    else if (cur.unresolvedNestedTypes >= prev.unresolvedNestedTypes && cur.unresolvedNamespacibleTypes >= prev.unresolvedNamespacibleTypes) {
        if (cur.unresolvedNestedTypes == 0 || stage == NAMES_ONLY_FALLBACK)
            stage = FINAL;
        else
            stage = NAMES_ONLY_FALLBACK;
    }
    prev = cur;
    cur.unresolvedNestedTypes = 0;
    cur.unresolvedNamespacibleTypes = 0;
}

HeaderParser::Pass::operator bool() const {
    return stage != NONE;
}

bool HeaderParser::Pass::namesOnly() const {
    return stage <= NAMES_ONLY_FALLBACK;
}

void HeaderParser::Pass::unresolvedNestedTypeEncountered() {
    ++cur.unresolvedNestedTypes;
}

void HeaderParser::Pass::unresolvedTypeAliasEncountered() {
    ++cur.unresolvedNamespacibleTypes;
}

void HeaderParser::Pass::unresolvedBaseTypeEncountered() {
    ++cur.unresolvedNamespacibleTypes;
}

HeaderParser::NamespaceBlockGuard::NamespaceBlockGuard(HeaderParser &parent, QualifiedName::Ref namespaceName) : parent(parent), outerNamespace(parent.curNamespace), outerNamespaceNamesLength(parent.namespaceNames.size()), outerUsingNamespacesCount(parent.usingNamespaces.size()), outerWithinStruct(parent.withinStruct) {
    if (!namespaceName)
        return;
    size_t pos = 0;
    if (namespaceName.isAbsolute()) {
        parent.curNamespace = &parent.typeSet->root();
        // Stash namespaceNames if entering root namespace
        std::swap(outerNamespaceNames, parent.namespaceNames);
        pos += 2;
    }
    // Iterate over namespaceName's sub-names (this is only done because of parent.namespaceNames which is planned to be removed)
    while (namespaceName) {
        if (SymbolPtr symbol = parent.curNamespace->establishNamespace(QualifiedName::Ref(namespaceName.prefix()))) {
            parent.curNamespace = symbol->ns.get();
            parent.namespaceNames.push_back(namespaceName.prefix().string());
            namespaceName = namespaceName.exceptPrefix();
        } else
            throw Error::UNEXPECTED_CODE_PATH;
    }
}

HeaderParser::NamespaceBlockGuard::~NamespaceBlockGuard() {
    parent.withinStruct = outerWithinStruct;
    if (outerNamespaceNames.empty())
        parent.namespaceNames.resize(outerNamespaceNamesLength);
    else
        parent.namespaceNames = std::move(outerNamespaceNames);
    parent.usingNamespaces.resize(outerUsingNamespacesCount);
    parent.curNamespace = outerNamespace;
}

HeaderParser::HeaderParser(Pass &pass, TypeSet &outputTypeSet, const char *headerStart, size_t headerLength) : pass(pass), typeSet(&outputTypeSet), cur(headerStart), end(headerStart+headerLength), curNamespace(&outputTypeSet.root()), withinStruct(false) { }

const char *HeaderParser::currentChar() const {
    return cur;
}

void HeaderParser::parse() {
    while (skipWhitespaceAndComments(), cur < end) {
        const char *prev = cur;
        parseSection();
        cur += cur == prev;
    }
}

std::string HeaderParser::fullTypeName(QualifiedName::Ref baseName) const {
    if (baseName.isAbsolute())
        return baseName.exceptAbsolute().string();
    std::string prefix;
    for (const std::string &ns : namespaceNames)
        prefix += ns+"::";
    return prefix+baseName.string();
}

SymbolPtr HeaderParser::newTypeSymbol(QualifiedName::Ref newTypeName, Namespace **newTypeNamespace) {
    if (!newTypeName)
        return nullptr;
    Namespace *targetNamespace = curNamespace;
    if (newTypeName.isAbsolute()) {
        if (withinStruct) // struct X { struct ::Y::Z { } }; is invalid
            return nullptr;
        targetNamespace = &typeSet->root();
        newTypeName = newTypeName.exceptAbsolute();
    }
    while (!newTypeName.isUnqualified()) {
        SymbolPtr symbol = (
            withinStruct || pass.stage >= Pass::NAMES_ONLY_FALLBACK ?
            targetNamespace->requireLocalSymbol(newTypeName.prefix()) :
            targetNamespace->findLocalSymbol(newTypeName.prefix())
        );
        if (!symbol) {
            pass.unresolvedNestedTypeEncountered();
            return nullptr;
        }
        if (!symbol->ns)
            symbol->ns = std::unique_ptr<Namespace>(new Namespace(targetNamespace));
        targetNamespace = symbol->ns.get();
        newTypeName = newTypeName.exceptPrefix();
    }
    SymbolPtr symbol = targetNamespace->requireLocalSymbol(newTypeName.prefix());
    if (newTypeNamespace) {
        if (!symbol->ns)
            symbol->ns = std::unique_ptr<Namespace>(new Namespace(targetNamespace));
        *newTypeNamespace = symbol->ns.get();
    }
    return symbol;
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
    QualifiedName namespaceName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    if (matchSymbol(';'))
        return;
    if (!matchSymbol('{'))
        throw Error::INVALID_NAMESPACE_SYNTAX;
    NamespaceBlockGuard namespaceBlockGuard(*this, namespaceName);
    for (skipWhitespaceAndComments(); !matchSymbol('}'); skipWhitespaceAndComments()) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        parseSection();
    }
}

void HeaderParser::parseUsing() {
    skipWhitespaceAndComments();
    if (matchKeyword("namespace")) {
        skipWhitespaceAndComments();
        if (QualifiedName namespaceName = readNamespacedIdentifier()) {
            if (namespaceName.isAbsolute())
                usingNamespaces.push_back(namespaceName.exceptAbsolute().string());
            else {
                std::string namespaceNameString = namespaceName.string();
                for (int i = (int) namespaceNames.size(); i >= 0; --i) {
                    std::string fullNamespaceName;
                    for (int j = 0; j < i; ++j)
                        fullNamespaceName += namespaceNames[j]+"::";
                    fullNamespaceName += namespaceNameString;
                    usingNamespaces.push_back((std::string &&) fullNamespaceName);
                }
            }
        }
        return skipSection();
    }
    QualifiedName aliasName = readNamespacedIdentifier();
    if (!aliasName)
        return skipSection();
    skipWhitespaceAndComments();
    if (matchSymbol(';')) {
        // using ns::type;
        if (aliasName.exceptSuffix()) {
            if (SymbolPtr aliasedSymbol = curNamespace->findSymbol(aliasName, true)) {
                if (!curNamespace->makeLocalAlias(aliasName.suffix(), aliasedSymbol))
                    throw Error::TYPE_REDEFINITION;
            } else
                pass.unresolvedTypeAliasEncountered();
        }
        return;
    }
    if (matchSymbol('=') && aliasName.isUnqualified()) {
        // using x = y;
        skipWhitespaceAndComments();
        SymbolPtr aliasedSymbol;
        if (matchKeyword("struct"))
            aliasedSymbol = parseStruct(false);
        else if (matchKeyword("class")) {
            parseStruct(true);
            return skipSection();
        } else if (matchKeyword("enum"))
            aliasedSymbol = parseEnum();
        else if (matchKeyword("const") || matchKeyword("volatile"))
            return skipSection();
        else
            aliasedSymbol = tryParseType();
        if (aliasedSymbol && (aliasedSymbol = tryParseArrayTypeSuffix(aliasedSymbol))) {
            skipWhitespaceAndComments();
            if (matchSymbol(';')) {
                if (!curNamespace->makeLocalAlias(aliasName.prefix(), aliasedSymbol))
                    throw Error::TYPE_REDEFINITION;
                return;
            }
        } else
            pass.unresolvedTypeAliasEncountered();
    }
    skipSection();
}

void HeaderParser::parseTypedef() {
    skipWhitespaceAndComments();
    SymbolPtr aliasedType, unnamedType;
    if (matchKeyword("struct"))
        aliasedType = unnamedType = parseStruct(false);
    else if (matchKeyword("class")) {
        parseStruct(true);
        return skipSection();
    } else if (matchKeyword("enum"))
        aliasedType = unnamedType = parseEnum();
    else if (matchKeyword("const") || matchKeyword("volatile"))
        return skipSection();
    else {
        const char *orig = cur;
        try {
            aliasedType = parseType();
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
        QualifiedName aliasName = readNamespacedIdentifier();
        if (aliasName.isUnqualified()) {
            if (aliasedType) {
                if (SymbolPtr aliasedArrayType = tryParseArrayTypeSuffix(aliasedType))
                    aliasedType = aliasedArrayType;
                if (!curNamespace->makeLocalAlias(aliasName.prefix(), aliasedType))
                    throw Error::TYPE_REDEFINITION;
            } else
                pass.unresolvedTypeAliasEncountered();
        }
        skipWhitespaceAndComments();
    } while (matchSymbol(','));
    if (!matchSymbol(';'))
        skipSection();
}

SymbolPtr HeaderParser::parseStruct(bool isClass) {
    skipWhitespaceAndComments();
    QualifiedName structName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    // TODO struct STRUCT_API StructName { };
    const char *possibleDeclarationEnd = cur;
    if (matchKeyword("final"))
        skipWhitespaceAndComments();
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not a structure declaration/definition but a variable
        // Rewind cur because "final" is actually variable name
        cur = possibleDeclarationEnd;
        return curNamespace->findSymbol(structName, true);
    }

    SymbolPtr symbol;
    StructureType *structType = nullptr;
    Namespace *structNamespace = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    if (!isClass) {
        if (structName) {
            if ((symbol = newTypeSymbol(structName, &structNamespace))) {
                if (symbol->type) {
                    if (!((structType = symbol->type->incompleteStructureType()) || (forwardDeclaration && symbol->type->structureType())))
                        throw Error::TYPE_REDEFINITION;
                } else
                    symbol->type = std::unique_ptr<StructureType>(structType = new StructureType(Generator::safeName(fullTypeName(structName))));
            }
        }
        if (!structName && !forwardDeclaration && !pass.namesOnly()) {
            symbol = typeSet->newUnnamedStruct();
            assert(symbol->type);
            structType = static_cast<StructureType *>(symbol->type.get());
            assert(structType == symbol->type->incompleteStructureType());
        }
    } else if (structName)
        symbol = newTypeSymbol(structName, &structNamespace); // Establish symbol for class's potential public nested types
    if (!symbol) { // Structure cannot be parsed at this time because its real name could not be deduced
        skipSection();
        return nullptr;
    }
    if (forwardDeclaration)
        return symbol;
    assert(pass.namesOnly() || isClass || structType);

    NamespaceBlockGuard namespaceBlockGuard(*this, structName);
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
            SymbolPtr baseSymbol = tryParseType();
            if (const Type *baseType = symbolType(baseSymbol)) {
                skipWhitespaceAndComments();
                if (cur < end && *cur == '<') {
                    skipBlock(ANY_BRACES_INCLUDING_ANGLED);
                    continue;
                }
                if (!nonPublic && !pass.namesOnly() && structType)
                    structType->inheritFrom(baseType);
                if (structNamespace)
                    structNamespace->inheritFrom(baseSymbol->ns.get());
            } else {
                pass.unresolvedBaseTypeEncountered();
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
    withinStruct = true;
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

        SymbolPtr memberBaseTypeSymbol;
        const Type *memberBaseType = nullptr;
        bool nestedType = true;
        if (matchKeyword("using")) {
            parseUsing();
            continue;
        } else if (matchKeyword("typedef")) {
            parseTypedef();
            continue;
        } else if (matchKeyword("struct")) {
            memberBaseType = symbolType(memberBaseTypeSymbol = parseStruct(false));
            skipWhitespaceAndComments();
            if (matchSymbol(';')) {
                if (!pass.namesOnly() && !isClass && !staticMember && memberBaseType && memberBaseType->name().substance() == TypeName::VIRTUAL) {
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
            memberBaseType = symbolType(memberBaseTypeSymbol = parseEnum());
            skipWhitespaceAndComments();
            if (matchSymbol(';'))
                continue;
        } else
            nestedType = false;

        if (pass.namesOnly() || isClass || staticMember) {
            skipSection();
            continue;
        }

        if (!nestedType)
            memberBaseType = symbolType(memberBaseTypeSymbol = tryParseType());
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
                    const Type *memberType = symbolType(tryParseArrayTypeSuffix(memberBaseTypeSymbol));
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
    if (!pass.namesOnly() && !isClass)
        structType->completeMembers();
    assert(structType == (symbol ? symbol->type.get() : nullptr));
    return symbol;
}

SymbolPtr HeaderParser::parseEnum() {
    bool enumClass = false;
    skipWhitespaceAndComments();
    if (matchKeyword("class") || matchKeyword("struct")) {
        enumClass = true;
        skipWhitespaceAndComments();
    }
    QualifiedName enumName = readNamespacedIdentifier();
    skipWhitespaceAndComments();
    if (!(cur < end && (*cur == ';' || *cur == ':' || *cur == '{'))) {
        // Actually not an enum declaration/definition but a variable
        return curNamespace->findSymbol(enumName, true);
    }
    if (matchSymbol(':')) {
        while ((skipWhitespaceAndComments(), cur < end) && !(*cur == ';' || *cur == '{')) {
            const char *prev = cur;
            skipStringLiteral();
            cur += cur == prev;
        }
    }

    SymbolPtr symbol;
    EnumType *enumType = nullptr;
    bool forwardDeclaration = cur < end && *cur == ';';
    std::string enumNamespace;
    for (const std::string &namespaceSubName : namespaceNames)
        enumNamespace += namespaceSubName+"::";
    if (enumName) {
        std::string fullEnumName;
        if (enumName.isAbsolute()) {
            fullEnumName = enumName.exceptAbsolute().string();
            enumNamespace.clear();
        } else
            fullEnumName = enumNamespace+enumName.string();
        assert(fullEnumName == fullTypeName(enumName));
        if ((symbol = newTypeSymbol(enumName))) {
            if (symbol->type) {
                if (!(((enumType = symbol->type->incompleteEnumType()) && enumType->isEnumClass() == enumClass) || (forwardDeclaration && symbol->type->enumType() && symbol->type->enumType()->isEnumClass() == enumClass)))
                    throw Error::TYPE_REDEFINITION;
            } else
                symbol->type = std::unique_ptr<EnumType>(enumType = new EnumType(enumClass, enumNamespace, Generator::safeName(fullEnumName)));
        }
    }
    if (forwardDeclaration)
        return symbol;
    if (!enumName && !pass.namesOnly()) {
        symbol = typeSet->newUnnamedEnum(enumClass, enumNamespace);
        assert(symbol->type);
        enumType = static_cast<EnumType *>(symbol->type.get());
        assert(enumType == symbol->type->enumType());
    }

    if (!matchSymbol('{'))
        throw Error::INVALID_ENUM_SYNTAX;
    for (skipWhitespaceAndComments(); !matchSymbol('}'); skipWhitespaceAndComments()) {
        if (cur >= end)
            throw Error::UNEXPECTED_EOF;
        std::string enumValue = readIdentifier();
        if (enumValue.empty())
            throw Error::INVALID_ENUM_SYNTAX;
        if (!pass.namesOnly() && enumType)
            enumType->addValue(std::move(enumValue));
        skipWhitespaceAndComments();
        if (matchSymbol('=')) {
            skipExpression();
            skipWhitespaceAndComments();
        }
        matchSymbol(',');
    }
    if (!pass.namesOnly() && enumType)
        enumType->completeValues();
    assert(enumType == (symbol ? symbol->type.get() : nullptr));
    return symbol;
}

SymbolPtr HeaderParser::parseType() {
    const char *orig = cur;
    skipWhitespaceAndComments();
    if (cur < end && (isInitialNameChar(*cur) || *cur == ':')) {
        QualifiedName typeName = readNamespacedIdentifier();
        if (!typeName)
            return nullptr;
        const char *prev = cur;
        skipWhitespaceAndComments();
        // Special case: multi-word fundamental types
        if (typeName.isUnqualified() && (typeName.prefix().string() == "signed" || typeName.prefix().string() == "unsigned" || typeName.prefix().string() == "short" || typeName.prefix().string() == "long")) {
            std::string multiWordTypeName = typeName.prefix().string();
            const char *kwStart = cur;
            if (matchKeyword("short") || matchKeyword("long")) { // Possible 3 word type?
                multiWordTypeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments();
                kwStart = cur;
            }
            if (matchKeyword("short") || matchKeyword("long")) { // 4 word type??? (unsigned long long int)
                multiWordTypeName += " "+std::string(kwStart, cur);
                prev = cur;
                skipWhitespaceAndComments();
                kwStart = cur;
            }
            if (matchKeyword("int") || matchKeyword("char") || matchKeyword("double") || matchKeyword("float")) {
                multiWordTypeName += " "+std::string(kwStart, cur);
                prev = cur;
            }
            cur = prev;
            return typeSet->root().findLocalSymbol(multiWordTypeName);
        } else if (matchSymbol('<')) { // Template type
            skipWhitespaceAndComments();
            const Type *elementType = nullptr;
            // regular container
            if (ContainerTemplate<> *containerTemplate = findContainerTemplate<>(typeName.string())) {
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == elementTypeIndex)
                        elementType = symbolType(parseType());
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (elementType)
                    return typeSet->getContainerType(containerTemplate, elementType);
                return nullptr;
            }
            // static array container
            else if (ContainerTemplate<int> *containerTemplate = findContainerTemplate<int>(typeName.string())) {
                int arrayLength = -1;
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                int arrayLengthIndex = containerTemplate->templateArgIndex('N');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == elementTypeIndex)
                        elementType = symbolType(parseType());
                    else if (index == arrayLengthIndex)
                        arrayLength = parseArrayLength();
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (elementType && arrayLength >= 0)
                    return typeSet->getContainerType(containerTemplate, elementType, arrayLength);
                return nullptr;
            }
            // object map container
            else if (ContainerTemplate<const Type *> *containerTemplate = findContainerTemplate<const Type *>(typeName.string())) {
                const Type *keyType = nullptr;
                int keyTypeIndex = containerTemplate->templateArgIndex('K');
                int elementTypeIndex = containerTemplate->templateArgIndex('T');
                for (int index = 0; !matchSymbol('>'); ++index) {
                    if (cur >= end)
                        throw Error::UNEXPECTED_EOF;
                    if (index && !matchSymbol(','))
                        throw Error::INVALID_TYPENAME_SYNTAX;
                    if (index == keyTypeIndex)
                        keyType = symbolType(parseType());
                    else if (index == elementTypeIndex)
                        elementType = symbolType(parseType());
                    else
                        skipTemplateArgument();
                    skipWhitespaceAndComments();
                }
                if (keyType && elementType)
                    return typeSet->getContainerType(containerTemplate, elementType, keyType);
                return nullptr;
            }
        } else {
            cur = prev;
            return curNamespace->findSymbol(typeName, true);
        }
    }
    cur = orig;
    return nullptr;
}

SymbolPtr HeaderParser::tryParseType() {
    const char *orig = cur;
    try {
        return parseType();
    } catch (Error::Type) {
        cur = orig;
        return nullptr;
    }
}

SymbolPtr HeaderParser::tryParseArrayTypeSuffix(SymbolPtr symbol) {
    skipWhitespaceAndComments();
    if (symbol && symbol->type) {
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
            while (!arrayDimensions.empty()) {
                symbol = typeSet->getStaticArray(symbol->type.get(), arrayDimensions.top());
                assert(symbol && symbol->type);
                arrayDimensions.pop();
            }
            return symbol;
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
    if (cur < end && !(*cur&0x80) && isdigit(*cur)) {
        std::string number;
        do {
            number.push_back(*cur++);
        } while (cur < end && isNameChar(*cur)); // isNameChar because of literal suffixes
        int arrayLength = 0;
        if (sscanf(number.c_str(), "%i", &arrayLength) == 1)
            return arrayLength;
    }
    throw Error::INVALID_ARRAY_SYNTAX;
}

QualifiedName HeaderParser::readNamespacedIdentifier() {
    const char *orig = cur;
    QualifiedName identifier;
    if (cur+1 < end && *cur == ':' && *(cur+1) == ':') {
        cur += 2;
        skipWhitespaceAndComments();
        identifier.setAbsolute(true);
    }
    if (cur < end && isInitialNameChar(*cur)) {
        while (true) {
            std::string subIdentifier = readIdentifier();
            if (subIdentifier.empty())
                break;
            identifier.append((std::string &&) subIdentifier);
            const char *prev = cur;
            skipWhitespaceAndComments();
            if (cur+1 < end && *cur == ':' && *(cur+1) == ':') {
                cur += 2;
                skipWhitespaceAndComments();
            } else {
                cur = prev;
                return identifier;
            }
        }
    }
    cur = orig;
    return identifier;
}

std::string HeaderParser::readIdentifier() {
    if (cur < end && isInitialNameChar(*cur)) {
        std::string identifier;
        do {
            identifier.push_back(*cur++);
        } while (cur < end && isNameChar(*cur));
        return identifier;
    }
    return std::string();
}

void HeaderParser::skipLine(bool ignoreComments) {
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
            case '/':
                if (!ignoreComments && cur < end && *cur == '*') {
                    --cur;
                    skipComment();
                }
                escaped = false;
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
                skipLine(false);
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
                skipLine(true);
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
                braceTypes = ANY_BRACES_EXCEPT_ANGLED;
                break;
            case '[':
                endSymbol = ']';
                braceTypes = ANY_BRACES_EXCEPT_ANGLED;
                break;
            case '{':
                endSymbol = '}';
                braceTypes = ANY_BRACES_EXCEPT_ANGLED;
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
            if (*cur == ')' || *cur == ']' || *cur == '}')
                throw Error::BRACE_MISMATCH;
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
        if (matchSymbol(';') || *cur == '}')
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
            if (subCur < end && isNameChar(*subCur))
                return false;
            cur = subCur;
            return true;
        }
    } while (subCur < end && *subCur++ == *keyword++);
    return false;
}

HeaderParser::Error parseHeader(HeaderParser::Pass &pass, TypeSet &outputTypeSet, const std::string &headerString) {
    const char *headerStart = headerString.c_str();
    HeaderParser parser(pass, outputTypeSet, headerStart, headerString.size());
    HeaderParser::Error::Type parseErrorType = HeaderParser::Error::OK;
    try {
        parser.parse();
    } catch (HeaderParser::Error::Type errorType) {
        parseErrorType = errorType;
    }
    int position = int(parser.currentChar()-headerStart);
    return HeaderParser::Error(parseErrorType, position);
}
