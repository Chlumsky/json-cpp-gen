
#include "ConstStringType.h"

#include "../pattern-fill.h"
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

ConstStringType::ConstStringType(const std::string &name, const StringType *stringType, const ConstStringAPI &api) : Type(TypeName(name)), stringType(stringType), api(api) { }

ConstStringType::ConstStringType(std::string &&name, const StringType *stringType, const ConstStringAPI &api) : Type(TypeName((std::string &&) name)), stringType(stringType), api(api) { }

std::string ConstStringType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+stringType->name().variableDeclaration("str")+";\n";
    body += generator->generateValueParse(stringType, "str", indent);
    body += indent+generateMoveFromString(indent, "value", "str")+";\n";
    return body;
}

std::string ConstStringType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'\"'")+";\n";
    body += indent+generateIterateChars(indent, "value", "i", "end", "c", "writeEscaped(c);")+"\n";
    body += indent+generator->stringType()->generateAppendChar(indent, SerializerGenerator::OUTPUT_STRING, "'\"'")+";\n";
    return body;
}

std::string ConstStringType::generateCopyFromString(const std::string &indent, const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.copyFromString, r, ARRAY_LENGTH(r), indent);
}

std::string ConstStringType::generateMoveFromString(const std::string &indent, const char *subject, const char *x) const {
    Replacer r[] = {
        { 'S', subject },
        { 'X', x }
    };
    return fillPattern(api.moveFromString, r, ARRAY_LENGTH(r), indent);
}

std::string ConstStringType::generateIterateChars(const std::string &indent, const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const {
    Replacer r[] = {
        { 'S', subject },
        { 'I', iteratorName },
        { 'Z', endIteratorName },
        { 'E', elementName },
        { 'F', body }
    };
    return fillPattern(api.iterateChars, r, ARRAY_LENGTH(r), indent);
}
