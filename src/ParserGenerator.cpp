
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
            return Error::OK;
        default:
            if (isalnum(*cur) || *cur == '-' || *cur == '.') {
                while (isalnum(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
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

$::Error $::parseEscaped(char *sequence) {
    switch (*++cur) {
        case '\0':
            return Error::UNEXPECTED_END_OF_FILE;
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
                return Error::JSON_SYNTAX_ERROR;
            sequence[0] = cur[0], sequence[1] = cur[1], sequence[2] = cur[2], sequence[3] = cur[3];
            sequence[4] = '\0';
            cur += 3;
            if (sscanf(sequence, "%hx", &wc) != 1)
                return Error::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[1] == '\\' && (cur[2] == 'u' || cur[2] == 'U')))
                    return Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) (wc&0x03ff)<<10;
                cur += 3;
                if (!(cur[0] && cur[1] && cur[2] && cur[3]))
                    return Error::JSON_SYNTAX_ERROR;
                sequence[0] = cur[0], sequence[1] = cur[1], sequence[2] = cur[2], sequence[3] = cur[3];
                sequence[4] = '\0';
                cur += 3;
                if (sscanf(sequence, "%hx", &wc) != 1)
                    return Error::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    return Error::UTF16_ENCODING_ERROR;
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
            if (isalnum(*cur) || *cur == '-' || *cur == '.') {
                while (isalnum(*++cur) || *cur == '+' || *cur == '-' || *cur == '.');
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

void $::parseEscaped(char *sequence) {
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
)";

std::string ParserGenerator::generateMatchKeyword(const char *keyword) {
    std::string expr;
    int i;
    for (i = 0; *keyword; ++i, ++keyword)
        expr += "cur["+std::to_string(i)+"] == '"+*keyword+"' && ";
    std::string iStr = std::to_string(i);
    expr += "!isalnum(cur["+iStr+"]) && cur["+iStr+"] != '_' && ((cur += "+iStr+"), true)";
    return expr;
}

ParserGenerator::ParserGenerator(const std::string &className, const StringType *stringType, const Settings &settings) : Generator(className, stringType, settings) { }

void ParserGenerator::generateParserFunction(const Type *type) {
    generateParserFunctionCall(type, "value", true);
}

std::string ParserGenerator::generateParserFunctionCall(const Type *type, const std::string &outputArg, bool rootStructure) {
    if (!type)
        return std::string();
    std::string &functionName = functionNames[type->name().fullName()];
    if (functionName.empty()) {
        functionName = generateFunctionName("parse", type);
        Function parseFunction;
        parseFunction.type = type;
        parseFunction.name = functionName;
        parseFunction.body = type->generateParserFunctionBody(this, INDENT);
        parseFunction.rootStructure = rootStructure;
        functions.push_back(std::move(parseFunction));
    }
    return functionName+"("+outputArg+")";
}

std::string ParserGenerator::generateValueParse(const Type *type, const std::string &outputArg, const std::string &indent) {
    if (settings().noThrow) {
        return indent+"if (Error error = "+generateParserFunctionCall(type, outputArg, false)+")\n"+
            indent+INDENT "return error;\n";
    } else
        return indent+generateParserFunctionCall(type, outputArg, false)+";\n";
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
    for (const Function &parseFunction : functions) {
        if (parseFunction.rootStructure)
            code += INDENT "static Error parse("+parseFunction.type->name().refArgDeclaration("output")+", const char *jsonString);\n";
    }
    code += "\nprotected:\n";
    code += INDENT "const char *cur;\n\n";
    code += INDENT "explicit "+className+"(const char *str);\n";
    code += INDENT "void skipWhitespace();\n";
    code += std::string(INDENT)+(settings().noThrow ? "Error" : "void")+" skipValue();\n";
    if (!settings().noThrow)
        code += INDENT "void requireSymbol(char s);\n";
    code += INDENT "bool matchSymbol(char s);\n";
    code += std::string(INDENT)+(settings().noThrow ? "Error" : "void")+" parseEscaped(char *sequence);\n";
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
    for (const Function &parseFunction : functions) {
        if (parseFunction.rootStructure) {
            code += "\n";
            code += className+"::Error "+className+"::parse("+parseFunction.type->name().refArgDeclaration("output")+", const char *jsonString) {\n";
            if (settings().noThrow)
                code += INDENT "return "+className+"(jsonString)."+parseFunction.name+"(output);\n";
            else {
                code += INDENT "try {\n";
                code += INDENT INDENT+className+"(jsonString)."+parseFunction.name+"(output);\n";
                code += INDENT "} catch (Error error) {\n";
                code += INDENT INDENT "return error;\n";
                code += INDENT "}\n";
                code += INDENT "return Error::OK;\n";
            }
            code += "}\n";
        }
    }
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
