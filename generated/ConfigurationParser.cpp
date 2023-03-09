
// Generated by json-cpp-gen by Viktor Chlumsky
// https://github.com/Chlumsky/json-cpp-gen

#include "ConfigurationParser.h"

ConfigurationParser::Error::operator ConfigurationParser::Error::Type() const {
    return type;
}

ConfigurationParser::Error::operator bool() const {
    return type != Error::OK;
}

const char *ConfigurationParser::Error::typeString() const {
    switch (type) {
        case Error::OK:
            return "OK";
        case Error::JSON_SYNTAX_ERROR:
            return "JSON_SYNTAX_ERROR";
        case Error::UNEXPECTED_END_OF_FILE:
            return "UNEXPECTED_END_OF_FILE";
        case Error::TYPE_MISMATCH:
            return "TYPE_MISMATCH";
        case Error::ARRAY_SIZE_MISMATCH:
            return "ARRAY_SIZE_MISMATCH";
        case Error::UNKNOWN_KEY:
            return "UNKNOWN_KEY";
        case Error::UNKNOWN_ENUM_VALUE:
            return "UNKNOWN_ENUM_VALUE";
        case Error::VALUE_OUT_OF_RANGE:
            return "VALUE_OUT_OF_RANGE";
        case Error::STRING_EXPECTED:
            return "STRING_EXPECTED";
        case Error::UTF16_ENCODING_ERROR:
            return "UTF16_ENCODING_ERROR";
    }
    return "";
}

ConfigurationParser::ConfigurationParser(const char *str) : cur(str) { }

void ConfigurationParser::skipWhitespace() {
    while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')
        ++cur;
}

void ConfigurationParser::skipValue() {
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
            ++cur;
            for (int openBrackets = 1; openBrackets;) {
                switch (*cur) {
                    case '\0':
                        throw Error::UNEXPECTED_END_OF_FILE;
                    case '"':
                        skipValue();
                        continue;
                    case '[': case '{':
                        ++openBrackets;
                        break;
                    case ']': case '}':
                        --openBrackets;
                        break;
                }
                ++cur;
            }
            return;
        default:
            if (isAlphanumeric(*cur) || *cur == '-' || *cur == '.') {
                while (isAlphanumeric(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
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

void ConfigurationParser::readHexQuad(int &value) {
    if (!(
        cur[0] && cur[1] && cur[2] && cur[3] &&
        (value = decodeHexDigit(cur[3])) >= 0 &&
        (value += 0x0010*decodeHexDigit(cur[2])) >= 0 &&
        (value += 0x0100*decodeHexDigit(cur[1])) >= 0 &&
        (value += 0x1000*decodeHexDigit(cur[0])) >= 0
    ))
        throw Error::JSON_SYNTAX_ERROR;
    cur += 4;
}

void ConfigurationParser::unescape(char *codepoints) {
    switch (++cur, *cur++) {
        case '\0':
            --cur;
            throw Error::UNEXPECTED_END_OF_FILE;
        case 'B': case 'b': codepoints[0] = '\b'; break;
        case 'F': case 'f': codepoints[0] = '\f'; break;
        case 'N': case 'n': codepoints[0] = '\n'; break;
        case 'R': case 'r': codepoints[0] = '\r'; break;
        case 'T': case 't': codepoints[0] = '\t'; break;
        case 'U': case 'u': {
            unsigned long cp;
            int wc;
            readHexQuad(wc);
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[0] == '\\' && (cur[1] == 'u' || cur[1] == 'U')))
                    throw Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) ((wc&0x03ff)<<10);
                cur += 2;
                readHexQuad(wc);
                if ((wc&0xfc00) != 0xdc00)
                    throw Error::UTF16_ENCODING_ERROR;
                cp = 0x010000+(cp|(unsigned long) (wc&0x03ff));
            } else
                cp = (unsigned long) wc;
            if (cp&0xffffff80) {
                int len;
                for (len = 1; cp>>(5*len+1) && len < 6; ++len);
                codepoints[0] = (char) (0xff<<(8-len)|cp>>6*(len-1));
                for (int i = 1; i < len; ++i)
                    *++codepoints = (char) (0x80|(cp>>6*(len-i-1)&0x3f));
            } else
                codepoints[0] = (char) cp;
            break;
        }
        default:
            codepoints[0] = cur[-1];
    }
    codepoints[1] = '\0';
}

bool ConfigurationParser::isAlphanumeric(char c) {
    switch (c) {
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
        case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
        case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return true;
        default:
            return false;
    }
}

int ConfigurationParser::decodeHexDigit(char digit) {
    switch (digit) {
        case '0': return 0x00;
        case '1': return 0x01;
        case '2': return 0x02;
        case '3': return 0x03;
        case '4': return 0x04;
        case '5': return 0x05;
        case '6': return 0x06;
        case '7': return 0x07;
        case '8': return 0x08;
        case '9': return 0x09;
        case 'A': case 'a': return 0x0a;
        case 'B': case 'b': return 0x0b;
        case 'C': case 'c': return 0x0c;
        case 'D': case 'd': return 0x0d;
        case 'E': case 'e': return 0x0e;
        case 'F': case 'f': return 0x0f;
    }
    return -1;
}

ConfigurationParser::Error ConfigurationParser::parse(Configuration &output, const char *jsonString) {
    ConfigurationParser parser(jsonString);
    try {
        parser.parseConfiguration(output);
    } catch (Error::Type error) {
        return Error(error, static_cast<int>(parser.cur-jsonString));
    }
    return Error(Error::OK, static_cast<int>(parser.cur-jsonString));
}

void ConfigurationParser::parseStdString(std::string &value) {
    skipWhitespace();
    if (*cur != '"')
        throw Error::STRING_EXPECTED;
    value.clear();
    ++cur;
    while (*cur != '"') {
        if (*cur == '\\') {
            char buffer[8];
            unescape(buffer);
            value += buffer;
            continue;
        }
        if (!*cur)
            throw Error::UNEXPECTED_END_OF_FILE;
        value.push_back(*cur);
        ++cur;
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
        parseStdString((value.resize(value.size()+1), value.back()));
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
    if (cur[0] == 'f' && cur[1] == 'a' && cur[2] == 'l' && cur[3] == 's' && cur[4] == 'e' && !isAlphanumeric(cur[5]) && cur[5] != '_' && ((cur += 5), true))
        value = false;
    else if (cur[0] == 't' && cur[1] == 'r' && cur[2] == 'u' && cur[3] == 'e' && !isAlphanumeric(cur[4]) && cur[4] != '_' && ((cur += 4), true))
        value = true;
    else
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
        else if (key == "replacementIncludes")
            parseStdVectorStdString(value.replacementIncludes);
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
        parseConfigurationGeneratorDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationStringDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationConstStringDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationArrayContainerDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationFixedArrayContainerDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationStaticArrayContainerDef((value.resize(value.size()+1), value.back()));
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
        else if (key == "keyType")
            parseStdString(value.keyType);
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
        parseConfigurationObjectContainerDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationObjectMapContainerDef((value.resize(value.size()+1), value.back()));
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
        parseConfigurationOptionalContainerDef((value.resize(value.size()+1), value.back()));
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
