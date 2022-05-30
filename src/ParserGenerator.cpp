
#include "ParserGenerator.h"

#define PARSER_GENERATOR_ERROR_STR_INSTANTIATE(e) const char * const ParserGenerator::Error::e = #e;
FOR_PARSER_ERRORS(PARSER_GENERATOR_ERROR_STR_INSTANTIATE)

const unsigned ParserGenerator::FEATURE_READ_SIGNED = 0x0100;
const unsigned ParserGenerator::FEATURE_READ_UNSIGNED = 0x0200;

static constexpr const char * const COMMON_FUNCTION_IMPL_NO_THROW =
R"($::$(const char *str) : cur(str) { }

void $::skipWhitespace() {
    while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')
        ++cur;
}

$::ErrorType $::skipValue() {
    int openBrackets = 1;
    skipWhitespace();
    switch (*cur) {
        case '\0':
            return ErrorType::UNEXPECTED_END_OF_FILE;
        case '"':
            while (*++cur != '"') {
                if (!*(cur += *cur == '\\'))
                    return ErrorType::UNEXPECTED_END_OF_FILE;
            }
            ++cur;
            return ErrorType::OK;
        case '[': case '{':
            while (openBrackets) {
                switch (*++cur) {
                    case '\0':
                        return ErrorType::UNEXPECTED_END_OF_FILE;
                    case '"':
                        if (ErrorType error = skipValue())
                            return error;
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
            return ErrorType::OK;
        default:
            if (isAlphanumeric(*cur) || *cur == '-' || *cur == '.') {
                while (isAlphanumeric(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
                return ErrorType::OK;
            }
    }
    return ErrorType::JSON_SYNTAX_ERROR;
}

bool $::matchSymbol(char s) {
    skipWhitespace();
    if (*cur == s) {
        ++cur;
        return true;
    }
    return false;
}

$::ErrorType $::unescape(char *codepoints) {
    switch (*++cur) {
        case '\0':
            return ErrorType::UNEXPECTED_END_OF_FILE;
        case 'B': case 'b': codepoints[0] = '\b'; break;
        case 'F': case 'f': codepoints[0] = '\f'; break;
        case 'N': case 'n': codepoints[0] = '\n'; break;
        case 'R': case 'r': codepoints[0] = '\r'; break;
        case 'T': case 't': codepoints[0] = '\t'; break;
        case 'U': case 'u': {
            unsigned long cp;
            unsigned short wc;
            ++cur;
            if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                return ErrorType::JSON_SYNTAX_ERROR;
            codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
            codepoints[4] = '\0';
            cur += 3;
            if (sscanf(codepoints, "%hx", &wc) != 1)
                return ErrorType::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    return ErrorType::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    return ErrorType::JSON_SYNTAX_ERROR;
                codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
                codepoints[4] = '\0';
                cur += 3;
                if (sscanf(codepoints, "%hx", &wc) != 1)
                    return ErrorType::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    return ErrorType::UTF16_ENCODING_ERROR;
                cp = 0x010000+(cp|(unsigned long) (wc&0x03ff));
            } else
                cp = wc;
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
            codepoints[0] = *cur;
    }
    codepoints[1] = '\0';
    return ErrorType::OK;
}
)";

static constexpr const char * const COMMON_FUNCTION_IMPL_THROW =
R"($::$(const char *str) : cur(str) { }

void $::skipWhitespace() {
    while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')
        ++cur;
}

void $::skipValue() {
    int openBrackets = 1;
    skipWhitespace();
    switch (*cur) {
        case '\0':
            throw ErrorType::UNEXPECTED_END_OF_FILE;
        case '"':
            while (*++cur != '"') {
                if (!*(cur += *cur == '\\'))
                    throw ErrorType::UNEXPECTED_END_OF_FILE;
            }
            ++cur;
            return;
        case '[': case '{':
            while (openBrackets) {
                switch (*++cur) {
                    case '\0':
                        throw ErrorType::UNEXPECTED_END_OF_FILE;
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
            if (isAlphanumeric(*cur) || *cur == '-' || *cur == '.') {
                while (isAlphanumeric(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
                return;
            }
    }
    throw ErrorType::JSON_SYNTAX_ERROR;
}

void $::requireSymbol(char s) {
    skipWhitespace();
    if (*cur++ != s)
        throw ErrorType::JSON_SYNTAX_ERROR;
}

bool $::matchSymbol(char s) {
    skipWhitespace();
    if (*cur == s) {
        ++cur;
        return true;
    }
    return false;
}

void $::unescape(char *codepoints) {
    switch (*++cur) {
        case '\0':
            throw ErrorType::UNEXPECTED_END_OF_FILE;
        case 'B': case 'b': codepoints[0] = '\b'; break;
        case 'F': case 'f': codepoints[0] = '\f'; break;
        case 'N': case 'n': codepoints[0] = '\n'; break;
        case 'R': case 'r': codepoints[0] = '\r'; break;
        case 'T': case 't': codepoints[0] = '\t'; break;
        case 'U': case 'u': {
            unsigned long cp;
            unsigned short wc;
            ++cur;
            if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                throw ErrorType::JSON_SYNTAX_ERROR;
            codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
            codepoints[4] = '\0';
            cur += 3;
            if (sscanf(codepoints, "%hx", &wc) != 1)
                throw ErrorType::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    throw ErrorType::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    throw ErrorType::JSON_SYNTAX_ERROR;
                codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
                codepoints[4] = '\0';
                cur += 3;
                if (sscanf(codepoints, "%hx", &wc) != 1)
                    throw ErrorType::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    throw ErrorType::UTF16_ENCODING_ERROR;
                cp = 0x010000+(cp|(unsigned long) (wc&0x03ff));
            } else
                cp = wc;
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
            codepoints[0] = *cur;
    }
    codepoints[1] = '\0';
}
)";

std::string ParserGenerator::generateMatchKeyword(const char *keyword) {
    std::string expr;
    int i;
    for (i = 0; *keyword; ++i, ++keyword)
        expr += "cur["+std::to_string(i)+"] == '"+*keyword+"' && ";
    std::string iStr = std::to_string(i);
    expr += "!isAlphanumeric(cur["+iStr+"]) && cur["+iStr+"] != '_' && ((cur += "+iStr+"), true)";
    return expr;
}

ParserGenerator::ParserGenerator(const std::string &className, const StringType *stringType, const Settings &settings) : Generator(className, stringType, settings) { }

void ParserGenerator::generateParserFunction(const Type *type) {
    if (type) {
        generateParserFunctionCall(type, "");
        entryTypes.push_back(type);
    }
}

std::string ParserGenerator::generateParserFunctionCall(const Type *type, const std::string &outputArg) {
    if (!type)
        return std::string();
    std::string &functionName = functionNames[type->name().fullName()];
    if (functionName.empty()) {
        functionName = generateFunctionName("parse", type);
        Function parseFunction;
        parseFunction.type = type;
        parseFunction.name = functionName;
        parseFunction.body = type->generateParserFunctionBody(this, INDENT);
        functions.push_back(std::move(parseFunction));
    }
    return functionName+"("+outputArg+")";
}

std::string ParserGenerator::generateValueParse(const Type *type, const std::string &outputArg, const std::string &indent) {
    if (settings().noThrow) {
        return indent+"if (ErrorType error = "+generateParserFunctionCall(type, outputArg)+")\n"+
            indent+INDENT "return error;\n";
    } else
        return indent+generateParserFunctionCall(type, outputArg)+";\n";
}

std::string ParserGenerator::generateErrorStatement(const char *errorName) const {
    return std::string(settings().noThrow ? "return" : "throw")+" ErrorType::"+errorName;
}

std::string ParserGenerator::generateHeader() {
    std::string code;
    code += "\n#pragma once\n\n";
    for (const std::string &typeInclude : typeIncludes)
        code += "#include "+typeInclude+"\n";
    if (!typeIncludes.empty())
        code += "\n";
    code += beginNamespace();
    code += signature;
    code += "class "+className+" {\n";

    code += "\npublic:\n";
    code += INDENT "enum ErrorType {\n";
    code += INDENT INDENT "OK,\n";
    #define PARSER_GENERATOR_APPEND_ERROR_NAME(e) code += std::string(INDENT INDENT)+Error::e+",\n";
    FOR_PARSER_ERRORS(PARSER_GENERATOR_APPEND_ERROR_NAME)
    code += INDENT "};\n\n";

    code += INDENT "struct Error {\n";
    code += INDENT INDENT "ErrorType type;\n";
    code += INDENT INDENT "int position;\n\n";
    code += INDENT INDENT "inline Error(ErrorType type = ErrorType::OK, int position = -1) : type(type), position(position) { }\n";
    code += INDENT INDENT "operator ErrorType() const;\n";
    code += INDENT INDENT "operator bool() const;\n";
    code += INDENT INDENT "const char *typeString() const;\n";
    code += INDENT "};\n\n";

    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end())
            code += INDENT "static Error parse("+type->name().refArgDeclaration("output")+", const char *jsonString);\n";
    }

    code += "\nprotected:\n";
    code += INDENT "const char *cur;\n\n";
    code += INDENT "explicit "+className+"(const char *str);\n";
    code += INDENT "void skipWhitespace();\n";
    code += std::string(INDENT)+(settings().noThrow ? "ErrorType" : "void")+" skipValue();\n";
    if (!settings().noThrow)
        code += INDENT "void requireSymbol(char s);\n";
    code += INDENT "bool matchSymbol(char s);\n";
    code += std::string(INDENT)+(settings().noThrow ? "ErrorType" : "void")+" unescape(char *codepoints);\n";
    code += INDENT "static bool isAlphanumeric(char c);\n";
    code += "\n";

    for (const Function &parseFunction : functions)
        code += std::string(INDENT)+(settings().noThrow ? "ErrorType " : "void ")+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+");\n";

    if (featureBits&(FEATURE_READ_SIGNED|FEATURE_READ_UNSIGNED)) {
        code += "\nprivate:\n";
        if (featureBits&FEATURE_READ_SIGNED) {
            code += INDENT "template <typename T>\n";
            code += std::string(INDENT)+(settings().noThrow ? "ErrorType" : "void")+" readSigned(T &value);\n";
        }
        if (featureBits&FEATURE_READ_UNSIGNED) {
            code += INDENT "template <typename T>\n";
            code += std::string(INDENT)+(settings().noThrow ? "ErrorType" : "void")+" readUnsigned(T &value);\n";
        }
    }
    code += "\n};\n";
    code += endNamespace();
    return code;
}

std::string ParserGenerator::generateSource() {
    return generateSource(className+".h");
}

std::string ParserGenerator::generateSource(const std::string &relativeHeaderAddress) {
    std::string code;
    code += "\n";
    if (featureBits&FEATURE_CSTDLIB)
        code += "#include <cstdlib>\n";
    code += "#include <cstdio>\n"; // Currently always required for unescape function
    code += "#include \""+relativeHeaderAddress+"\"\n\n";
    code += signature;
    code += beginNamespace();

    // Error member functions
    code += className+"::Error::operator "+className+"::ErrorType() const {\n" INDENT "return type;\n}\n\n";
    code += className+"::Error::operator bool() const {\n" INDENT "return type != ErrorType::OK;\n}\n\n";
    code += "const char *"+className+"::Error::typeString() const {\n";
    code += INDENT "switch (type) {\n";
    #define PARSER_GENERATOR_ERROR_TYPE_STRING_CASE(e) code += INDENT INDENT "case ErrorType::" #e ":\n" INDENT INDENT INDENT "return \"" #e "\";\n";
    PARSER_GENERATOR_ERROR_TYPE_STRING_CASE(OK)
    FOR_PARSER_ERRORS(PARSER_GENERATOR_ERROR_TYPE_STRING_CASE)
    code += INDENT "}\n";
    code += INDENT "return \"\";\n";
    code += "}\n\n";

    // Common functions
    for (const char *c = settings().noThrow ? COMMON_FUNCTION_IMPL_NO_THROW : COMMON_FUNCTION_IMPL_THROW; *c; ++c) {
        if (*c == '$')
            code += className;
        else
            code.push_back(*c);
    }

    // isAlphanumeric
    code += "\n";
    code += "bool "+className+"::isAlphanumeric(char c) {\n";
    code += INDENT "switch (c) {";
    for (int i = 0; i < 26+26+10; ++i) {
        if (!(i%26%9) && i != 26+26+9)
            code += "\n" INDENT INDENT;
        else
            code.push_back(' ');
        code += std::string("case '")+"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[i]+"':";
    }
    code += "\n" INDENT INDENT INDENT "return true;\n";
    code += INDENT INDENT "default:\n";
    code += INDENT INDENT INDENT "return false;\n";
    code += INDENT "}\n";
    code += "}\n";

    // Integer read functions
    std::string readUnsignedBody;
    if (featureBits&(FEATURE_READ_SIGNED|FEATURE_READ_UNSIGNED)) {
        readUnsignedBody += INDENT "if (*cur >= '0' && *cur <= '9')\n";
        readUnsignedBody += INDENT INDENT "value = *cur++-'0';\n";
        readUnsignedBody += INDENT "else\n";
        readUnsignedBody += INDENT INDENT+generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
        readUnsignedBody += INDENT "while (*cur >= '0' && *cur <= '9')";
        if (settings().checkIntegerOverflow) {
            readUnsignedBody += " {\n";
            readUnsignedBody += INDENT INDENT "if (10*value < value)\n";
            readUnsignedBody += INDENT INDENT INDENT+generateErrorStatement(ParserGenerator::Error::VALUE_OUT_OF_RANGE)+";\n";
            readUnsignedBody += INDENT INDENT "value = 10*value+(*cur++-'0');\n";
            readUnsignedBody += INDENT "}\n";
        } else
            readUnsignedBody += "\n" INDENT INDENT "value = 10*value+(*cur++-'0');\n";
    }
    if (featureBits&FEATURE_READ_SIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += (settings().noThrow ? className+"::ErrorType " : "void ")+className+"::readSigned(T &value) {\n";
        code += INDENT "bool negative = *cur == '-' && (++cur, true);\n";
        code += readUnsignedBody;
        code += INDENT "if (negative)\n";
        code += INDENT INDENT "value = -value;\n";
        if (settings().noThrow)
            code += INDENT "return ErrorType::OK;\n";
        code += "}\n";
    }
    if (featureBits&FEATURE_READ_UNSIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += (settings().noThrow ? className+"::ErrorType " : "void ")+className+"::readUnsigned(T &value) {\n";
        code += readUnsignedBody;
        if (settings().noThrow)
            code += INDENT "return ErrorType::OK;\n";
        code += "}\n";
    }

    // Public parse functions
    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end()) {
            code += "\n";
            code += className+"::Error "+className+"::parse("+type->name().refArgDeclaration("output")+", const char *jsonString) {\n";
            code += INDENT+className+" parser(jsonString);\n";
            if (settings().noThrow) {
                code += INDENT "ErrorType error = parser."+it->second+"(output);\n";
                code += INDENT "return Error(error, static_cast<int>(parser.cur-jsonString));\n";
            } else {
                code += INDENT "try {\n";
                code += INDENT INDENT "parser."+it->second+"(output);\n";
                code += INDENT "} catch (ErrorType error) {\n";
                code += INDENT INDENT "return Error(error, static_cast<int>(parser.cur-jsonString));\n";
                code += INDENT "}\n";
                code += INDENT "return Error(ErrorType::OK, static_cast<int>(parser.cur-jsonString));\n";
            }
            code += "}\n";
        }
    }

    // Private parse functions
    for (const Function &parseFunction : functions) {
        code += "\n";
        if (settings().noThrow)
            code += className+"::ErrorType ";
        else
            code += "void ";
        code += className+"::"+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+") {\n";
        code += parseFunction.body;
        if (settings().noThrow)
            code += INDENT "return ErrorType::OK;\n";
        code += "}\n";
    }
    code += endNamespace();
    return code;
}
