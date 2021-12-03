
#include "ConfigurationParser.h"

// Generated by json-cpp-gen by Viktor Chlumsky
// https://github.com/Chlumsky/json-cpp-gen

ConfigurationParser::ConfigurationParser(const char *str) : cur(str) { }

void ConfigurationParser::skipWhitespace() {
    while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')
        ++cur;
}

void ConfigurationParser::skipValue() {
    int openBrackets = 1;
    skipWhitespace();
    switch (*cur) {
        case '\0':
            throw Error::UNEXPECTED_END_OF_FILE;
        case '"':
            while (*++cur != '"') {
                if (!*(cur += *cur == '\\'))
                    throw Error::UNEXPECTED_END_OF_FILE;
            }
            ++cur;
            return;
        case '[': case '{':
            while (openBrackets) {
                switch (*++cur) {
                    case '\0':
                        throw Error::UNEXPECTED_END_OF_FILE;
                    case '"':
                        skipValue();
                        break;
                    case '[': case '{':
                        ++openBrackets;
                        break;
                    case ']': case '}':
                        --openBrackets;
                        break;
                }
            }
            ++cur;
            return;
        default:
            if (isalnum(*cur) || *cur == '-' || *cur == '.') {
                while (isalnum(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
                return;
            }
    }
    throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::requireSymbol(char s) {
    skipWhitespace();
    if (*cur++ != s)
        throw Error::JSON_SYNTAX_ERROR;
}

bool ConfigurationParser::matchSymbol(char s) {
    skipWhitespace();
    if (*cur == s) {
        ++cur;
        return true;
    }
    return false;
}

void ConfigurationParser::parseEscaped(char *sequence) {
    switch (*++cur) {
        case '\0':
            throw Error::UNEXPECTED_END_OF_FILE;
        case 'B': case 'b': sequence[0] = '\b'; break;
        case 'F': case 'f': sequence[0] = '\f'; break;
        case 'N': case 'n': sequence[0] = '\n'; break;
        case 'R': case 'r': sequence[0] = '\r'; break;
        case 'T': case 't': sequence[0] = '\t'; break;
        case 'U': case 'u': {
            unsigned long cp;
            unsigned short wc;
            ++cur;
            if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                throw Error::JSON_SYNTAX_ERROR;
            sequence[0] = cur[0], sequence[1] = cur[1], sequence[2] = cur[2], sequence[3] = cur[3];
            sequence[4] = '\0';
            cur += 3;
            if (sscanf(sequence, "%hx", &wc) != 1)
                throw Error::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    throw Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    throw Error::JSON_SYNTAX_ERROR;
                sequence[0] = cur[0], sequence[1] = cur[1], sequence[2] = cur[2], sequence[3] = cur[3];
                sequence[4] = '\0';
                cur += 3;
                if (sscanf(sequence, "%hx", &wc) != 1)
                    throw Error::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    throw Error::UTF16_ENCODING_ERROR;
                cp = 0x010000+(cp|(unsigned long) (wc&0x03ff));
            } else
                cp = wc;
            if (cp&0xffffff80) {
                int len;
                for (len = 1; cp>>(5*len+1) && len < 6; ++len);
                sequence[0] = (char) (0xff<<(8-len)|cp>>6*(len-1));
                for (int i = 1; i < len; ++i)
                    *++sequence = (char) (0x80|(cp>>6*(len-i-1)&0x3f));
            } else
                sequence[0] = (char) cp;
            break;
        }
        default:
            sequence[0] = *cur;
    }
    sequence[1] = '\0';
}

ConfigurationParser::Error ConfigurationParser::parse(Configuration &output, const char *jsonString) {
    try {
        ConfigurationParser(jsonString).parseConfiguration(output);
    } catch (Error error) {
        return error;
    }
    return Error::OK;
}

void ConfigurationParser::parseStdString(std::string &value) {
    skipWhitespace();
    if (*cur != '"')
        throw Error::STRING_EXPECTED;
    value.clear();
    while (*++cur != '"') {
        if (*cur == '\\') {
            char buffer[8];
            parseEscaped(buffer);
            value += buffer;
            continue;
        }
        if (!*cur)
            throw Error::UNEXPECTED_END_OF_FILE;
        value.push_back(*cur);
    }
    ++cur;
}

void ConfigurationParser::parseStdVectorStdString(std::vector<std::string> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseSettingsJsonIO(Settings::JsonIO &value) {
    std::string str;
    parseStdString(str);
    if (str == "NULL_TERMINATED_STRING")
        value = Settings::JsonIO::NULL_TERMINATED_STRING;
    else
        throw Error::UNKNOWN_ENUM_VALUE;
}

void ConfigurationParser::parseNameFormat(NameFormat &value) {
    std::string str;
    parseStdString(str);
    if (str == "ANY")
        value = NameFormat::ANY;
    else if (str == "UPPERCASE_UNDERSCORE")
        value = NameFormat::UPPERCASE_UNDERSCORE;
    else if (str == "LOWERCASE_UNDERSCORE")
        value = NameFormat::LOWERCASE_UNDERSCORE;
    else if (str == "UPERCASE_DASH")
        value = NameFormat::UPERCASE_DASH;
    else if (str == "LOWERCASE_DASH")
        value = NameFormat::LOWERCASE_DASH;
    else if (str == "CAMELCASE")
        value = NameFormat::CAMELCASE;
    else if (str == "CAMELCASE_CAPITAL")
        value = NameFormat::CAMELCASE_CAPITAL;
    else
        throw Error::UNKNOWN_ENUM_VALUE;
}

void ConfigurationParser::parseBool(bool &value) {
    skipWhitespace();
    if (cur[0] == 'f' && cur[1] == 'a' && cur[2] == 'l' && cur[3] == 's' && cur[4] == 'e' && !isalnum(cur[5]) && cur[5] != '_') {
        value = false;
        cur += 5;
    } else if (cur[0] == 't' && cur[1] == 'r' && cur[2] == 'u' && cur[3] == 'e' && !isalnum(cur[4]) && cur[4] != '_') {
        value = true;
        cur += 4;
    } else
        throw Error::TYPE_MISMATCH;
}

void ConfigurationParser::parseSettingsNanPolicy(Settings::NanPolicy &value) {
    std::string str;
    parseStdString(str);
    if (str == "SERIALIZER_ERROR")
        value = Settings::NanPolicy::SERIALIZER_ERROR;
    else if (str == "ZERO_VALUE")
        value = Settings::NanPolicy::ZERO_VALUE;
    else if (str == "NULL_VALUE")
        value = Settings::NanPolicy::NULL_VALUE;
    else if (str == "UPPERCASE_NAN_STRING_VALUE")
        value = Settings::NanPolicy::UPPERCASE_NAN_STRING_VALUE;
    else if (str == "LOWERCASE_NAN_STRING_VALUE")
        value = Settings::NanPolicy::LOWERCASE_NAN_STRING_VALUE;
    else if (str == "MIXED_CASE_NAN_STRING_VALUE")
        value = Settings::NanPolicy::MIXED_CASE_NAN_STRING_VALUE;
    else
        throw Error::UNKNOWN_ENUM_VALUE;
}

void ConfigurationParser::parseSettingsInfPolicy(Settings::InfPolicy &value) {
    std::string str;
    parseStdString(str);
    if (str == "SERIALIZER_ERROR")
        value = Settings::InfPolicy::SERIALIZER_ERROR;
    else if (str == "EXPONENT_OVERFLOW")
        value = Settings::InfPolicy::EXPONENT_OVERFLOW;
    else if (str == "ZERO_VALUE")
        value = Settings::InfPolicy::ZERO_VALUE;
    else if (str == "NULL_VALUE")
        value = Settings::InfPolicy::NULL_VALUE;
    else if (str == "UPPERCASE_INF_STRING_VALUE")
        value = Settings::InfPolicy::UPPERCASE_INF_STRING_VALUE;
    else if (str == "LOWERCASE_INF_STRING_VALUE")
        value = Settings::InfPolicy::LOWERCASE_INF_STRING_VALUE;
    else if (str == "CAPITALIZED_INF_STRING_VALUE")
        value = Settings::InfPolicy::CAPITALIZED_INF_STRING_VALUE;
    else if (str == "UPPERCASE_INFINITY_STRING_VALUE")
        value = Settings::InfPolicy::UPPERCASE_INFINITY_STRING_VALUE;
    else if (str == "LOWERCASE_INFINITY_STRING_VALUE")
        value = Settings::InfPolicy::LOWERCASE_INFINITY_STRING_VALUE;
    else if (str == "CAPITALIZED_INFINITY_STRING_VALUE")
        value = Settings::InfPolicy::CAPITALIZED_INFINITY_STRING_VALUE;
    else
        throw Error::UNKNOWN_ENUM_VALUE;
}

void ConfigurationParser::parseSettings(Settings &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "jsonIOMode")
            parseSettingsJsonIO(value.jsonIOMode);
        else if (key == "keyFormat")
            parseNameFormat(value.keyFormat);
        else if (key == "enumFormat")
            parseNameFormat(value.enumFormat);
        else if (key == "noThrow")
            parseBool(value.noThrow);
        else if (key == "verboseErrors")
            parseBool(value.verboseErrors);
        else if (key == "strictSyntaxCheck")
            parseBool(value.strictSyntaxCheck);
        else if (key == "checkMissingKeys")
            parseBool(value.checkMissingKeys);
        else if (key == "checkRepeatingKeys")
            parseBool(value.checkRepeatingKeys);
        else if (key == "ignoreExtraKeys")
            parseBool(value.ignoreExtraKeys);
        else if (key == "checkIntegerOverflow")
            parseBool(value.checkIntegerOverflow);
        else if (key == "escapeForwardSlash")
            parseBool(value.escapeForwardSlash);
        else if (key == "skipEmptyFields")
            parseBool(value.skipEmptyFields);
        else if (key == "nanPolicy")
            parseSettingsNanPolicy(value.nanPolicy);
        else if (key == "infPolicy")
            parseSettingsInfPolicy(value.infPolicy);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationGeneratorDef(Configuration::GeneratorDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "baseClass")
            parseStdString(value.baseClass);
        else if (key == "types")
            parseStdVectorStdString(value.types);
        else if (key == "headerOutput")
            parseStdString(value.headerOutput);
        else if (key == "sourceOutput")
            parseStdString(value.sourceOutput);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationGeneratorDef(std::vector<Configuration::GeneratorDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationGeneratorDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStringAPI(StringAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "clear")
            parseStdString(value.clear);
        else if (key == "appendChar")
            parseStdString(value.appendChar);
        else if (key == "appendCStr")
            parseStdString(value.appendCStr);
        else if (key == "iterateChars")
            parseStdString(value.iterateChars);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationStringDef(Configuration::StringDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "api")
            parseStringAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationStringDef(std::vector<Configuration::StringDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationStringDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConstStringAPI(ConstStringAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "copyFromString")
            parseStdString(value.copyFromString);
        else if (key == "moveFromString")
            parseStdString(value.moveFromString);
        else if (key == "iterateChars")
            parseStdString(value.iterateChars);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationConstStringDef(Configuration::ConstStringDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "stringType")
            parseStdString(value.stringType);
        else if (key == "api")
            parseConstStringAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationConstStringDef(std::vector<Configuration::ConstStringDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationConstStringDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseArrayContainerAPI(ArrayContainerAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "clear")
            parseStdString(value.clear);
        else if (key == "refAppended")
            parseStdString(value.refAppended);
        else if (key == "iterateElements")
            parseStdString(value.iterateElements);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationArrayContainerDef(Configuration::ArrayContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "api")
            parseArrayContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationArrayContainerDef(std::vector<Configuration::ArrayContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationArrayContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseFixedArrayContainerAPI(FixedArrayContainerAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "copyFromArrayContainer")
            parseStdString(value.copyFromArrayContainer);
        else if (key == "moveFromArrayContainer")
            parseStdString(value.moveFromArrayContainer);
        else if (key == "iterateElements")
            parseStdString(value.iterateElements);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationFixedArrayContainerDef(Configuration::FixedArrayContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "arrayContainerType")
            parseStdString(value.arrayContainerType);
        else if (key == "api")
            parseFixedArrayContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationFixedArrayContainerDef(std::vector<Configuration::FixedArrayContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationFixedArrayContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStaticArrayContainerAPI(StaticArrayContainerAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "refByIndex")
            parseStdString(value.refByIndex);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationStaticArrayContainerDef(Configuration::StaticArrayContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "api")
            parseStaticArrayContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationStaticArrayContainerDef(std::vector<Configuration::StaticArrayContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationStaticArrayContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseObjectContainerAPI(ObjectContainerAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "clear")
            parseStdString(value.clear);
        else if (key == "refByKey")
            parseStdString(value.refByKey);
        else if (key == "iterateElements")
            parseStdString(value.iterateElements);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationObjectContainerDef(Configuration::ObjectContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "stringType")
            parseStdString(value.stringType);
        else if (key == "api")
            parseObjectContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationObjectContainerDef(std::vector<Configuration::ObjectContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationObjectContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationObjectMapContainerDef(Configuration::ObjectMapContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "api")
            parseObjectContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationObjectMapContainerDef(std::vector<Configuration::ObjectMapContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationObjectMapContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseOptionalContainerAPI(OptionalContainerAPI &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "clear")
            parseStdString(value.clear);
        else if (key == "refInitialized")
            parseStdString(value.refInitialized);
        else if (key == "hasValue")
            parseStdString(value.hasValue);
        else if (key == "getValue")
            parseStdString(value.getValue);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfigurationOptionalContainerDef(Configuration::OptionalContainerDef &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "name")
            parseStdString(value.name);
        else if (key == "api")
            parseOptionalContainerAPI(value.api);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseStdVectorConfigurationOptionalContainerDef(std::vector<Configuration::OptionalContainerDef> &value) {
    requireSymbol('[');
    value.clear();
    int separatorCheck = -1;
    while (!matchSymbol(']')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseConfigurationOptionalContainerDef((value.emplace_back(), value.back()));
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}

void ConfigurationParser::parseConfiguration(Configuration &value) {
    std::string key;
    requireSymbol('{');
    int separatorCheck = -1;
    while (!matchSymbol('}')) {
        if (!separatorCheck)
            throw Error::JSON_SYNTAX_ERROR;
        parseStdString(key);
        requireSymbol(':');
        if (key == "inputs")
            parseStdVectorStdString(value.inputs);
        else if (key == "includes")
            parseStdVectorStdString(value.includes);
        else if (key == "settings")
            parseSettings(value.settings);
        else if (key == "parsers")
            parseStdVectorConfigurationGeneratorDef(value.parsers);
        else if (key == "serializers")
            parseStdVectorConfigurationGeneratorDef(value.serializers);
        else if (key == "stringType")
            parseStdString(value.stringType);
        else if (key == "stringTypes")
            parseStdVectorConfigurationStringDef(value.stringTypes);
        else if (key == "constStringTypes")
            parseStdVectorConfigurationConstStringDef(value.constStringTypes);
        else if (key == "arrayContainerTypes")
            parseStdVectorConfigurationArrayContainerDef(value.arrayContainerTypes);
        else if (key == "fixedArrayContainerTypes")
            parseStdVectorConfigurationFixedArrayContainerDef(value.fixedArrayContainerTypes);
        else if (key == "staticArrayContainerTypes")
            parseStdVectorConfigurationStaticArrayContainerDef(value.staticArrayContainerTypes);
        else if (key == "objectContainerTypes")
            parseStdVectorConfigurationObjectContainerDef(value.objectContainerTypes);
        else if (key == "objectMapContainerTypes")
            parseStdVectorConfigurationObjectMapContainerDef(value.objectMapContainerTypes);
        else if (key == "optionalContainerTypes")
            parseStdVectorConfigurationOptionalContainerDef(value.optionalContainerTypes);
        else
            skipValue();
        separatorCheck = matchSymbol(',');
    }
    if (separatorCheck == 1)
        throw Error::JSON_SYNTAX_ERROR;
}
