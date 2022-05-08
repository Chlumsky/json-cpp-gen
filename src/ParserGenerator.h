
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include "TypeSet.h"
#include "Generator.h"

/// Generates the code for the JSON parser of a structure
class ParserGenerator : public Generator {

public:
    struct Error {
        static const char * const JSON_SYNTAX_ERROR;
        static const char * const UNEXPECTED_END_OF_FILE;
        static const char * const TYPE_MISMATCH;
        static const char * const ARRAY_SIZE_MISMATCH;
        static const char * const UNKNOWN_KEY;
        static const char * const UNKNOWN_ENUM_VALUE;
        static const char * const VALUE_OUT_OF_RANGE;
        static const char * const STRING_EXPECTED;
        static const char * const UTF16_ENCODING_ERROR;
    };

    static std::string generateMatchKeyword(const char *keyword);

    explicit ParserGenerator(const std::string &className, const StringType *stringType = &StringType::STD_STRING, const Settings &settings = Settings());
    void generateParserFunction(const Type *type);
    std::string generateParserFunctionCall(const Type *type, const std::string &outputArg);
    std::string generateValueParse(const Type *type, const std::string &outputArg, const std::string &indent);

    std::string generateHeader();
    std::string generateSource();
    std::string generateSource(const std::string &relativeHeaderAddress);

};
