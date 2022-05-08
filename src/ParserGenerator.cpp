
#include "ParserGenerator.h"

const char * const ParserGenerator::Error::JSON_SYNTAX_ERROR = "JSON_SYNTAX_ERROR";
const char * const ParserGenerator::Error::UNEXPECTED_END_OF_FILE = "UNEXPECTED_END_OF_FILE";
const char * const ParserGenerator::Error::TYPE_MISMATCH = "TYPE_MISMATCH";
const char * const ParserGenerator::Error::ARRAY_SIZE_MISMATCH = "ARRAY_SIZE_MISMATCH";
const char * const ParserGenerator::Error::UNKNOWN_KEY = "UNKNOWN_KEY";
const char * const ParserGenerator::Error::UNKNOWN_ENUM_VALUE = "UNKNOWN_ENUM_VALUE";
const char * const ParserGenerator::Error::VALUE_OUT_OF_RANGE = "VALUE_OUT_OF_RANGE";
const char * const ParserGenerator::Error::STRING_EXPECTED = "STRING_EXPECTED";
const char * const ParserGenerator::Error::UTF16_ENCODING_ERROR = "UTF16_ENCODING_ERROR";

static constexpr const char * const COMMON_FUNCTION_IMPL_NO_THROW =
R"($::$(const char *str) : cur(str) { }

void $::skipWhitespace() {
    while (*cur == ' ' || *cur == '\t' || *cur == '\r' || *cur == '\n')
        ++cur;
}

$::Error $::skipValue() {
    int openBrackets = 1;
    skipWhitespace();
    switch (*cur) {
        case '\0':
            return Error::UNEXPECTED_END_OF_FILE;
        case '"':
            while (*++cur != '"') {
                if (!*(cur += *cur == '\\'))
                    return Error::UNEXPECTED_END_OF_FILE;
            }
            ++cur;
            return Error::OK;
        case '[': case '{':
            while (openBrackets) {
                switch (*++cur) {
                    case '\0':
                        return Error::UNEXPECTED_END_OF_FILE;
                    case '"':
                        if (Error error = skipValue())
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
            return Error::OK;
        default:
            if (isAlphanumeric(*cur) || *cur == '-' || *cur == '.') {
                while (isAlphanumeric(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
                return Error::OK;
            }
    }
    return Error::JSON_SYNTAX_ERROR;
}

bool $::matchSymbol(char s) {
    skipWhitespace();
    if (*cur == s) {
        ++cur;
        return true;
    }
    return false;
}

$::Error $::unescape(char *codepoints) {
    switch (*++cur) {
        case '\0':
            return Error::UNEXPECTED_END_OF_FILE;
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
                return Error::JSON_SYNTAX_ERROR;
            codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
            codepoints[4] = '\0';
            cur += 3;
            if (sscanf(codepoints, "%hx", &wc) != 1)
                return Error::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    return Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    return Error::JSON_SYNTAX_ERROR;
                codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
                codepoints[4] = '\0';
                cur += 3;
                if (sscanf(codepoints, "%hx", &wc) != 1)
                    return Error::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    return Error::UTF16_ENCODING_ERROR;
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
    return Error::OK;
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
            if (isAlphanumeric(*cur) || *cur == '-' || *cur == '.') {
                while (isAlphanumeric(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
                return;
            }
    }
    throw Error::JSON_SYNTAX_ERROR;
}

void $::requireSymbol(char s) {
    skipWhitespace();
    if (*cur++ != s)
        throw Error::JSON_SYNTAX_ERROR;
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
            throw Error::UNEXPECTED_END_OF_FILE;
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
                throw Error::JSON_SYNTAX_ERROR;
            codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
            codepoints[4] = '\0';
            cur += 3;
            if (sscanf(codepoints, "%hx", &wc) != 1)
                throw Error::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    throw Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    throw Error::JSON_SYNTAX_ERROR;
                codepoints[0] = cur[0], codepoints[1] = cur[1], codepoints[2] = cur[2], codepoints[3] = cur[3];
                codepoints[4] = '\0';
                cur += 3;
                if (sscanf(codepoints, "%hx", &wc) != 1)
                    throw Error::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    throw Error::UTF16_ENCODING_ERROR;
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
        generateParserFunctionCall(type, "value");
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
        return indent+"if (Error error = "+generateParserFunctionCall(type, outputArg)+")\n"+
            indent+INDENT "return error;\n";
    } else
        return indent+generateParserFunctionCall(type, outputArg)+";\n";
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
    code += INDENT "enum Error {\n";
    code += INDENT INDENT "OK,\n";
    code += std::string(INDENT INDENT)+Error::JSON_SYNTAX_ERROR+",\n";
    code += std::string(INDENT INDENT)+Error::UNEXPECTED_END_OF_FILE+",\n";
    code += std::string(INDENT INDENT)+Error::TYPE_MISMATCH+",\n";
    code += std::string(INDENT INDENT)+Error::ARRAY_SIZE_MISMATCH+",\n";
    code += std::string(INDENT INDENT)+Error::UNKNOWN_KEY+",\n";
    code += std::string(INDENT INDENT)+Error::UNKNOWN_ENUM_VALUE+",\n";
    code += std::string(INDENT INDENT)+Error::VALUE_OUT_OF_RANGE+",\n";
    code += std::string(INDENT INDENT)+Error::STRING_EXPECTED+",\n";
    code += std::string(INDENT INDENT)+Error::UTF16_ENCODING_ERROR+",\n";
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
    code += std::string(INDENT)+(settings().noThrow ? "Error" : "void")+" skipValue();\n";
    if (!settings().noThrow)
        code += INDENT "void requireSymbol(char s);\n";
    code += INDENT "bool matchSymbol(char s);\n";
    code += std::string(INDENT)+(settings().noThrow ? "Error" : "void")+" unescape(char *codepoints);\n";
    code += INDENT "static bool isAlphanumeric(char c);\n";
    code += "\n";
    for (const Function &parseFunction : functions)
        code += std::string(INDENT)+(settings().noThrow ? "Error " : "void ")+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+");\n";
    code += "\n};\n";
    code += endNamespace();
    return code;
}

std::string ParserGenerator::generateSource() {
    return generateSource(className+".h");
}

std::string ParserGenerator::generateSource(const std::string &relativeHeaderAddress) {
    std::string code;
    code += "\n#include \""+relativeHeaderAddress+"\"\n\n";
    code += signature;
    code += beginNamespace();
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
    // public parse functions
    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end()) {
            code += "\n";
            code += className+"::Error "+className+"::parse("+type->name().refArgDeclaration("output")+", const char *jsonString) {\n";
            if (settings().noThrow)
                code += INDENT "return "+className+"(jsonString)."+it->second+"(output);\n";
            else {
                code += INDENT "try {\n";
                code += INDENT INDENT+className+"(jsonString)."+it->second+"(output);\n";
                code += INDENT "} catch (Error error) {\n";
                code += INDENT INDENT "return error;\n";
                code += INDENT "}\n";
                code += INDENT "return Error::OK;\n";
            }
            code += "}\n";
        }
    }
    // private parse functions
    for (const Function &parseFunction : functions) {
        code += "\n";
        if (settings().noThrow)
            code += className+"::Error ";
        else
            code += "void ";
        code += className+"::"+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+") {\n";
        code += parseFunction.body;
        if (settings().noThrow)
            code += INDENT "return Error::OK;\n";
        code += "}\n";
    }
    code += endNamespace();
    return code;
}
