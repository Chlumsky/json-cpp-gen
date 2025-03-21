
#pragma once

#include <string>
#include <vector>
#include "QualifiedName.h"
#include "Namespace.h"
#include "TypeSet.h"

#define FOR_HEADER_PARSER_ERROR_TYPES(M) \
    M(TYPE_REDEFINITION) \
    M(NAMESPACE_REDEFINITION) \
    M(UNSUPPORTED_TYPE) \
    M(UNEXPECTED_EOF) \
    M(BRACE_MISMATCH) \
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
            /// Unresolved nested types are a separate category because they are handled differently in the NAMES_ONLY_FALLBACK stage
            int unresolvedNestedTypes;
            /// Unresolved aliases and class base types can prevent nested types from being recognized
            int nameResolutionBlockers;
        } cur, prev;

        bool namesOnly() const;
        void unresolvedNestedTypeEncountered();
        void unresolvedAliasEncountered();
        void unresolvedBaseTypeEncountered();
    };

    HeaderParser(Pass &pass, TypeSet &outputTypeSet, const char *headerStart, size_t headerLength);
    const char *currentChar() const;
    void parse();

private:
    Pass &pass;
    TypeSet *typeSet;
    const char *cur, *end;
    Namespace *curNamespace;
    bool withinStruct;

    enum BraceTypes {
        ANY_BRACES_EXCEPT_ANGLED,
        ANY_BRACES_INCLUDING_ANGLED
    };

    class NamespaceBlockGuard {
        HeaderParser &parent;
        Namespace *outerNamespace;
        bool outerWithinStruct;
    public:
        NamespaceBlockGuard(HeaderParser &parent, QualifiedName::Ref namespaceName);
        ~NamespaceBlockGuard();
    };

    template <typename... T>
    ContainerTemplate<T...> *findContainerTemplate(const std::string &name);
    SymbolPtr newTypeSymbol(QualifiedName::Ref newTypeName, Namespace **newTypeNamespace = nullptr);

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
    void skipLine(bool ignoreComments);
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
    for (Namespace *searchedNamespace = curNamespace; searchedNamespace; searchedNamespace = searchedNamespace->parentNamespace()) {
        if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>(searchedNamespace->fullName().stringPrefix()+name))
            return containerTemplate;
    }
    // using namespace fallback
    for (Namespace *searchedNamespace = curNamespace; searchedNamespace; searchedNamespace = searchedNamespace->parentNamespace()) {
        for (const QualifiedName &usingNamespaceName : searchedNamespace->usingNamespaces()) {
            if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>((searchedNamespace->fullName()+usingNamespaceName).exceptAbsolute().stringPrefix()+name))
                return containerTemplate;
        }
    }
    return nullptr;
}
