
#include "OptionalContainerType.h"

#include "../pattern-fill.h"
#include "../container-templates/OptionalContainerTemplate.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

OptionalContainerType::OptionalContainerType(const OptionalContainerTemplate *containerTemplate, const Type *elementType) : ContainerType(containerTemplate, elementType) { }

const OptionalContainerTemplate *OptionalContainerType::optionalContainerTemplate() const {
    return static_cast<const OptionalContainerTemplate *>(containerTemplate);
}

std::string OptionalContainerType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"skipWhitespace();\n";
    body += indent+"if ("+ParserGenerator::generateMatchKeyword("null")+")\n";
    body += indent+INDENT+generateClear("value")+";\n";
    std::string elemRef = generateRefInitialized("value");
    if (generator->settings().noThrow) {
        body += indent+"else if (Error error = "+generator->generateParserFunctionCall(elementType(), elemRef)+")\n";
        body += indent+INDENT "return error;\n";
    } else {
        body += indent+"else\n";
        body += indent+INDENT+generator->generateParserFunctionCall(elementType(), elemRef)+";\n";
    }
    return body;
}

std::string OptionalContainerType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"if ("+generateHasValue("value")+")\n";
    body += generator->generateValueSerialization(elementType(), generateGetValue("value"), indent+INDENT);
    body += indent+"else\n";
    body += indent+INDENT "write(\"null\");\n";
    return body;
}

std::string OptionalContainerType::generateClear(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(optionalContainerTemplate()->api().clear, r, ARRAY_LENGTH(r));
}

std::string OptionalContainerType::generateRefInitialized(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(optionalContainerTemplate()->api().refInitialized, r, ARRAY_LENGTH(r));
}

std::string OptionalContainerType::generateHasValue(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(optionalContainerTemplate()->api().hasValue, r, ARRAY_LENGTH(r));
}

std::string OptionalContainerType::generateGetValue(const char *subject) const {
    Replacer r[] = {
        { 'T', elementType()->name().body().c_str() },
        { 'S', subject }
    };
    return fillPattern(optionalContainerTemplate()->api().getValue, r, ARRAY_LENGTH(r));
}
