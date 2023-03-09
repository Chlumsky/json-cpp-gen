
#include "ArrayContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/ArrayContainerTemplate.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ArrayContainerType::ArrayContainerType(const ArrayContainerTemplate *containerTemplate, const Type *elementType) : ContainerType(containerTemplate, elementType) { }

const ArrayContainerTemplate * ArrayContainerType::arrayContainerTemplate() const {
    return static_cast<const ArrayContainerTemplate *>(containerTemplate);
}

std::string ArrayContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    if (generator->settings().noThrow) {
        body += indent+"if (!matchSymbol('['))\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    } else
        body += indent+"requireSymbol('[');\n";
    body += indent+generateClear("value")+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol(']')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    std::string elemRef = generateRefAppended("value");
    body += generator->generateValueParse(elementType(), elemRef, indent+INDENT);
    if (generator->settings().strictSyntaxCheck)
        body += indent+INDENT "separatorCheck = matchSymbol(',');\n";
    else
        body += indent+INDENT "matchSymbol(',');\n";
    body += indent+"}\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"if (separatorCheck == 1)\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    return body;
}

std::string ArrayContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+"write('[');\n";
    std::string iterBody;
    iterBody += "if (prev) ";
    iterBody += "write(','); ";
    iterBody += "prev = true; ";
    iterBody += generator->generateValueSerialization(elementType(), "elem");
    body += indent+generateIterateElements("value", "i", "end", "elem", iterBody.c_str())+"\n";
    body += indent+"write(']');\n";
    return body;
}

std::string ArrayContainerType::generateClear(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(arrayContainerTemplate()->api().clear, r, ARRAY_LENGTH(r));
}

std::string ArrayContainerType::generateRefAppended(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(arrayContainerTemplate()->api().refAppended, r, ARRAY_LENGTH(r));
}

std::string ArrayContainerType::generateIterateElements(const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'E', elementName },
        { 'F', body }
    };
    return fillPattern(arrayContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r));
}
