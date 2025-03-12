
#pragma once

#include <string>
#include <vector>
#include "QualifiedName.h"
#include "Namespace.h"
#include "TypeSet.h"

#define FOR_HEADER_PARSER_ERROR_TYPES(M) \
    M(TYPE_REDEFINITION) \
    M(UNSUPPORTED_TYPE) \
    M(UNEXPECTED_EOF) \
    M(INVALID_STRUCTURE_SYNTAX) \
    M(INVALID_ENUM_SYNTAX) \
    M(INVALID_TYPENAME_SYNTAX) \
    M(INVALID_ARRAY_SYNTAX) \
    M(INVALID_NAMESPACE_SYNTAX) \
    M(DUPLICATE_STRUCT_MEMBER) \
    M(CYCLIC_TYPE_ALIAS) \
    M(UNEXPECTED_CODE_PATH) \
    M(NOT_IMPLEMENTED) \

const Type *parseType(TypeSet &typeSet, const std::string &typeString);

// Parses structures from a C++ header file.
class HeaderParser {
    friend const Type *parseType(TypeSet &typeSet, const std::string &typeString);

public:
    struct Error {
        enum Type {
            OK = 0,
            #define HEADER_PARSER_ERROR_TYPE_ENUM_VAL(x) x,
            FOR_HEADER_PARSER_ERROR_TYPES(HEADER_PARSER_ERROR_TYPE_ENUM_VAL)
            #undef HEADER_PARSER_ERROR_TYPE_ENUM_VAL
        } type;
        int position;

        inline Error(Type type = Error::OK, int position = -1) : type(type), position(position) { }
        operator Type() const;
        operator bool() const;
        const char *typeString() const;
    };

    class Pass {
        friend class HeaderParser;
    public:
        enum Stage {
            INITIAL,
            NAMES_ONLY,
            /// In this pass, nested structures and enums with unrecognized parent namespaces will be taken as-is.
            NAMES_ONLY_FALLBACK,
            FINAL,
            NONE
        };

        explicit Pass(Stage initialStage = INITIAL);
        void operator++();
        explicit operator bool() const;

    private:
        Stage stage;
        struct {
            int unresolvedNestedTypes;
            int unresolvedTypeAliases;
        } cur, prev;

        bool namesOnly() const;
    };

    HeaderParser(Pass &pass, TypeSet &outputTypeSet, const char *headerStart, size_t headerLength);
    const char *currentChar() const;
    void parse();

private:
    Pass &pass;
    TypeSet *typeSet;
    const char *cur, *end;
    Namespace *curNamespace;
    std::vector<std::string> namespaceNames;
    std::vector<std::string> usingNamespaces;
    bool withinStruct;

    enum BraceTypes {
        ANY_BRACES_EXCEPT_ANGLED,
        ANY_BRACES_INCLUDING_ANGLED
    };

    class NamespaceBlockGuard {
        HeaderParser &parent;
        Namespace *outerNamespace;
        size_t outerNamespaceNamesLength;
        size_t outerUsingNamespacesCount;
        std::vector<std::string> outerNamespaceNames;
        bool outerWithinStruct;
    public:
        NamespaceBlockGuard(HeaderParser &parent, QualifiedName::Ref namespaceName);
        ~NamespaceBlockGuard();
    };

    template <typename... T>
    ContainerTemplate<T...> *findContainerTemplate(const std::string &name);
    std::string fullTypeName(QualifiedName::Ref baseName) const;
    SymbolPtr newTypeSymbol(QualifiedName::Ref newTypeName);

    void parseSection();
    void parseNamespace();
    void parseUsing();
    void parseTypedef();
    SymbolPtr parseStruct(bool isClass);
    SymbolPtr parseEnum();
    SymbolPtr parseType();
    SymbolPtr tryParseType();
    SymbolPtr tryParseArrayTypeSuffix(SymbolPtr symbol);
    int parseArrayLength();
    QualifiedName readNamespacedIdentifier();
    std::string readIdentifier();
    void skipLine();
    void skipWhitespaceAndComments();
    bool skipComment();
    bool skipStringLiteral();
    void skipExpression();
    void skipBlock(BraceTypes braceTypes);
    void skipSection();
    void skipTemplateArgument();
    bool matchSymbol(char s);
    bool matchKeyword(const char *keyword);

};

HeaderParser::Error parseHeader(HeaderParser::Pass &pass, TypeSet &outputTypeSet, const std::string &headerString);

template <typename... T>
ContainerTemplate<T...> *HeaderParser::findContainerTemplate(const std::string &name) {
    if (name.empty())
        return nullptr;
    if (name.size() >= 2 && name[0] == ':' && name[1] == ':')
        return typeSet->findContainerTemplate<T...>(name.substr(2));
    for (int i = (int) namespaceNames.size(); i >= 0; --i) {
        std::string fullName;
        for (int j = 0; j < i; ++j)
            fullName += namespaceNames[j]+"::";
        fullName += name;
        if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>(fullName))
            return containerTemplate;
    }
    for (const std::string &ns : usingNamespaces)
        if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>(ns+"::"+name))
            return containerTemplate;
    return nullptr;
}
