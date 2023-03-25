
#include "FixedArrayContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/FixedArrayContainerTemplate.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

FixedArrayContainerType::FixedArrayContainerType(const FixedArrayContainerTemplate *containerTemplate, const ArrayContainerType *arrayContainerType, const Type *elementType) : ContainerType(containerTemplate, elementType), arrayContainerType(arrayContainerType) { }

const FixedArrayContainerTemplate *FixedArrayContainerType::fixedArrayContainerTemplate() const {
    return static_cast<const FixedArrayContainerTemplate *>(containerTemplate);
}

std::string FixedArrayContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+arrayContainerType->name().variableDeclaration("elems")+";\n";
    body += generator->generateValueParse(arrayContainerType, "elems", indent);
    body += indent+generateMoveFromArrayContainer("value", "elems")+";\n";
    return body;
}

std::string FixedArrayContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'['")+";\n";
    std::string iterBody;
    iterBody += "if (prev) { ";
    iterBody += generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "','");
    iterBody += "; } prev = true; ";
    iterBody += generator->generateValueSerialization(elementType(), "elem");
    body += indent+generateIterateElements("value", "i", "end", "elem", iterBody.c_str())+"\n";
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "']'")+";\n";
    return body;
}

std::string FixedArrayContainerType::generateCopyFromArrayContainer(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(fixedArrayContainerTemplate()->api().copyFromArrayContainer, r, ARRAY_LENGTH(r));
}

std::string FixedArrayContainerType::generateMoveFromArrayContainer(const char *subject, const char *x) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(fixedArrayContainerTemplate()->api().moveFromArrayContainer, r, ARRAY_LENGTH(r));
}

std::string FixedArrayContainerType::generateIterateElements(const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'E', elementName },
        { 'F', body }
    };
    return fillPattern(fixedArrayContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r));
}
