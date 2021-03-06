
#include "SerializerGenerator.h"

#define SERIALIZER_GENERATOR_ERROR_STR_INSTANTIATE(e) const char * const SerializerGenerator::Error::e = #e;
FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_ERROR_STR_INSTANTIATE)

const unsigned SerializerGenerator::FEATURE_WRITE_SIGNED = 0x0100;
const unsigned SerializerGenerator::FEATURE_WRITE_UNSIGNED = 0x0200;

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
    return std::string(settings().noThrow ? "return" : "throw")+" Error(ErrorType::"+errorName+", &value)";
}

std::string SerializerGenerator::generateHeader() {
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
    #define SERIALIZER_GENERATOR_APPEND_ERROR_NAME(e) code += std::string(INDENT INDENT)+Error::e+",\n";
    FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_APPEND_ERROR_NAME)
    code += INDENT "};\n\n";

    code += INDENT "struct Error {\n";
    code += INDENT INDENT "ErrorType type;\n";
    code += INDENT INDENT "const void *datapoint;\n\n";
    code += INDENT INDENT "inline Error(ErrorType type = ErrorType::OK) : type(type), datapoint() { }\n";
    code += INDENT INDENT "inline Error(ErrorType type, const void *datapoint) : type(type), datapoint(datapoint) { }\n";
    code += INDENT INDENT "operator ErrorType() const;\n";
    code += INDENT INDENT "operator bool() const;\n";
    code += INDENT INDENT "const char *typeString() const;\n";
    code += INDENT "};\n\n";

    for (const Type *type : entryTypes) {
        std::map<std::string, std::string>::const_iterator it = functionNames.find(type->name().fullName());
        if (it != functionNames.end())
            code += INDENT "static Error serialize("+stringType()->name().refArgDeclaration("jsonString")+", "+type->name().constRefArgDeclaration("input")+");\n";
    }

    code += "\nprotected:\n";
    code += INDENT+stringType()->name().refArgDeclaration("json")+";\n\n";
    code += INDENT+className+"("+stringType()->name().refArgDeclaration("json")+");\n";
    code += INDENT "void write(char c);\n";
    code += INDENT "void write(const char *str);\n";
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
    if (featureBits&FEATURE_CSTDIO)
        code += "#include <cstdio>\n";
    code += "#include \""+relativeHeaderAddress+"\"\n\n";
    code += signature;
    code += beginNamespace();

    // Error member functions
    code += className+"::Error::operator "+className+"::ErrorType() const {\n" INDENT "return type;\n}\n\n";
    code += className+"::Error::operator bool() const {\n" INDENT "return type != ErrorType::OK;\n}\n\n";
    code += "const char *"+className+"::Error::typeString() const {\n";
    code += INDENT "switch (type) {\n";
    #define SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE(e) code += INDENT INDENT "case ErrorType::" #e ":\n" INDENT INDENT INDENT "return \"" #e "\";\n";
    SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE(OK)
    FOR_SERIALIZER_ERRORS(SERIALIZER_GENERATOR_ERROR_TYPE_STRING_CASE)
    code += INDENT "}\n";
    code += INDENT "return \"\";\n";
    code += "}\n\n";

    // Constructor
    code += className+"::"+className+"("+stringType()->name().refArgDeclaration("json")+") : json(json) {\n";
    code += INDENT+stringType()->generateClear("json")+";\n";
    code += "}\n\n";
    // write(char)
    code += "void "+className+"::write(char c) {\n";
    code += INDENT+stringType()->generateAppendChar("json", "c")+";\n";
    code += "}\n\n";
    // write(const char *)
    code += "void "+className+"::write(const char *str) {\n";
    code += INDENT+stringType()->generateAppendCStr("json", "str")+";\n";
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
        char buffer[256];
        if (alias)
            sprintf(buffer, INDENT INDENT "case '\\%c': write(\"\\\\%c\"); break;\n", alias, alias);
        else
            sprintf(buffer, INDENT INDENT "case '\\x%02x': write(\"\\\\u%04x\"); break;\n", i, i);
        code += buffer;
    }
    code += INDENT INDENT "case '\"': write(\"\\\\\\\"\"); break;\n";
    if (settings().escapeForwardSlash)
        code += INDENT INDENT "case '/': write(\"\\\\/\"); break;\n";
    code += INDENT INDENT "case '\\\\': write(\"\\\\\\\\\"); break;\n";
    code += INDENT INDENT "default:\n";
    code += INDENT INDENT INDENT "write(c);\n";
    code += INDENT "}\n";
    code += "}\n";

    // Integer write functions
    if (featureBits&FEATURE_WRITE_SIGNED) {
        code += "\n";
        code += "template <typename U, typename T>\n";
        code += "void "+className+"::writeSigned(T value) {\n";
        code += INDENT "if (value < 0)\n";
        code += INDENT INDENT "write('-'), value = -value;\n";
        code += INDENT "U unsignedValue = static_cast<U>(value);\n";
        code += INDENT "char buffer[4*(sizeof(U)+1)], *cur = &(buffer[4*(sizeof(U)+1)-1] = '\\0');\n";
        code += INDENT "do *--cur = '0'+unsignedValue%10; while (unsignedValue /= 10);\n";
        code += INDENT "write(cur);\n";
        code += "}\n";
    }
    if (featureBits&FEATURE_WRITE_UNSIGNED) {
        code += "\n";
        code += "template <typename T>\n";
        code += "void "+className+"::writeUnsigned(T value) {\n";
        code += INDENT "char buffer[4*(sizeof(T)+1)], *cur = &(buffer[4*(sizeof(T)+1)-1] = '\\0');\n";
        code += INDENT "do *--cur = '0'+value%10; while (value /= 10);\n";
        code += INDENT "write(cur);\n";
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
                code += INDENT "return ErrorType::OK;\n";
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
        if (settings().noThrow)
            code += INDENT "return ErrorType::OK;\n";
        code += "}\n";
    }
    code += endNamespace();
    return code;
}
