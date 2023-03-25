
#include "ObjectMapContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/ObjectMapContainerTemplate.h"
#include "../types/OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ObjectMapContainerType::ObjectMapContainerType(const ObjectMapContainerTemplate *containerTemplate, const Type *elementType, const Type *keyType) : ContainerType(containerTemplate, elementType, keyType) { }

const ObjectMapContainerTemplate *ObjectMapContainerType::objectMapContainerTemplate() const {
    return static_cast<const ObjectMapContainerTemplate *>(containerTemplate);
}

const Type *ObjectMapContainerType::keyType() const {
    return std::get<0>(templateArgs);
}

// TODO: remaining methods are exact duplicates of ObjectContainerType

std::string ObjectMapContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    const char *keyName = "key";
    if (keyType() == generator->stringType())
        keyName = ParserGenerator::COMMON_STRING_BUFFER;
    else
        body += indent+keyType()->name().variableDeclaration(keyName)+";\n";
    body += indent+"if (!matchSymbol('{'))\n";
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    body += indent+generateClear("value")+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol('}')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(keyType(), keyName, indent+INDENT);
    body += indent+INDENT "if (!matchSymbol(':'))\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    std::string elemRef = generateRefByKey("value", keyName);
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

std::string ObjectMapContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'{'")+";\n";
    std::string iterBody;
    const OptionalContainerType *optionalElemType = nullptr;
    if (generator->settings().skipEmptyFields)
        optionalElemType = dynamic_cast<const OptionalContainerType *>(elementType());
    if (optionalElemType)
        iterBody += "if ("+optionalElemType->generateHasValue("elem")+") { ";
    iterBody += "if (prev) { ";
    iterBody += generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "','");
    iterBody += "; } prev = true; ";
    iterBody += generator->generateValueSerialization(keyType(), "key");
    iterBody += " ";
    iterBody += generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "':'");
    iterBody += "; ";
    if (optionalElemType) {
        iterBody += generator->generateValueSerialization(optionalElemType->elementType(), optionalElemType->generateGetValue("elem"));
        iterBody += " }";
    } else
        iterBody += generator->generateValueSerialization(elementType(), "elem");
    body += indent+generateIterateElements("value", "i", "end", "key", "elem", iterBody.c_str())+"\n";
    body += indent+generator->stringType()->generateAppendChar(SerializerGenerator::OUTPUT_STRING, "'}'")+";\n";
    return body;
}

std::string ObjectMapContainerType::generateClear(const char *subject) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(objectMapContainerTemplate()->api().clear, r, ARRAY_LENGTH(r));
}

std::string ObjectMapContainerType::generateRefByKey(const char *subject, const char *keyName) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'K', keyName }
    };
    return fillPattern(objectMapContainerTemplate()->api().refByKey, r, ARRAY_LENGTH(r));
}

std::string ObjectMapContainerType::generateIterateElements(const char *subject, const char *iteratorName, const char *endIteratorName, const char *keyName, const char *elementName, const char *body) const {
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
    return fillPattern(objectMapContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r));
}
