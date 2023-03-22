
#include "StaticArrayType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

StaticArrayType::StaticArrayType(const Type *elementType, int length) : Type(TypeName(elementType->name().body(), '['+std::to_string(length)+']'+elementType->name().suffix())), elemType(elementType), length(length) { }

std::string StaticArrayType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('['))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"int i = 0;\n";
    body += indent+"while (!matchSymbol(']')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += indent+INDENT "if (i == "+std::to_string(length)+")\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    body += generator->generateValueParse(elemType, "value[i++]", indent+INDENT);
    if (generator->settings().strictSyntaxCheck)
        body += indent+INDENT "separatorCheck = matchSymbol(',');\n";
    else
        body += indent+INDENT "matchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += indent+"if (i != "+std::to_string(length)+")\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    return body;
}

std::string StaticArrayType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"write('[');\n";
    if (length >= 1) {
        body += generator->generateValueSerialization(elemType, "value[0]", indent);
        if (length >= 2) {
            body += indent+"for (int i = 1; i < "+std::to_string(length)+"; ++i) {\n";
            body += indent+INDENT "write(',');\n";
            body += generator->generateValueSerialization(elemType, "value[i]", indent+INDENT);
            body += indent+"}\n";
        }
    }
    body += indent+"write(']');\n";
    return body;
}
