
#include "BasicType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

static const BasicType UNSIGNED_CHAR_OBJ(BasicType::UNSIGNED_CHAR);
static const BasicType UNSIGNED_SHORT_OBJ(BasicType::UNSIGNED_SHORT);
static const BasicType UNSIGNED_INT_OBJ(BasicType::UNSIGNED_INT);
static const BasicType UNSIGNED_LONG_OBJ(BasicType::UNSIGNED_LONG);
static const BasicType LONG_LONG_OBJ(BasicType::LONG_LONG);
static const BasicType UNSIGNED_LONG_LONG_OBJ(BasicType::UNSIGNED_LONG_LONG);
static const BasicType UINT8_T_OBJ(BasicType::UINT8_T);
static const BasicType UINT16_T_OBJ(BasicType::UINT16_T);
static const BasicType UINT32_T_OBJ(BasicType::UINT32_T);
static const BasicType UINT64_T_OBJ(BasicType::UINT64_T);
static const BasicType PTRDIFF_T_OBJ(BasicType::PTRDIFF_T);
static const BasicType SIZE_T_OBJ(BasicType::SIZE_T);

const char * BasicType::getTypeName(Type type) {
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

std::string BasicType::serializerInputArgDeclaration() const {
    return name().body()+" value";
}

static std::string generateSscanf(ParserGenerator *generator, const char *pattern, const char *outputVar, const std::string &indent) {
    std::string body;
    body += indent+"int consumed = 0;\n";
    body += indent+"if (sscanf(cur, \""+pattern+"%n\", &"+outputVar+", &consumed) != 1)\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += indent+"cur += consumed;\n";
    return body;
}

static std::string generateIndirectSscanf(ParserGenerator *generator, const TypeName &outputType, const char *intermediateType, const char *pattern, const std::string &indent) {
    std::string body;
    body += indent+intermediateType+" intermediate;\n";
    body += generateSscanf(generator, pattern, "intermediate", indent);
    body += indent+"value = static_cast<"+outputType.body()+">(intermediate);\n";
    if (generator->settings().checkIntegerOverflow) {
        body += indent+"if (static_cast<"+intermediateType+">(value) != intermediate)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::VALUE_OUT_OF_RANGE)+";\n";
    }
    return body;
}

static std::string generateFloatSprintf(SerializerGenerator *generator, const char *pattern, const std::string &indent) {
    std::string body;
    body += indent+"char buffer[32];\n";
    body += indent+"sprintf(buffer, \""+pattern+"\", value);\n";
    body += indent+"switch (buffer[1]) {\n";
    body += indent+INDENT "case 'i':\n"; // -inf
    switch (generator->settings().infPolicy) {
        case Settings::InfPolicy::SERIALIZER_ERROR:
            body += indent+INDENT INDENT+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::InfPolicy::EXPONENT_OVERFLOW:
            body += indent+INDENT INDENT "write(\"-1e999\");\n";
            break;
        case Settings::InfPolicy::ZERO_VALUE:
            body += indent+INDENT INDENT "write('0');\n";
            break;
        case Settings::InfPolicy::NULL_VALUE:
            body += indent+INDENT INDENT "write(\"null\");\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INF_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-INF\\\"\");\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INF_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-inf\\\"\");\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INF_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-Inf\\\"\");\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-INFINITY\\\"\");\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-infinity\\\"\");\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"-Infinity\\\"\");\n";
            break;
    }
    body += indent+INDENT INDENT "break;\n";
    body += indent+INDENT "case 'n':\n"; // inf or -nan
    body += indent+INDENT INDENT "if (buffer[0] == 'i') {\n"; // inf
    switch (generator->settings().infPolicy) {
        case Settings::InfPolicy::SERIALIZER_ERROR:
            body += indent+INDENT INDENT INDENT+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::InfPolicy::EXPONENT_OVERFLOW:
            body += indent+INDENT INDENT INDENT "write(\"1e999\");\n";
            break;
        case Settings::InfPolicy::ZERO_VALUE:
            body += indent+INDENT INDENT INDENT "write('0');\n";
            break;
        case Settings::InfPolicy::NULL_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"null\");\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INF_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"INF\\\"\");\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INF_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"inf\\\"\");\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INF_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"Inf\\\"\");\n";
            break;
        case Settings::InfPolicy::UPPERCASE_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"INFINITY\\\"\");\n";
            break;
        case Settings::InfPolicy::LOWERCASE_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"infinity\\\"\");\n";
            break;
        case Settings::InfPolicy::CAPITALIZED_INFINITY_STRING_VALUE:
            body += indent+INDENT INDENT INDENT "write(\"\\\"Infinity\\\"\");\n";
            break;
    }
    body += indent+INDENT INDENT INDENT "break;\n";
    body += indent+INDENT INDENT "}\n";
    body += indent+INDENT "case 'a':\n"; // nan
    switch (generator->settings().nanPolicy) {
        case Settings::NanPolicy::SERIALIZER_ERROR:
            body += indent+INDENT INDENT+generator->generateErrorStatement(SerializerGenerator::Error::UNREPRESENTABLE_FLOAT_VALUE)+";\n";
            break;
        case Settings::NanPolicy::ZERO_VALUE:
            body += indent+INDENT INDENT "write('0');\n";
            break;
        case Settings::NanPolicy::NULL_VALUE:
            body += indent+INDENT INDENT "write(\"null\");\n";
            break;
        case Settings::NanPolicy::UPPERCASE_NAN_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"NAN\\\"\");\n";
            break;
        case Settings::NanPolicy::LOWERCASE_NAN_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"nan\\\"\");\n";
            break;
        case Settings::NanPolicy::MIXED_CASE_NAN_STRING_VALUE:
            body += indent+INDENT INDENT "write(\"\\\"NaN\\\"\");\n";
            break;
    }
    body += indent+INDENT INDENT "break;\n";
    body += indent+INDENT "default:\n";
    body += indent+INDENT INDENT "write(buffer);\n";
    body += indent+"}\n";
    return body;
}

std::string BasicType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    switch (type) {
        case VOID:
            break;
        case BOOL: {
            std::string body;
            body += indent+"skipWhitespace();\n";
            body += indent+"if ("+ParserGenerator::generateMatchKeyword("false")+")\n";
            body += indent+INDENT "value = false;\n";
            body += indent+"else if ("+ParserGenerator::generateMatchKeyword("true")+")\n";
            body += indent+INDENT "value = true;\n";
            body += indent+"else\n";
            body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
            return body;
        }
        case CHAR: case SIGNED_CHAR: case SHORT: case WCHAR_T: case INT8_T: case INT16_T: // via signed int
            return generateIndirectSscanf(generator, name(), "int", "%d", indent);
        case UNSIGNED_CHAR: case UNSIGNED_SHORT: case CHAR8_T: case CHAR16_T: case UINT8_T: case UINT16_T: // via unsigned int
            return generateIndirectSscanf(generator, name(), "unsigned", "%u", indent);
        case INT32_T: // via signed long
            return generateIndirectSscanf(generator, name(), "long", "%ld", indent);
        case CHAR32_T: case UINT32_T: // via unsigned long
            return generateIndirectSscanf(generator, name(), "unsigned long", "%lu", indent);
        case INTPTR_T: case PTRDIFF_T: case INT64_T: // via signed long long
            return generateIndirectSscanf(generator, name(), "long long", "%lld", indent);
        case UINTPTR_T: case SIZE_T: case UINT64_T: // via unsigned long long
            return generateIndirectSscanf(generator, name(), "unsigned long long", "%llu", indent);
        case INT: case UNSIGNED_INT: case LONG: case UNSIGNED_LONG: case LONG_LONG: case UNSIGNED_LONG_LONG: case FLOAT: case DOUBLE: case LONG_DOUBLE: { // exact type
            const char *pattern = nullptr;
            switch (type) {
                case INT:
                    pattern = "%d";
                    break;
                case UNSIGNED_INT:
                    pattern = "%u";
                    break;
                case LONG:
                    pattern = "%ld";
                    break;
                case UNSIGNED_LONG:
                    pattern = "%lu";
                    break;
                case LONG_LONG:
                    pattern = "%lld";
                    break;
                case UNSIGNED_LONG_LONG:
                    pattern = "%llu";
                    break;
                case FLOAT:
                    pattern = "%f";
                    break;
                case DOUBLE:
                    pattern = "%lf";
                    break;
                case LONG_DOUBLE:
                    pattern = "%Lf";
                    break;
                default:
                    return std::string(); // ERROR
            }
            return generateSscanf(generator, pattern, "value", indent);
        }
    }
    return std::string();
}

static std::string delegateSerializerFunctionBody(SerializerGenerator *generator, const BasicType *targetType, const std::string &indent) {
    return indent+"return "+generator->generateSerializerFunctionCall(targetType, "static_cast<"+targetType->name().fullName()+">(value)")+";\n";
}

static std::string generateSignedSerializerFunctionBody(SerializerGenerator *generator, const BasicType *unsignedType, const std::string &indent) {
    return (
        indent+"if (value < 0)\n"+
        indent+INDENT "write('-'), value = -value;\n"+
        delegateSerializerFunctionBody(generator, unsignedType, indent)
    );
}

std::string BasicType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    switch (type) {
        case VOID:
            break;
        case BOOL:
            return indent+"write(value ? \"true\" : \"false\");\n";
        // Signed integer types
        case CHAR:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_CHAR_OBJ, indent);
        case SIGNED_CHAR:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_CHAR_OBJ, indent);
        case SHORT:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_SHORT_OBJ, indent);
        case INT:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_INT_OBJ, indent);
        case LONG:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_LONG_OBJ, indent);
        case LONG_LONG:
            return generateSignedSerializerFunctionBody(generator, &UNSIGNED_LONG_LONG_OBJ, indent);
        case PTRDIFF_T:
            return generateSignedSerializerFunctionBody(generator, &SIZE_T_OBJ, indent);
        case INT8_T:
            return generateSignedSerializerFunctionBody(generator, &UINT8_T_OBJ, indent);
        case INT16_T:
            return generateSignedSerializerFunctionBody(generator, &UINT16_T_OBJ, indent);
        case INT32_T:
            return generateSignedSerializerFunctionBody(generator, &UINT32_T_OBJ, indent);
        case INT64_T:
            return generateSignedSerializerFunctionBody(generator, &UINT64_T_OBJ, indent);
        // Unsigned integer types
        case UNSIGNED_CHAR:
        case UNSIGNED_SHORT:
        case UNSIGNED_INT:
        case UNSIGNED_LONG:
        case UNSIGNED_LONG_LONG:
        case SIZE_T:
        case CHAR8_T:
        case CHAR16_T:
        case CHAR32_T:
        case UINT8_T:
        case UINT16_T:
        case UINT32_T:
        case UINT64_T:
            return (
                indent+"char buffer[48], *cur = &(buffer[47] = '\\0');\n"+
                indent+"do *--cur = '0'+value%10; while (value /= 10);\n"+
                indent+"write(cur);\n"
            );
        // Floating point types
        case FLOAT: case DOUBLE: case LONG_DOUBLE: { // exact floating-point type
            const char *pattern = nullptr;
            switch (type) {
                case FLOAT:
                    pattern = "%.9g";
                    break;
                case DOUBLE:
                    pattern = "%.17lg";
                    break;
                case LONG_DOUBLE:
                    pattern = "%.33Lg";
                    break;
                default:
                    return std::string(); // unreachable
            }
            return generateFloatSprintf(generator, pattern, indent);
        }
        // Special cases
        case WCHAR_T:
            // May or may not be signed depending on implementation and size is not certain -> convert to a signed type that is definitely larger because of possible (x == -x) for minimum value
            return delegateSerializerFunctionBody(generator, &LONG_LONG_OBJ, indent);
        case INTPTR_T:
            return delegateSerializerFunctionBody(generator, &PTRDIFF_T_OBJ, indent);
        case UINTPTR_T:
            return delegateSerializerFunctionBody(generator, &SIZE_T_OBJ, indent);
    }
    return std::string();
}
