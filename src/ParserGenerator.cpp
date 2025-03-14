
#include "ParserGenerator.h"

#define PARSER_GENERATOR_ERROR_STR_INSTANTIATE(e) const char *const ParserGenerator::Error::e = #e;
FOR_PARSER_ERRORS(PARSER_GENERATOR_ERROR_STR_INSTANTIATE)

const unsigned ParserGenerator::FEATURE_READ_SIGNED = 0x0100;
const unsigned ParserGenerator::FEATURE_READ_UNSIGNED = 0x0200;

static constexpr const char *const COMMON_FUNCTION_IMPL_NO_THROW =
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
)";

static constexpr const char *const COMMON_FUNCTION_IMPL_THROW =
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
        parseFunction.body = type->generateParserFunctionBody(this, "\t");
        functions.push_back(std::move(parseFunction));
    }
    return functionName+"("+outputArg+")";
}

std::string ParserGenerator::generateValueParse(const Type *type, const std::string &outputArg, const std::string &indent) {
    if (settings().noThrow) {
        return indent+"if (Error::Type error = "+generateParserFunctionCall(type, outputArg)+")\n"+
            indent+"\treturn error;\n";
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
            body += indent+"switch ("+valueType->generateGetLength(indent+"\t", value)+") {\n";
            for (const std::map<int, std::unique_ptr<StringSwitchTree> >::value_type &branch : switchTree->branches) {
                body += indent+"\tcase "+std::to_string(branch.first)+":\n";
                body += generateSwitchTree(caseGenerator, branch.second.get(), valueType, value, indent+"\t\t", branch.first > knownMinLength ? branch.first : knownMinLength);
                body += indent+"\t\tbreak;\n";
            }
            body += indent+"}\n";
        } else if (switchTree->position >= 0) {
            std::string switchIndent = indent;
            bool lengthCondition = switchTree->position >= knownMinLength;
            if (lengthCondition) {
                body += indent+"if ("+valueType->generateGetLength(indent+"\t", value)+" > "+std::to_string(switchTree->position)+") {\n";
                knownMinLength = switchTree->position+1;
                switchIndent += "\t";
            }
            body += switchIndent+"switch ("+valueType->generateGetCharAt(switchIndent+"\t", value, std::to_string(switchTree->position).c_str())+") {\n";
            for (const std::map<int, std::unique_ptr<StringSwitchTree> >::value_type &branch : switchTree->branches) {
                body += switchIndent+"\tcase "+Generator::charLiteral(char(branch.first))+":\n";
                body += generateSwitchTree(caseGenerator, branch.second.get(), valueType, value, switchIndent+"\t\t", knownMinLength);
                body += switchIndent+"\t\tbreak;\n";
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
        body += "\tbool negative = *cur == '-' && (++cur, true);\n";
    body += "\tif (*cur >= '0' && *cur <= '9')\n";
    body += "\t\tvalue = *cur++-'0';\n";
    body += "\telse\n";
    body += "\t\t"+generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += "\twhile (*cur >= '0' && *cur <= '9')";
    if (settings().checkIntegerOverflow) {
        body += " {\n";
        body += "\t\tif (";
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
        body += "\t\t\t"+generateErrorStatement(ParserGenerator::Error::VALUE_OUT_OF_RANGE)+";\n";
        body += "\t\tvalue = static_cast<T>(10*value+(*cur++-'0'));\n";
        body += "\t}\n";
    } else
        body += "\n" "\t\tvalue = static_cast<T>(10*value+(*cur++-'0'));\n";
    if (signedInt) {
        body += "\tif (negative)\n";
        body += "\t\tvalue = -value;\n";
    }
    if (settings().noThrow)
        body += "\treturn Error::OK;\n";
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
    code += "\tstruct Error {\n";
    code += "\t\tenum Type {\n";
    code += "\t\t\tOK";
    #define PARSER_GENERATOR_APPEND_ERROR_NAME(e) code += std::string(",\n\t\t\t")+Error::e;
    FOR_PARSER_ERRORS(PARSER_GENERATOR_APPEND_ERROR_NAME)
    code += "\n" "\t\t} type;\n";
    code += "\t\tint position;\n\n";
    code += "\t\tinline Error(Type type = Error::OK, int position = -1) : type(type), position(position) { }\n";
    code += "\t\toperator Type() const;\n";
    code += "\t\toperator bool() const;\n";
    code += "\t\tconst char *typeString() const;\n";
    code += "\t};\n\n";

    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end())
            code += "\tstatic Error parse("+type->name().refArgDeclaration("output")+", const char *jsonString);\n";
    }

    code += "\nprotected:\n";
    code += generateVirtualTypedefs("\t");
    code += "\tconst char *cur;\n";
    code += "\t"+stringType()->name().variableDeclaration(COMMON_STRING_BUFFER)+";\n\n";
    code += "\texplicit "+className+"(const char *str);\n";
    code += "\tvoid skipWhitespace();\n";
    code += std::string("\t")+(settings().noThrow ? "Error::Type" : "void")+" skipValue();\n";
    code += "\tbool matchSymbol(char s);\n";
    code += std::string("\t")+(settings().noThrow ? "bool" : "void")+" readHexQuad(int &value);\n";
    code += "\tstatic bool isAlphanumeric(char c);\n";
    code += "\tstatic int decodeHexDigit(char digit);\n";
    code += "\n";

    for (const Function &parseFunction : functions)
        code += std::string("\t")+(settings().noThrow ? "Error::Type " : "void ")+parseFunction.name+"("+parseFunction.type->name().refArgDeclaration("value")+");\n";

    if (featureBits&(FEATURE_READ_SIGNED|FEATURE_READ_UNSIGNED)) {
        code += "\nprivate:\n";
        if (featureBits&FEATURE_READ_SIGNED) {
            code += "\ttemplate <typename T>\n";
            code += std::string("\t")+(settings().noThrow ? "Error::Type" : "void")+" readSigned(T &value);\n";
        }
        if (featureBits&FEATURE_READ_UNSIGNED) {
            code += "\ttemplate <typename T>\n";
            code += std::string("\t")+(settings().noThrow ? "Error::Type" : "void")+" readUnsigned(T &value);\n";
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
    code += className+"::Error::operator "+className+"::Error::Type() const {\n\treturn type;\n}\n\n";
    code += className+"::Error::operator bool() const {\n\treturn type != Error::OK;\n}\n\n";
    code += "const char *"+className+"::Error::typeString() const {\n";
    code += "\tswitch (type) {\n";
    #define PARSER_GENERATOR_ERROR_TYPE_STRING_CASE(e) code += "\t\tcase Error::" #e ":\n\t\t\treturn \"" #e "\";\n";
    PARSER_GENERATOR_ERROR_TYPE_STRING_CASE(OK)
    FOR_PARSER_ERRORS(PARSER_GENERATOR_ERROR_TYPE_STRING_CASE)
    code += "\t}\n";
    code += "\treturn \"\";\n";
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
    code += "\tswitch (c) {";
    for (int i = 0; i < 26+26+10; ++i) {
        if (!(i%26%9) && i != 26+26+9)
            code += "\n\t\t";
        else
            code.push_back(' ');
        code += std::string("case '")+"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[i]+"':";
    }
    code += "\n\t\t\treturn true;\n";
    code += "\t\tdefault:\n";
    code += "\t\t\treturn false;\n";
    code += "\t}\n";
    code += "}\n";

    // decodeHexDigit
    code += "\n";
    code += "int "+className+"::decodeHexDigit(char digit) {\n";
    code += "\tswitch (digit) {\n";
    for (int i = 0; i < 10; ++i)
        code += "\t\tcase '"+std::to_string(i)+"': return 0x0"+std::to_string(i)+";\n";
    for (int i = 0; i < 6; ++i)
        code += std::string("\t\tcase '")+"ABCDEF"[i]+"': case '"+"abcdef"[i]+"': return 0x0"+"abcdef"[i]+";\n";
    code += "\t}\n";
    code += "\treturn -1;\n";
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
            code += "\t"+className+" parser(jsonString);\n";
            if (settings().noThrow) {
                code += "\tError::Type error = parser."+it->second+"(output);\n";
                code += "\treturn Error(error, static_cast<int>(parser.cur-jsonString));\n";
            } else {
                code += "\ttry {\n";
                code += "\t\tparser."+it->second+"(output);\n";
                code += "\t} catch (Error::Type error) {\n";
                code += "\t\treturn Error(error, static_cast<int>(parser.cur-jsonString));\n";
                code += "\t}\n";
                code += "\treturn Error(Error::OK, static_cast<int>(parser.cur-jsonString));\n";
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
            code += "\treturn Error::OK;\n";
        code += "}\n";
    }
    code += endNamespace();
    return code;
}
