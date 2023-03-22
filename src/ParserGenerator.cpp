
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

$::Error::Type $::skipValue() {
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
            ++cur;
            for (int openBrackets = 1; openBrackets;) {
                switch (*cur) {
                    case '\0':
                        return Error::UNEXPECTED_END_OF_FILE;
                    case '"':
                        if (Error::Type error = skipValue())
                            return error;
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

bool $::readHexQuad(int &value) {
    return (
        cur[0] && cur[1] && cur[2] && cur[3] &&
        (value = decodeHexDigit(cur[3])) >= 0 &&
        (value += 0x0010*decodeHexDigit(cur[2])) >= 0 &&
        (value += 0x0100*decodeHexDigit(cur[1])) >= 0 &&
        (value += 0x1000*decodeHexDigit(cur[0])) >= 0 &&
        (cur += 4, true)
    );
}

$::Error::Type $::unescape(char *codepoints) {
    switch (++cur, *cur++) {
        case '\0':
            --cur;
            return Error::UNEXPECTED_END_OF_FILE;
        case 'B': case 'b': codepoints[0] = '\b'; break;
        case 'F': case 'f': codepoints[0] = '\f'; break;
        case 'N': case 'n': codepoints[0] = '\n'; break;
        case 'R': case 'r': codepoints[0] = '\r'; break;
        case 'T': case 't': codepoints[0] = '\t'; break;
        case 'U': case 'u': {
            unsigned long cp;
            int wc;
            if (!readHexQuad(wc))
                return Error::JSON_SYNTAX_ERROR;
            if ((wc&0xfc00) == 0xd800) {
                if (!(cur[0] == '\\' && (cur[1] == 'u' || cur[1] == 'U')))
                    return Error::UTF16_ENCODING_ERROR;
                cp = (unsigned long) ((wc&0x03ff)<<10);
                cur += 2;
                if (!readHexQuad(wc))
                    return Error::JSON_SYNTAX_ERROR;
                if ((wc&0xfc00) != 0xdc00)
                    return Error::UTF16_ENCODING_ERROR;
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

bool $::matchSymbol(char s) {
    skipWhitespace();
    if (*cur == s) {
        ++cur;
        return true;
    }
    return false;
}

void $::readHexQuad(int &value) {
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

void $::unescape(char *codepoints) {
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
)";

std::string ParserGenerator::generateMatchKeyword(const char *keyword) {
    std::string expr;
    int i;
    for (i = 0; *keyword; ++i, ++keyword)
        expr += "cur["+std::to_string(i)+"] == '"+*keyword+"' && ";
    std::string iStr = std::to_string(i);
    expr += "!isAlphanumeric(cur["+iStr+"]) && cur["+iStr+"] != '_' && (cur += "+iStr+", true)";
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
        return indent+"if (Error::Type error = "+generateParserFunctionCall(type, outputArg)+")\n"+
            indent+INDENT "return error;\n";
    } else
        return indent+generateParserFunctionCall(type, outputArg)+";\n";
}

std::string ParserGenerator::generateErrorStatement(const char *errorName) const {
    return std::string(settings().noThrow ? "return" : "throw")+" Error::"+errorName;
}

std::string ParserGenerator::generateSwitchTree(SwitchTreeCaseGenerator *caseGenerator, const StringSwitchTree *switchTree, const StringType *valueType, const char *value, const std::string &indent, int knownMinLength) {
    std::string body;
    if (switchTree) {
        if (switchTree->position == StringSwitchTree::LEAF_NODE_MARKER)
            return (*caseGenerator)(this, switchTree->label, valueType, value, knownMinLength, indent);
        if (switchTree->position == StringSwitchTree::LENGTH_SWITCH_MARKER) {
            body += indent+"switch ("+valueType->generateGetLength(value)+") {\n";
            for (const std::map<int, std::unique_ptr<StringSwitchTree> >::value_type &branch : switchTree->branches) {
                body += indent+INDENT "case "+std::to_string(branch.first)+":\n";
                body += generateSwitchTree(caseGenerator, branch.second.get(), valueType, value, indent+INDENT INDENT, branch.first > knownMinLength ? branch.first : knownMinLength);
                body += indent+INDENT INDENT "break;\n";
            }
            body += indent+"}\n";
        } else if (switchTree->position >= 0) {
            std::string switchIndent = indent;
            bool lengthCondition = switchTree->position >= knownMinLength;
            if (lengthCondition) {
                body += indent+"if ("+valueType->generateGetLength(value)+" > "+std::to_string(switchTree->position)+") {\n";
                knownMinLength = switchTree->position+1;
                switchIndent += INDENT;
            }
            body += switchIndent+"switch ("+valueType->generateGetCharAt(value, std::to_string(switchTree->position).c_str())+") {\n";
            for (const std::map<int, std::unique_ptr<StringSwitchTree> >::value_type &branch : switchTree->branches) {
                body += switchIndent+INDENT "case "+Generator::charLiteral(char(branch.first))+":\n";
                body += generateSwitchTree(caseGenerator, branch.second.get(), valueType, value, switchIndent+INDENT INDENT, knownMinLength);
                body += switchIndent+INDENT INDENT "break;\n";
            }
            body += switchIndent+"}\n";
            if (lengthCondition)
                body += indent+"}\n";
        }
    }
    return body;
}

std::string ParserGenerator::generateReadIntegerBody(bool signedInt) const {
    // This implementation requires that '1'-'0' == 1, '2'-'0' == 2, ... , '9'-'0' == 9
    std::string body;
    if (signedInt)
        body += INDENT "bool negative = *cur == '-' && (++cur, true);\n";
    body += INDENT "if (*cur >= '0' && *cur <= '9')\n";
    body += INDENT INDENT "value = *cur++-'0';\n";
    body += INDENT "else\n";
    body += INDENT INDENT+generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += INDENT "while (*cur >= '0' && *cur <= '9')";
    if (settings().checkIntegerOverflow) {
        body += " {\n";
        body += INDENT INDENT "if (";
        if (signedInt) {
            // value has already overflown to the lowest representable negative value (e.g. -128)
            body += "value < 0 || (";
            // Optional: Necessary overflow condition evaluated first to avoid evaluating multiple
            body += "value >= JSON_CPP_MAX_INTEGER(T)/10 && (";
            // Adding another digit in this case will definitely overflow
            body += "value > JSON_CPP_MAX_INTEGER(T)/10 || (";
            // Only the remaining cases can be found by checking that value becomes lower after overflow
            body += "static_cast<T>(10*value+(*cur-'0')) < value && (";
            // Special exception - but only if negative:
            body += "!negative || ";
            // value will overflow exactly to -(JSON_CPP_MAX_INTEGER(T)+1) which is OK
            body += "*cur-'0' != (JSON_CPP_MAX_INTEGER(T)-9)%10)))))\n";
        } else {
            // Optional: Necessary overflow condition evaluated first to avoid evaluating multiple
            body += "value >= JSON_CPP_MAX_INTEGER(T)/10 && (";
            // Adding another digit in this case will definitely overflow
            body += "value > JSON_CPP_MAX_INTEGER(T)/10 || ";
            // Only the remaining cases can be found by checking that value becomes lower after overflow
            body += "static_cast<T>(10*value+(*cur-'0')) < value))\n";
        }
        body += INDENT INDENT INDENT+generateErrorStatement(ParserGenerator::Error::VALUE_OUT_OF_RANGE)+";\n";
        body += INDENT INDENT "value = static_cast<T>(10*value+(*cur++-'0'));\n";
        body += INDENT "}\n";
    } else
        body += "\n" INDENT INDENT "value = static_cast<T>(10*value+(*cur++-'0'));\n";
    if (signedInt) {
        body += INDENT "if (negative)\n";
        body += INDENT INDENT "value = -value;\n";
    }
    if (settings().noThrow)
        body += INDENT "return Error::OK;\n";
    return body;
}

std::string ParserGenerator::generateHeader() {
    std::string code;
    code += "\n";
    code += signature;
    code += "#pragma once\n\n";
    for (const std::string &typeInclude : typeIncludes)
        code += "#include "+typeInclude+"\n";
    if (!typeIncludes.empty())
        code += "\n";
    code += beginNamespace();
    code += "class "+className+" {\n";

    code += "\npublic:\n";
    code += INDENT "struct Error {\n";
    code += INDENT INDENT "enum Type {\n";
    code += INDENT INDENT INDENT "OK";
    #define PARSER_GENERATOR_APPEND_ERROR_NAME(e) code += std::string(",\n" INDENT INDENT INDENT)+Error::e;
    FOR_PARSER_ERRORS(PARSER_GENERATOR_APPEND_ERROR_NAME)
    code += "\n" INDENT INDENT "} type;\n";
    code += INDENT INDENT "int position;\n\n";
    code += INDENT INDENT "inline Error(Type type = Error::OK, int position = -1) : type(type), position(position) { }\n";
    code += INDENT INDENT "operator Type() const;\n";
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
    code += std::string(INDENT)+(settings().noThrow ? "Error::Type" : "void")+" skipValue();\n";
    code += INDENT "bool matchSymbol(char s);\n";
    code += std::string(INDENT)+(settings().noThrow ? "bool" : "void")+" readHexQuad(int &value);\n";
    code += std::string(INDENT)+(settings().noThrow ? "Error::Type" : "void")+" unescape(char *codepoints);\n";
    code += INDENT "static bool isAlphanumeric(char c);\n";
    code += INDENT "static int decodeHexDigit(char digit);\n";
    code += "\n";

    for (const Function &parseFunction : functions)
        code += std::string(INDENT)+(settings().noThrow ? "Error::Type " : "void ")+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+");\n";

    if (featureBits&(FEATURE_READ_SIGNED|FEATURE_READ_UNSIGNED)) {
        code += "\nprivate:\n";
        if (featureBits&FEATURE_READ_SIGNED) {
            code += INDENT "template <typename T>\n";
            code += std::string(INDENT)+(settings().noThrow ? "Error::Type" : "void")+" readSigned(T &value);\n";
        }
        if (featureBits&FEATURE_READ_UNSIGNED) {
            code += INDENT "template <typename T>\n";
            code += std::string(INDENT)+(settings().noThrow ? "Error::Type" : "void")+" readUnsigned(T &value);\n";
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
    code += signature;
    if (featureBits&FEATURE_CSTDLIB)
        code += "#include <cstdlib>\n";
    code += "#include \""+relativeHeaderAddress+"\"\n\n";

    if ((featureBits&(FEATURE_READ_SIGNED|FEATURE_READ_UNSIGNED)) && settings().checkIntegerOverflow) {
        code += "#ifndef JSON_CPP_MAX_INTEGER\n";
        // This requires that integer type T is encoded as two's complement or unsigned with exactly 8*sizeof(T) binary digits
        code += "#define JSON_CPP_MAX_INTEGER(T) ((T) ~(((T) ~(T) 0 <= (T) 0 ? -2 : 0)*((T) 1<<(8*sizeof(T)-2))))\n";
        code += "#endif\n\n";
    }

    code += beginNamespace();

    // Error member functions
    code += className+"::Error::operator "+className+"::Error::Type() const {\n" INDENT "return type;\n}\n\n";
    code += className+"::Error::operator bool() const {\n" INDENT "return type != Error::OK;\n}\n\n";
    code += "const char *"+className+"::Error::typeString() const {\n";
    code += INDENT "switch (type) {\n";
    #define PARSER_GENERATOR_ERROR_TYPE_STRING_CASE(e) code += INDENT INDENT "case Error::" #e ":\n" INDENT INDENT INDENT "return \"" #e "\";\n";
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

    // decodeHexDigit
    code += "\n";
    code += "int "+className+"::decodeHexDigit(char digit) {\n";
    code += INDENT "switch (digit) {\n";
    for (int i = 0; i < 10; ++i)
        code += INDENT INDENT "case '"+std::to_string(i)+"': return 0x0"+std::to_string(i)+";\n";
    for (int i = 0; i < 6; ++i)
        code += std::string(INDENT INDENT "case '")+"ABCDEF"[i]+"': case '"+"abcdef"[i]+"': return 0x0"+"abcdef"[i]+";\n";
    code += INDENT "}\n";
    code += INDENT "return -1;\n";
    code += "}\n";

    // Integer read functions
    if (featureBits&FEATURE_READ_SIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += (settings().noThrow ? className+"::Error::Type " : "void ")+className+"::readSigned(T &value) {\n";
        code += generateReadIntegerBody(true);
        code += "}\n";
    }
    if (featureBits&FEATURE_READ_UNSIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += (settings().noThrow ? className+"::Error::Type " : "void ")+className+"::readUnsigned(T &value) {\n";
        code += generateReadIntegerBody(false);
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
                code += INDENT "Error::Type error = parser."+it->second+"(output);\n";
                code += INDENT "return Error(error, static_cast<int>(parser.cur-jsonString));\n";
            } else {
                code += INDENT "try {\n";
                code += INDENT INDENT "parser."+it->second+"(output);\n";
                code += INDENT "} catch (Error::Type error) {\n";
                code += INDENT INDENT "return Error(error, static_cast<int>(parser.cur-jsonString));\n";
                code += INDENT "}\n";
                code += INDENT "return Error(Error::OK, static_cast<int>(parser.cur-jsonString));\n";
            }
            code += "}\n";
        }
    }

    // Private parse functions
    for (const Function &parseFunction : functions) {
        code += "\n";
        if (settings().noThrow)
            code += className+"::Error::Type ";
        else
            code += "void ";
        code += className+"::"+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+") {\n";
        code += parseFunction.body;
        // Missing newline in function body signifies that it has already returned
        if (code.back() != '\n')
            code += "\n";
        else if (settings().noThrow)
            code += INDENT "return Error::OK;\n";
        code += "}\n";
    }
    code += endNamespace();
    return code;
}
