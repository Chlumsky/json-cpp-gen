
#include "ArrayContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/ArrayContainerTemplate.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ArrayContainerType::ArrayContainerType(const ArrayContainerTemplate *containerTemplate, const Type *elementType) : ContainerType(containerTemplate, elementType) { }

const ArrayContainerTemplate *ArrayContainerType::arrayContainerTemplate() const {
    return static_cast<const ArrayContainerTemplate *>(containerTemplate);
}

std::string ArrayContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if (!matchSymbol('['))\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += indent+generateClear(indent, "value")+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol(']')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"\tif (!separatorCheck)\n";
        body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    std::string elemRef = generateRefAppended(indent+"\t\t", "value");
    body += generator->generateValueParse(elementType(), elemRef, indent+"\t");
    if (generator->settings().strictSyntaxCheck)
        body += indent+"\tseparatorCheck = matchSymbol(',');\n";
    else
        body += indent+"\tmatchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    return body;
}

std::string ArrayContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'['")+";\n";
    std::string iterBody;
    iterBody += "if (prev) {\n";
    iterBody += "\t"+generator->stringType()->generateAppendChar("\t", SerializerGenerator::OUTPUT_STRING, "','")+";\n";
    iterBody += "}\n";
    iterBody += "prev = true;\n";
    iterBody += generator->generateValueSerialization(elementType(), "elem", "");
    iterBody.pop_back(); // trim \n at end
    body += indent+generateIterateElements(indent, "value", "i", "end", "elem", iterBody.c_str())+"\n";
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "']'")+";\n";
    return body;
}

std::string ArrayContainerType::generateClear(const std::string &indent, const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(arrayContainerTemplate()->api().clear, r, ARRAY_LENGTH(r), indent);
}

std::string ArrayContainerType::generateRefAppended(const std::string &indent, const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(arrayContainerTemplate()->api().refAppended, r, ARRAY_LENGTH(r), indent);
}

std::string ArrayContainerType::generateIterateElements(const std::string &indent, const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'E', elementName },
        { 'F', body }
    };
    return fillPattern(arrayContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r), indent);
}
