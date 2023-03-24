
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include "TypeSet.h"
#include "StringSwitchTree.h"
#include "Generator.h"

#define FOR_PARSER_ERRORS(M) \
    M(JSON_SYNTAX_ERROR) \
    M(UNEXPECTED_END_OF_FILE) \
    M(TYPE_MISMATCH) \
    M(ARRAY_SIZE_MISMATCH) \
    M(UNKNOWN_ENUM_VALUE) \
    M(UNKNOWN_KEY) \
    M(MISSING_KEY) \
    M(REPEATED_KEY) \
    M(VALUE_OUT_OF_RANGE) \
    M(STRING_EXPECTED) \
    M(UTF16_ENCODING_ERROR) \

/// Generates the code for the JSON parser of a structure
class ParserGenerator : public Generator {

public:
    struct Error {
        #define PARSER_GENERATOR_ERROR_STR_DECL(e) static const char *const e;
        FOR_PARSER_ERRORS(PARSER_GENERATOR_ERROR_STR_DECL)
        #undef PARSER_GENERATOR_ERROR_STR_DECL
    };

    class SwitchTreeCaseGenerator {
    public:
        virtual ~SwitchTreeCaseGenerator() = default;
        virtual std::string operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int knownMinLength, const std::string &indent) = 0;
    };

    static const unsigned FEATURE_READ_SIGNED;
    static const unsigned FEATURE_READ_UNSIGNED;

    static std::string generateMatchKeyword(const char *keyword);

    explicit ParserGenerator(const std::string &className, const StringType *stringType = &StringType::STD_STRING, const Settings &settings = Settings());
    void generateParserFunction(const Type *type);
    std::string generateParserFunctionCall(const Type *type, const std::string &outputArg);
    std::string generateValueParse(const Type *type, const std::string &outputArg, const std::string &indent);
    std::string generateErrorStatement(const char *errorName) const; // throw / return depending on config
    std::string generateSwitchTree(SwitchTreeCaseGenerator *caseGenerator, const StringSwitchTree *switchTree, const StringType *valueType, const char *value, const std::string &indent, int knownMinLength = 0);

    std::string generateHeader();
    std::string generateSource();
    std::string generateSource(const std::string &relativeHeaderAddress);

private:
    std::string generateReadIntegerBody(bool signedInt) const;

};
