
#include "StaticArrayContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/StaticArrayContainerTemplate.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

StaticArrayContainerType::StaticArrayContainerType(const StaticArrayContainerTemplate *containerTemplate, const Type *elementType, int length) : ContainerType(containerTemplate, elementType, length) { }

const StaticArrayContainerTemplate * StaticArrayContainerType::staticArrayContainerTemplate() const {
    return static_cast<const StaticArrayContainerTemplate *>(containerTemplate);
}

int StaticArrayContainerType::length() const {
    return std::get<0>(templateArgs);
}

std::string StaticArrayContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
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
    body += indent+INDENT "if (i == "+std::to_string(length())+")\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    body += generator->generateValueParse(elementType(), generateRefByIndex("value", "i"), indent+INDENT);
    body += indent+INDENT "++i;\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+INDENT "separatorCheck = matchSymbol(',');\n";
    else
        body += indent+INDENT "matchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += indent+"if (i != "+std::to_string(length())+")\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::ARRAY_SIZE_MISMATCH)+";\n";
    return body;
}

std::string StaticArrayContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"write('[');\n";
    if (length() >= 1) {
        body += generator->generateValueSerialization(elementType(), generateRefByIndex("value", "0"), indent);
        if (length() >= 2) {
            body += indent+"for (int i = 1; i < "+std::to_string(length())+"; ++i) {\n";
            body += indent+INDENT "write(',');\n";
            body += generator->generateValueSerialization(elementType(), generateRefByIndex("value", "i"), indent+INDENT);
            body += indent+"}\n";
        }
    }
    body += indent+"write(']');\n";
    return body;
}

std::string StaticArrayContainerType::generateRefByIndex(const char *subject, const char *index) const {
    std::string lengthStr = std::to_string(length());
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'N', lengthStr.c_str() },
        { 'S', subject },
        { 'I', index }
    };
    return fillPattern(staticArrayContainerTemplate()->api().refByIndex, r, ARRAY_LENGTH(r));
}
