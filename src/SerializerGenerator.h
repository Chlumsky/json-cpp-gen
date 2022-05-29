
#pragma once

#include "Generator.h"
#include "types/StringType.h"

class SerializerGenerator : public Generator {

public:
    struct Error {
        static const char * const UNREPRESENTABLE_FLOAT_VALUE;
        static const char * const UNKNOWN_ENUM_VALUE;
    };

    static const unsigned FEATURE_WRITE_SIGNED;
    static const unsigned FEATURE_WRITE_UNSIGNED;

    explicit SerializerGenerator(const std::string &className, const StringType *stringType = &StringType::STD_STRING, const Settings &settings = Settings());
    void generateSerializerFunction(const Type *type);
    std::string generateSerializerFunctionCall(const Type *type, const std::string &inputArg);
    std::string generateValueSerialization(const Type *type, const std::string &inputArg, const std::string &indent = std::string());

    std::string generateHeader();
    std::string generateSource();
    std::string generateSource(const std::string &relativeHeaderAddress);

};
