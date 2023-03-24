
#pragma once

#include <string>
#include <vector>
#include "TypeSet.h"

// Parses structures from a C++ header file.
class HeaderParser {

public:
    struct Error {
        #define FOR_HEADER_PARSER_ERROR_TYPES(M) \
            M(TYPE_REDEFINITION) \
            M(UNSUPPORTED_TYPE) \
            M(UNEXPECTED_EOF) \
            M(STRUCT_NAME_EXPECTED) \
            M(ENUM_NAME_EXPECTED) \
            M(INVALID_STRUCTURE_SYNTAX) \
            M(INVALID_ENUM_SYNTAX) \
            M(INVALID_TYPENAME_SYNTAX) \
            M(INVALID_ARRAY_SYNTAX) \
            M(INVALID_NAMESPACE_SYNTAX) \
            M(NOT_IMPLEMENTED)

        enum Type {
            OK = 0,
            #define HEADER_PARSER_ERROR_TYPE_ENUM(x) x,
            FOR_HEADER_PARSER_ERROR_TYPES(HEADER_PARSER_ERROR_TYPE_ENUM)
            #undef HEADER_PARSER_ERROR_TYPE_ENUM
        } type;
        int position;

        inline Error(Type type = Error::OK, int position = -1) : type(type), position(position) { }
        operator Type() const;
        operator bool() const;
        const char * typeString() const;
    };

    HeaderParser(TypeSet *outputTypeSet, const char *headerStart, size_t headerLength, bool parseNamesOnly = false);
    const char * currentChar() const;
    void parse();
    const Type * parseType();

private:
    TypeSet *typeSet;
    const char *cur, *end;
    std::vector<std::string> curNamespace;
    std::vector<std::string> usingNamespaces;
    bool parseNamesOnly; // prepass in case input files are in the wrong order

    enum SkipWhitespaceMode {
        SINGLE_LINE,
        MULTI_LINE
    };
    enum BraceTypes {
        ANY_BRACES_EXCEPT_ANGLED,
        ANY_BRACES_INCLUDING_ANGLED
    };

    Type * findType(const std::string &name);
    template <typename... T>
    ContainerTemplate<T...> * findContainerTemplate(const std::string &name);

    void parseSection();
    void parseNamespace();
    const Type * parseStruct();
    const Type * parseEnum();
    int parseArrayLength();
    std::string readNamespacedIdentifier();
    std::string readIdentifier();
    void skipWhitespaceAndComments(SkipWhitespaceMode mode);
    void skipWhitespace(SkipWhitespaceMode mode);
    void skipComment();
    void skipStringLiteral();
    void skipExpression();
    void skipBlock(BraceTypes braceTypes);
    void skipSection();
    void skipTemplateArgument();
    void skipDirective();
    bool matchSymbol(char s);
    bool matchKeyword(const char *keyword);

    static bool isNonSymbol(char c);

};

HeaderParser::Error preparseHeader(TypeSet &outputTypeSet, const std::string &headerString);
HeaderParser::Error parseHeader(TypeSet &outputTypeSet, const std::string &headerString, bool parseNamesOnly = false);
const Type * parseType(TypeSet &typeSet, const std::string &typeString);

template <typename... T>
ContainerTemplate<T...> * HeaderParser::findContainerTemplate(const std::string &name) {
    if (name.empty())
        return nullptr;
    if (name.size() >= 2 && name[0] == ':' && name[1] == ':')
        return typeSet->findContainerTemplate<T...>(name.substr(2));
    if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>(name))
        return containerTemplate;
    for (const std::string &ns : usingNamespaces)
        if (ContainerTemplate<T...> *containerTemplate = typeSet->findContainerTemplate<T...>(ns+"::"+name))
            return containerTemplate;
    return nullptr;
}
