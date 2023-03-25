
#pragma once

#include "Generator.h"
#include "types/StringType.h"

#define FOR_SERIALIZER_ERRORS(M) \
    M(UNREPRESENTABLE_FLOAT_VALUE) \
    M(UNKNOWN_ENUM_VALUE) \

class SerializerGenerator : public Generator {

public:
    struct Error {
        #define SERIALIZER_GENERATOR_ERROR_STR_DECL(e) static const char *const e;
        FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_ERROR_STR_DECL)
        #undef SERIALIZER_GENERATOR_ERROR_STR_DECL
    };

    static constexpr const char *const OUTPUT_STRING = "json";

    static const unsigned FEATURE_WRITE_SIGNED;
    static const unsigned FEATURE_WRITE_UNSIGNED;
    static const unsigned FEATURE_SERIALIZE_FLOAT;
    static const unsigned FEATURE_SERIALIZE_DOUBLE;
    static const unsigned FEATURE_SERIALIZE_LONG_DOUBLE;

    explicit SerializerGenerator(const std::string &className, const StringType *stringType = &StringType::STD_STRING, const Settings &settings = Settings());
    void generateSerializerFunction(const Type *type);
    std::string generateSerializerFunctionCall(const Type *type, const std::string &inputArg);
    std::string generateValueSerialization(const Type *type, const std::string &inputArg, const std::string &indent = std::string());
    std::string generateErrorStatement(const char *errorName) const; // throw / return depending on config

    std::string generateHeader();
    std::string generateSource();
    std::string generateSource(const std::string &relativeHeaderAddress);

};
