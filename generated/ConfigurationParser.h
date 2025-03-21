
// Generated by json-cpp-gen by Viktor Chlumsky
// https://github.com/Chlumsky/json-cpp-gen

#pragma once

#include <string>
#include "../src/NameFormat.h"
#include "../src/Configuration.h"

class ConfigurationParser {

public:
    struct Error {
        enum Type {
            OK,
            JSON_SYNTAX_ERROR,
            UNEXPECTED_END_OF_FILE,
            TYPE_MISMATCH,
            ARRAY_SIZE_MISMATCH,
            UNKNOWN_ENUM_VALUE,
            UNKNOWN_KEY,
            MISSING_KEY,
            REPEATED_KEY,
            VALUE_OUT_OF_RANGE,
            STRING_EXPECTED,
            UTF16_ENCODING_ERROR
        } type;
        int position;

        inline Error(Type type = Error::OK, int position = -1) : type(type), position(position) { }
        operator Type() const;
        operator bool() const;
        const char *typeString() const;
    };

    static Error parse(Configuration &output, const char *jsonString);

protected:
    const char *cur;
    std::string buffer;

    explicit ConfigurationParser(const char *str);
    void skipWhitespace();
    void skipValue();
    bool matchSymbol(char s);
    void readHexQuad(int &value);
    static bool isAlphanumeric(char c);
    static int decodeHexDigit(char digit);

    void parseStdString(std::string &value);
    void parseStdVectorStdString(std::vector<std::string> &value);
    void parseConfigurationGeneratorDef(Configuration::GeneratorDef &value);
    void parseStdVectorConfigurationGeneratorDef(std::vector<Configuration::GeneratorDef> &value);
    void parseBool(bool &value);
    void parseSettingsJsonIO(Settings::JsonIO &value);
    void parseSettingsInfPolicy(Settings::InfPolicy &value);
    void parseSettingsNanPolicy(Settings::NanPolicy &value);
    void parseSettingsLineEndingStyle(Settings::LineEndingStyle &value);
    void parseSettings(Settings &value);
    void parseStringAPI(StringAPI &value);
    void parseConfigurationStringDef(Configuration::StringDef &value);
    void parseStdVectorConfigurationStringDef(std::vector<Configuration::StringDef> &value);
    void parseStdMapStdStringStdString(std::map<std::string, std::string> &value);
    void parseConstStringAPI(ConstStringAPI &value);
    void parseConfigurationConstStringDef(Configuration::ConstStringDef &value);
    void parseStdVectorConfigurationConstStringDef(std::vector<Configuration::ConstStringDef> &value);
    void parseArrayContainerAPI(ArrayContainerAPI &value);
    void parseConfigurationArrayContainerDef(Configuration::ArrayContainerDef &value);
    void parseStdVectorConfigurationArrayContainerDef(std::vector<Configuration::ArrayContainerDef> &value);
    void parseObjectContainerAPI(ObjectContainerAPI &value);
    void parseConfigurationObjectContainerDef(Configuration::ObjectContainerDef &value);
    void parseStdVectorConfigurationObjectContainerDef(std::vector<Configuration::ObjectContainerDef> &value);
    void parseOptionalContainerAPI(OptionalContainerAPI &value);
    void parseConfigurationOptionalContainerDef(Configuration::OptionalContainerDef &value);
    void parseStdVectorConfigurationOptionalContainerDef(std::vector<Configuration::OptionalContainerDef> &value);
    void parseConfigurationObjectMapContainerDef(Configuration::ObjectMapContainerDef &value);
    void parseStdVectorConfigurationObjectMapContainerDef(std::vector<Configuration::ObjectMapContainerDef> &value);
    void parseFixedArrayContainerAPI(FixedArrayContainerAPI &value);
    void parseConfigurationFixedArrayContainerDef(Configuration::FixedArrayContainerDef &value);
    void parseStdVectorConfigurationFixedArrayContainerDef(std::vector<Configuration::FixedArrayContainerDef> &value);
    void parseStaticArrayContainerAPI(StaticArrayContainerAPI &value);
    void parseConfigurationStaticArrayContainerDef(Configuration::StaticArrayContainerDef &value);
    void parseStdVectorConfigurationStaticArrayContainerDef(std::vector<Configuration::StaticArrayContainerDef> &value);
    void parseConfiguration(Configuration &value);

};
