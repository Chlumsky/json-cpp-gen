
#pragma once

#include <string>
#include <vector>
#include "TypeSet.h"

// Parses structures from a C++ header file.
class HeaderParser {

public:
    enum class Error {
        OK = 0,
        TYPE_REDEFINITION,
        UNSUPPORTED_TYPE,
        UNEXPECTED_EOF,
        STRUCT_NAME_EXPECTED,
        ENUM_NAME_EXPECTED,
        INVALID_STRUCTURE_SYNTAX,
        INVALID_ENUM_SYNTAX,
        INVALID_TYPENAME_SYNTAX,
        INVALID_ARRAY_SYNTAX,
        INVALID_NAMESPACE_SYNTAX,
        NOT_IMPLEMENTED
    };

    HeaderParser(TypeSet *outputTypeSet, const char *headerStart, size_t headerLength, bool parseNamesOnly = false);
    Error parse();
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
