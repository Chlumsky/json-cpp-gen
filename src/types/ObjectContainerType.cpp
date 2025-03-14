
#include "ObjectContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/ObjectContainerTemplate.h"
#include "../types/OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ObjectContainerType::ObjectContainerType(const ObjectContainerTemplate *containerTemplate, const Type *elementType) : ContainerType(containerTemplate, elementType) { }

const ObjectContainerTemplate *ObjectContainerType::objectContainerTemplate() const {
    return static_cast<const ObjectContainerTemplate *>(containerTemplate);
}

const Type *ObjectContainerType::keyType() const {
    return objectContainerTemplate()->keyType();
}

std::string ObjectContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    const char *keyName = "key";
    if (keyType() == generator->stringType())
        keyName = ParserGenerator::COMMON_STRING_BUFFER;
    else
        body += indent+keyType()->name().variableDeclaration(keyName)+";\n";
    body += indent+"if (!matchSymbol('{'))\n";
    body += indent+"\t"+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += indent+generateClear(indent, "value")+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol('}')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+"\tif (!separatorCheck)\n";
        body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(keyType(), keyName, indent+"\t");
    body += indent+"\tif (!matchSymbol(':'))\n";
    body += indent+"\t\t"+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    std::string elemRef = generateRefByKey(indent+"\t\t", "value", keyName);
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

std::string ObjectContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'{'")+";\n";
    std::string iterBody;
    const OptionalContainerType *optionalElemType = nullptr;
    if (generator->settings().skipEmptyFields)
        optionalElemType = elementType()->optionalContainerType();
    std::string iterIndent;
    if (optionalElemType) {
        iterBody += "if ("+optionalElemType->generateHasValue("\t", "elem")+") {\n";
        iterIndent += "\t";
    }
    iterBody += iterIndent+"if (prev) {\n";
    iterBody += iterIndent+"\t"+generator->stringType()->generateAppendChar(iterIndent+"\t", SerializerGenerator::OUTPUT_STRING, "','")+";\n";
    iterBody += iterIndent+"}\n";
    iterBody += iterIndent+"prev = true;\n";
    iterBody += generator->generateValueSerialization(keyType(), "key", iterIndent);
    iterBody += iterIndent+generator->stringType()->generateAppendChar(iterIndent, SerializerGenerator::OUTPUT_STRING, "':'")+";\n";
    if (optionalElemType) {
        iterBody += generator->generateValueSerialization(optionalElemType->elementType(), optionalElemType->generateGetValue("\t\t", "elem"), "\t");
        iterBody += "}\n";
    } else
        iterBody += generator->generateValueSerialization(elementType(), "elem", "");
    iterBody.pop_back(); // trim \n at end
    body += indent+generateIterateElements(indent, "value", "i", "end", "key", "elem", iterBody.c_str())+"\n";
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'}'")+";\n";
    return body;
}

std::string ObjectContainerType::generateClear(const std::string &indent, const char *subject) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(objectContainerTemplate()->api().clear, r, ARRAY_LENGTH(r), indent);
}

std::string ObjectContainerType::generateRefByKey(const std::string &indent, const char *subject, const char *keyName) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'K', keyName }
    };
    return fillPattern(objectContainerTemplate()->api().refByKey, r, ARRAY_LENGTH(r), indent);
}

std::string ObjectContainerType::generateIterateElements(const std::string &indent, const char *subject, const char *iteratorName, const char *endIteratorName, const char *keyName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'K', keyName },
        { 'V', elementName },
        { 'F', body }
    };
    return fillPattern(objectContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r), indent);
}
