
#include "StaticArrayType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

StaticArrayType::StaticArrayType(const Type *elementType, int length) : Type(TypeName(elementType->name().body(), '['+std::to_string(length)+']'+elementType->name().suffix(), elementType->name().substance())), elemType(elementType), length(length) { }

std::string StaticArrayType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('['))\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"int i = 0;\n";
    body += indent+"while (!matchSymbol(']')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"\tif (!separatorCheck)\n";
        body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += indent+"\tif (i == "+std::to_string(length)+")\n";
    body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    body += generator->generateValueParse(elemType, "value[i++]", indent+"\t");
    if (generator->settings().strictSyntaxCheck)
        body += indent+"\tseparatorCheck = matchSymbol(',');\n";
    else
        body += indent+"\tmatchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += indent+"if (i != "+std::to_string(length)+")\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    return body;
}

std::string StaticArrayType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'['")+";\n";
    if (length >= 1) {
        body += generator->generateValueSerialization(elemType, "value[0]", indent);
        if (length >= 2) {
            body += indent+"for (int i = 1; i < "+std::to_string(length)+"; ++i) {\n";
            body += indent+"\t"+generator->stringType()->generateAppendChar(indent+"\t", SerializerGenerator::OUTPUT_STRING, "','")+";\n";
            body += generator->generateValueSerialization(elemType, "value[i]", indent+"\t");
            body += indent+"}\n";
        }
    }
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "']'")+";\n";
    return body;
}
