
#include "ObjectContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/ObjectContainerTemplate.h"
#include "../types/OptionalContainerType.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ObjectContainerType::ObjectContainerType(const ObjectContainerTemplate *containerTemplate, const StringType *keyType, const Type *elementType) : ContainerType(containerTemplate, elementType) { }

const ObjectContainerTemplate * ObjectContainerType::objectContainerTemplate() const {
    return static_cast<const ObjectContainerTemplate *>(containerTemplate);
}

const StringType * ObjectContainerType::keyType() const {
    return objectContainerTemplate()->keyType();
}

const Type * ObjectContainerType::elementType() const {
    return std::get<0>(templateArgs);
}

std::string ObjectContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    const Type *keyType = generator->stringType(); // should be keyType() ?
    body += indent+keyType->name().variableDeclaration("key")+";\n";
    if (generator->settings().noThrow) {
        body += indent+"if (!matchSymbol('{'))\n";
        body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::TYPE_MISMATCH)+";\n";
    } else
        body += indent+"requireSymbol('{');\n";
    body += indent+generateClear("value")+";\n";
    if (generator->settings().strictSyntaxCheck)
        body += indent+"int separatorCheck = -1;\n";
    body += indent+"while (!matchSymbol('}')) {\n";
    if (generator->settings().strictSyntaxCheck) {
        body += indent+INDENT "if (!separatorCheck)\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    }
    body += generator->generateValueParse(keyType, "key", indent+INDENT);
    if (generator->settings().noThrow) {
        body += indent+INDENT "if (!requireSymbol(':'))\n";
        body += indent+INDENT INDENT+generator->generateErrorStatement(ParserGenerator::Error::JSON_SYNTAX_ERROR)+";\n";
    } else
        body += indent+INDENT "requireSymbol(':');\n";
    std::string elemRef = generateRefByKey("value", "key");
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

std::string ObjectContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"bool prev = false;\n";
    body += indent+"write('{');\n";
    std::string iterBody;
    const OptionalContainerType *optionalElemType = nullptr;
    if (generator->settings().skipEmptyFields)
        optionalElemType = dynamic_cast<const OptionalContainerType *>(elementType());
    if (optionalElemType)
        iterBody += "if ("+optionalElemType->generateHasValue("elem")+") { ";
    iterBody += "if (prev) ";
    iterBody += "write(','); ";
    iterBody += "prev = true; ";
    iterBody += generator->generateValueSerialization(keyType(), "key");
    iterBody += " write(':'); ";
    if (optionalElemType) {
        iterBody += generator->generateValueSerialization(optionalElemType->elementType(), optionalElemType->generateGetValue("elem"));
        iterBody += " }";
    } else
        iterBody += generator->generateValueSerialization(elementType(), "elem");
    body += indent+generateIterateElements("value", "i", "key", "elem", iterBody.c_str())+"\n";
    body += indent+"write('}');\n";
    return body;
}

std::string ObjectContainerType::generateClear(const char *subject) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(objectContainerTemplate()->api().clear, r, ARRAY_LENGTH(r));
}

std::string ObjectContainerType::generateRefByKey(const char *subject, const char *keyName) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'K', keyName }
    };
    return fillPattern(objectContainerTemplate()->api().refByKey, r, ARRAY_LENGTH(r));
}

std::string ObjectContainerType::generateIterateElements(const char *subject, const char *pairName, const char *keyName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'U', keyType()->name().body().c_str() },
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject },
        { 'I', pairName },
        { 'K', keyName },
        { 'V', elementName },
        { 'F', body }
    };
    return fillPattern(objectContainerTemplate()->api().iterateElements, r, ARRAY_LENGTH(r));
}
