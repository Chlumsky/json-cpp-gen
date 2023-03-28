
#include "SerializerGenerator.h"

#define SERIALIZER_GENERATOR_ERROR_STR_INSTANTIATE(e) const char *const SerializerGenerator::Error::e = #e;
FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_ERROR_STR_INSTANTIATE)

const unsigned SerializerGenerator::FEATURE_WRITE_SIGNED = 0x0100;
const unsigned SerializerGenerator::FEATURE_WRITE_UNSIGNED = 0x0200;
const unsigned SerializerGenerator::FEATURE_SERIALIZE_FLOAT = 0x1000;
const unsigned SerializerGenerator::FEATURE_SERIALIZE_DOUBLE = 0x2000;
const unsigned SerializerGenerator::FEATURE_SERIALIZE_LONG_DOUBLE = 0x4000;

static std::string hexUint8(unsigned char x) {
    char buffer[] = "00";
    char *c = buffer+sizeof(buffer)-1;
    for (int i = 0; i < 2; ++i, x >>= 4)
        *--c = "0123456789abcdef"[x&0x0f];
    return std::string(buffer);
}

SerializerGenerator::SerializerGenerator(const std::string &className, const StringType *stringType, const Settings &settings) : Generator(className, stringType, settings) { }

void SerializerGenerator::generateSerializerFunction(const Type *type) {
    if (type) {
        generateSerializerFunctionCall(type, "");
        entryTypes.push_back(type);
    }
}

std::string SerializerGenerator::generateSerializerFunctionCall(const Type *type, const std::string &inputArg) {
    if (!type)
        return std::string();
    std::string &functionName = functionNames[type->name().fullName()];
    if (functionName.empty()) {
        functionName = generateFunctionName("serialize", type);
        Function serializeFunction;
        serializeFunction.type = type;
        serializeFunction.name = functionName;
        serializeFunction.body = type->generateSerializerFunctionBody(this, INDENT);
        functions.push_back(std::move(serializeFunction));
    }
    return functionName+"("+inputArg+")";
}

std::string SerializerGenerator::generateValueSerialization(const Type *type, const std::string &inputArg, const std::string &indent) {
    if (settings().noThrow)
        return indent+"if (Error error = "+generateSerializerFunctionCall(type, inputArg)+(indent.empty() ? ") " : ")\n"+indent+INDENT)+"return error;"+(indent.empty() ? "" : "\n");
    else
        return indent+generateSerializerFunctionCall(type, inputArg)+(indent.empty() ? ";" : ";\n");
}

std::string SerializerGenerator::generateErrorStatement(const char *errorName) const {
    return std::string(settings().noThrow ? "return" : "throw")+" Error(Error::"+errorName+", &value)";
}

std::string SerializerGenerator::generateHeader() {
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
    #define SERIALIZER_GENERATOR_APPEND_ERROR_NAME(e) code += std::string(",\n" INDENT INDENT INDENT)+Error::e;
    FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_APPEND_ERROR_NAME)
    code += "\n" INDENT INDENT "} type;\n";
    code += INDENT INDENT "const void *datapoint;\n\n";
    code += INDENT INDENT "inline Error(Type type = Error::OK) : type(type), datapoint() { }\n";
    code += INDENT INDENT "inline Error(Type type, const void *datapoint) : type(type), datapoint(datapoint) { }\n";
    code += INDENT INDENT "operator Type() const;\n";
    code += INDENT INDENT "operator bool() const;\n";
    code += INDENT INDENT "const char *typeString() const;\n";
    code += INDENT "};\n\n";

    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end())
            code += INDENT "static Error serialize("+stringType()->name().refArgDeclaration("jsonString")+", "+type->name().constRefArgDeclaration("input")+");\n";
    }

    code += "\nprotected:\n";
    code += generateVirtualTypedefs(INDENT);
    code += INDENT+stringType()->name().refArgDeclaration(OUTPUT_STRING)+";\n\n";
    code += INDENT+className+"("+stringType()->name().refArgDeclaration(OUTPUT_STRING)+");\n";
    code += INDENT "void writeEscaped(char c);\n";
    code += "\n";

    for (const Function &serializeFunction : functions)
        code += std::string(INDENT)+(settings().noThrow ? "Error " : "void ")+serializeFunction.name+"("+serializeFunction.type->name().constRefArgDeclaration("value")+");\n";

    if (featureBits&(FEATURE_WRITE_SIGNED|FEATURE_WRITE_UNSIGNED)) {
        code += "\nprivate:\n";
        if (featureBits&FEATURE_WRITE_SIGNED) {
            code += INDENT "template <typename U, typename T>\n";
            code += INDENT "void writeSigned(T value);\n";
        }
        if (featureBits&FEATURE_WRITE_UNSIGNED) {
            code += INDENT "template <typename T>\n";
            code += INDENT "void writeUnsigned(T value);\n";
        }
    }
    code += "\n};\n";
    code += endNamespace();
    return code;
}

std::string SerializerGenerator::generateSource() {
    return generateSource(className+".h");
}

std::string SerializerGenerator::generateSource(const std::string &relativeHeaderAddress) {
    std::string code;
    code += "\n";
    code += signature;
    code += "#include \""+relativeHeaderAddress+"\"\n\n";

    if (featureBits&FEATURE_SERIALIZE_FLOAT) {
        code += "#ifndef JSON_CPP_SERIALIZE_FLOAT\n";
        code += "#include <cstdio>\n";
        code += "#define JSON_CPP_SERIALIZE_FLOAT(outBuffer, x) sprintf(outBuffer, \"%.9g\", x)\n";
        code += "#endif\n\n";
    }
    if (featureBits&FEATURE_SERIALIZE_DOUBLE) {
        code += "#ifndef JSON_CPP_SERIALIZE_DOUBLE\n";
        code += "#include <cstdio>\n";
        code += "#define JSON_CPP_SERIALIZE_DOUBLE(outBuffer, x) sprintf(outBuffer, \"%.17g\", x)\n";
        code += "#endif\n\n";
    }
    if (featureBits&FEATURE_SERIALIZE_LONG_DOUBLE) {
        code += "#ifndef JSON_CPP_SERIALIZE_LONG_DOUBLE\n";
        code += "#include <cstdio>\n";
        code += "#define JSON_CPP_SERIALIZE_LONG_DOUBLE(outBuffer, x) sprintf(outBuffer, \"%.33Lg\", x)\n";
        code += "#endif\n\n";
    }

    code += beginNamespace();

    // Error member functions
    code += className+"::Error::operator "+className+"::Error::Type() const {\n" INDENT "return type;\n}\n\n";
    code += className+"::Error::operator bool() const {\n" INDENT "return type != Error::OK;\n}\n\n";
    code += "const char *"+className+"::Error::typeString() const {\n";
    code += INDENT "switch (type) {\n";
    #define SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE(e) code += INDENT INDENT "case Error::" #e ":\n" INDENT INDENT INDENT "return \"" #e "\";\n";
    SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE(OK)
    FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE)
    code += INDENT "}\n";
    code += INDENT "return \"\";\n";
    code += "}\n\n";

    // Constructor
    code += className+"::"+className+"("+stringType()->name().refArgDeclaration(OUTPUT_STRING)+") : "+OUTPUT_STRING+"("+OUTPUT_STRING+") {\n";
    code += INDENT+stringType()->generateClear(OUTPUT_STRING)+";\n";
    code += "}\n\n";
    // writeEscaped
    code += "void "+className+"::writeEscaped(char c) {\n";
    code += INDENT "switch (c) {\n";
    for (int i = 0x00; i < 0x20; ++i) {
        char alias = '\0';
        switch ((char) i) {
            case '\b': alias = 'b'; break;
            case '\f': alias = 'f'; break;
            case '\n': alias = 'n'; break;
            case '\r': alias = 'r'; break;
            case '\t': alias = 't'; break;
        }
        if (alias)
            code += std::string(INDENT INDENT "case '\\")+alias+"': "+stringType()->generateAppendStringLiteral(OUTPUT_STRING, (std::string("\"\\\\")+alias+"\"").c_str())+"; break;\n";
        else {
            std::string hexChar = hexUint8((unsigned char) i);
            code += INDENT INDENT "case '\\x"+hexChar+"': "+stringType()->generateAppendStringLiteral(OUTPUT_STRING, ("\"\\\\u00"+hexChar+"\"").c_str())+"; break;\n";
        }
    }
    code += INDENT INDENT "case '\"': "+stringType()->generateAppendStringLiteral(OUTPUT_STRING, "\"\\\\\\\"\"")+"; break;\n";
    if (settings().escapeForwardSlash)
        code += INDENT INDENT "case '/': "+stringType()->generateAppendStringLiteral(OUTPUT_STRING, "\"\\\\/\"")+"; break;\n";
    code += INDENT INDENT "case '\\\\': "+stringType()->generateAppendStringLiteral(OUTPUT_STRING, "\"\\\\\\\\\"")+"; break;\n";
    code += INDENT INDENT "default:\n";
    code += INDENT INDENT INDENT+stringType()->generateAppendChar(OUTPUT_STRING, "c")+";\n";
    code += INDENT "}\n";
    code += "}\n";

    // Integer write functions
    // - this implementation requires that '0'+1 == '1', '0'+2 == '2', ... , '0'+9 == '9'
    if (featureBits&FEATURE_WRITE_SIGNED) {
        code += "\n";
        code += "template <typename U, typename T>\n";
        code += "void "+className+"::writeSigned(T value) {\n";
        code += INDENT "if (value < 0) {\n";
        code += INDENT INDENT+stringType()->generateAppendChar(OUTPUT_STRING, "'-'")+";\n";
        code += INDENT INDENT "value = -value;\n";
        code += INDENT "}\n";
        code += INDENT "U unsignedValue = static_cast<U>(value);\n";
        code += INDENT "char buffer[4*(sizeof(U)+1)], *cur = &(buffer[4*(sizeof(U)+1)-1] = '\\0');\n";
        code += INDENT "do *--cur = '0'+unsignedValue%10; while (unsignedValue /= 10);\n";
        code += INDENT+stringType()->generateAppendCStr(OUTPUT_STRING, "cur")+";\n";
        code += "}\n";
    }
    if (featureBits&FEATURE_WRITE_UNSIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += "void "+className+"::writeUnsigned(T value) {\n";
        code += INDENT "char buffer[4*(sizeof(T)+1)], *cur = &(buffer[4*(sizeof(T)+1)-1] = '\\0');\n";
        code += INDENT "do *--cur = '0'+value%10; while (value /= 10);\n";
        code += INDENT+stringType()->generateAppendCStr(OUTPUT_STRING, "cur")+";\n";
        code += "}\n";
    }

    // Public serialize functions
    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end()) {
            code += "\n";
            code += className+"::Error "+className+"::serialize("+stringType()->name().refArgDeclaration("jsonString")+", "+type->name().constRefArgDeclaration("input")+") {\n";
            if (settings().noThrow)
                code += INDENT "return "+className+"(jsonString)."+it->second+"(input);\n";
            else {
                code += INDENT "try {\n";
                code += INDENT INDENT+className+"(jsonString)."+it->second+"(input);\n";
                code += INDENT "} catch (const Error &error) {\n";
                code += INDENT INDENT "return error;\n";
                code += INDENT "}\n";
                code += INDENT "return Error::OK;\n";
            }
            code += "}\n";
        }
    }

    // Private serialize functions
    for (const Function &serializeFunction : functions) {
        code += "\n";
        if (settings().noThrow)
            code += className+"::Error ";
        else
            code += "void ";
        code += className+"::"+serializeFunction.name+"("+serializeFunction.type->name().constRefArgDeclaration("value")+") {\n";
        code += serializeFunction.body;
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
