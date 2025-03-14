
#include "BasicType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

const char *BasicType::getTypeName(Type type) {
    switch (type) {
        case VOID: return "void";
        case BOOL: return "bool";
        case CHAR: return "char";
        case SIGNED_CHAR: return "signed char";
        case UNSIGNED_CHAR: return "unsigned char";
        case SHORT: return "short";
        case UNSIGNED_SHORT: return "unsigned short";
        case INT: return "int";
        case UNSIGNED_INT: return "unsigned";
        case LONG: return "long";
        case UNSIGNED_LONG: return "unsigned long";
        case LONG_LONG: return "long long";
        case UNSIGNED_LONG_LONG: return "unsigned long long";
        case FLOAT: return "float";
        case DOUBLE: return "double";
        case LONG_DOUBLE: return "long double";
        case SIZE_T: return "size_t";
        case PTRDIFF_T: return "ptrdiff_t";
        case INTPTR_T: return "intptr_t";
        case UINTPTR_T: return "uintptr_t";
        case WCHAR_T: return "wchar_t";
        case CHAR8_T: return "char8_t";
        case CHAR16_T: return "char16_t";
        case CHAR32_T: return "char32_t";
        case INT8_T: return "int8_t";
        case UINT8_T: return "uint8_t";
        case INT16_T: return "int16_t";
        case UINT16_T: return "uint16_t";
        case INT32_T: return "int32_t";
        case UINT32_T: return "uint32_t";
        case INT64_T: return "int64_t";
        case UINT64_T: return "uint64_t";
    }
    return nullptr;
}

BasicType::BasicType(Type type) : DirectType(TypeName(getTypeName(type))), type(type) { }

static std::string generateFloatSerialization(SerializerGenerator *generator, BasicType::Type type, const std::string &indent) {
    const char *macroSuffix = nullptr;
    switch (type) {
        case BasicType::FLOAT:
            macroSuffix = "FLOAT";
            generator->addFeature(SerializerGenerator::FEATURE_SERIALIZE_FLOAT);
            break;
        case BasicType::DOUBLE:
            macroSuffix = "DOUBLE";
            generator->addFeature(SerializerGenerator::FEATURE_SERIALIZE_DOUBLE);
            break;
        case BasicType::LONG_DOUBLE:
            macroSuffix = "LONG_DOUBLE";
            generator->addFeature(SerializerGenerator::FEATURE_SERIALIZE_LONG_DOUBLE);
            break;
        default:
            return std::string();
    }
    std::string body;
    body += indent+"char buffer[64];\n";
    body += indent+"JSON_CPP_SERIALIZE_"+macroSuffix+"(buffer, value);\n";
    body += indent+"switch (buffer[1]) {\n";
    body += indent+"\tcase 'i':\n"; // -inf
    switch (generator->settings().infPolicy) {
        case Settings::InfPolicy::SERIALIZER_ERROR:
            body += indent+"\t\t"+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::InfPolicy::EXPONENT_OVERFLOW:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"-1e999\"")+";\n";
            break;
        case Settings::InfPolicy::ZERO_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'0'")+";\n";
            break;
        case Settings::InfPolicy::NULL_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"null\"")+";\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INF_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-INF\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INF_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-inf\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INF_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-Inf\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INFINITY_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-INFINITY\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INFINITY_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-infinity\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INFINITY_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"-Infinity\\\"\"")+";\n";
            break;
    }
    body += indent+"\t\tbreak;\n";
    body += indent+"\tcase 'n':\n"; // inf or -nan
    body += indent+"\t\tif (buffer[0] == 'i') {\n"; // inf
    switch (generator->settings().infPolicy) {
        case Settings::InfPolicy::SERIALIZER_ERROR:
            body += indent+"\t\t\t"+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::InfPolicy::EXPONENT_OVERFLOW:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"1e999\"")+";\n";
            break;
        case Settings::InfPolicy::ZERO_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'0'")+";\n";
            break;
        case Settings::InfPolicy::NULL_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"null\"")+";\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INF_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"INF\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INF_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"inf\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INF_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"Inf\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INFINITY_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"INFINITY\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INFINITY_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"infinity\\\"\"")+";\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INFINITY_STRING_VALUE:
            body += indent+"\t\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"Infinity\\\"\"")+";\n";
            break;
    }
    body += indent+"\t\t\tbreak;\n";
    body += indent+"\t\t}\n";
    body += indent+"\t\t// fallthrough\n";
    body += indent+"\tcase 'a':\n"; // nan
    switch (generator->settings().nanPolicy) {
        case Settings::NanPolicy::SERIALIZER_ERROR:
            body += indent+"\t\t"+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::NanPolicy::ZERO_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'0'")+";\n";
            break;
        case Settings::NanPolicy::NULL_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"null\"")+";\n";
            break;
        case Settings::NanPolicy::UPPERCASE_NAN_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"NAN\\\"\"")+";\n";
            break;
        case Settings::NanPolicy::LOWERCASE_NAN_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"nan\\\"\"")+";\n";
            break;
        case Settings::NanPolicy::MIXED_CASE_NAN_STRING_VALUE:
            body += indent+"\t\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"\\\"NaN\\\"\"")+";\n";
            break;
    }
    body += indent+"\t\tbreak;\n";
    body += indent+"\tdefault:\n";
    body += indent+"\t\t"+generator->stringType()->generateAppendCStr(SerializerGenerator::OUTPUT_STRING, "buffer")+";\n";
    body += indent+"}\n";
    return body;
}

static std::string generateSignedSerializerFunctionBody(SerializerGenerator *generator, BasicType::Type unsignedType, const std::string &indent) {
    generator->addFeature(SerializerGenerator::FEATURE_WRITE_SIGNED);
    return indent+"writeSigned<"+BasicType::getTypeName(unsignedType)+">(value);\n";
}

std::string BasicType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    switch (type) {
        case VOID:
            break;
        case BOOL:
            return (
                indent+"skipWhitespace();\n"+
                indent+"if ("+ParserGenerator::generateMatchKeyword("false")+")\n"+
                indent+"\tvalue = false;\n"+
                indent+"else if ("+ParserGenerator::generateMatchKeyword("true")+")\n"+
                indent+"\tvalue = true;\n"+
                indent+"else\n"+
                indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n"
            );
        // Signed integer types
        case CHAR:
        case SIGNED_CHAR:
        case SHORT:
        case INT:
        case LONG:
        case LONG_LONG:
        case PTRDIFF_T:
        case INTPTR_T:
        case INT8_T:
        case INT16_T:
        case INT32_T:
        case INT64_T:
        case WCHAR_T: // signedness of wchar_t is not certain but readSigned should cover both possibilities well enough
            generator->addFeature(ParserGenerator::FEATURE_READ_SIGNED);
            return (
                indent+"skipWhitespace();\n"+
                indent+"return readSigned(value);" // no newline = returned
            );
        // Unsigned integer types
        case UNSIGNED_CHAR:
        case UNSIGNED_SHORT:
        case UNSIGNED_INT:
        case UNSIGNED_LONG:
        case UNSIGNED_LONG_LONG:
        case SIZE_T:
        case UINTPTR_T:
        case CHAR8_T:
        case CHAR16_T:
        case CHAR32_T:
        case UINT8_T:
        case UINT16_T:
        case UINT32_T:
        case UINT64_T:
            generator->addFeature(ParserGenerator::FEATURE_READ_UNSIGNED);
            return (
                indent+"skipWhitespace();\n"+
                indent+"return readUnsigned(value);" // no newline = returned
            );
        // Floating point types
        case FLOAT:
            generator->addFeature(Generator::FEATURE_CSTDLIB);
            return (
                indent+"char *end;\n"+
                indent+"value = static_cast<float>(strtod(cur, &end));\n"+
                indent+"if (end == cur)\n"+
                indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n"+
                indent+"cur = end;\n"
            );
        case DOUBLE:
            generator->addFeature(Generator::FEATURE_CSTDLIB);
            return (
                indent+"char *end;\n"+
                indent+"value = strtod(cur, &end);\n"+
                indent+"if (end == cur)\n"+
                indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n"+
                indent+"cur = end;\n"
            );
        case LONG_DOUBLE:
            generator->addFeature(Generator::FEATURE_CSTDLIB);
            return (
                indent+"char *end;\n"+
                indent+"value = strtold(cur, &end);\n"+
                indent+"if (end == cur)\n"+
                indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n"+
                indent+"cur = end;\n"
            );
    }
    return std::string();
}

std::string BasicType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    switch (type) {
        case VOID:
            break;
        case BOOL:
            return (
                indent+"if (value) {\n"+
                indent+"\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"true\"")+";\n"+
                indent+"} else {\n"+
                indent+"\t"+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, "\"false\"")+";\n"+
                indent+"}\n"
            );
        // Signed integer types
        case CHAR:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_CHAR, indent);
        case SIGNED_CHAR:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_CHAR, indent);
        case SHORT:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_SHORT, indent);
        case INT:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_INT, indent);
        case LONG:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_LONG, indent);
        case LONG_LONG:
            return generateSignedSerializerFunctionBody(generator, UNSIGNED_LONG_LONG, indent);
        case PTRDIFF_T:
            return generateSignedSerializerFunctionBody(generator, SIZE_T, indent);
        case INTPTR_T:
            return generateSignedSerializerFunctionBody(generator, INTPTR_T, indent);
        case INT8_T:
            return generateSignedSerializerFunctionBody(generator, UINT8_T, indent);
        case INT16_T:
            return generateSignedSerializerFunctionBody(generator, UINT16_T, indent);
        case INT32_T:
            return generateSignedSerializerFunctionBody(generator, UINT32_T, indent);
        case INT64_T:
            return generateSignedSerializerFunctionBody(generator, UINT64_T, indent);
        // Unsigned integer types
        case UNSIGNED_CHAR:
        case UNSIGNED_SHORT:
        case UNSIGNED_INT:
        case UNSIGNED_LONG:
        case UNSIGNED_LONG_LONG:
        case SIZE_T:
        case UINTPTR_T:
        case CHAR8_T:
        case CHAR16_T:
        case CHAR32_T:
        case UINT8_T:
        case UINT16_T:
        case UINT32_T:
        case UINT64_T:
            generator->addFeature(SerializerGenerator::FEATURE_WRITE_UNSIGNED);
            return indent+"writeUnsigned(value);\n";
        // Floating point types
        case FLOAT: case DOUBLE: case LONG_DOUBLE:
            return generateFloatSerialization(generator, type, indent);
        // Special case
        case WCHAR_T:
            // May or may not be signed depending on implementation and size is not certain -> convert to a signed type that is definitely larger because of possible (x == -x) for minimum value
            generator->addFeature(SerializerGenerator::FEATURE_WRITE_SIGNED);
            return (
                indent+"long long safeValue = static_cast<long long>(value);\n"+
                indent+"writeSigned<unsigned long long>(safeValue);\n"
            );
    }
    return std::string();
}
