
#include "EnumType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

EnumType::EnumType(const std::string &name, bool enumClass) : DirectType(TypeName(name)), enumClass(enumClass) { }

EnumType::EnumType(std::string &&name, bool enumClass) : DirectType(TypeName((std::string &&) name)), enumClass(enumClass) { }

std::string EnumType::valuePrefix() const {
    if (enumClass)
        return name().body()+"::";
    std::string prefix = name().body();
    while (!prefix.empty() && prefix.back() != ':')
        prefix.pop_back();
    return prefix;
}

std::string EnumType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    if (values.empty())
        return indent+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_ENUM_VALUE)+";\n";
    std::string prefix = valuePrefix();
    // TODO make str a class member to reduce the number of allocations
    body += indent+generator->stringType()->name().variableDeclaration("str")+";\n";
    body += generator->generateValueParse(generator->stringType(), "str", indent);
    body += indent;
    for (const std::string &enumValue : values) {
        body += "if (str == "+generator->getJsonEnumValueLiteral(enumValue)+")\n";
        body += indent+INDENT "value = "+prefix+enumValue+";\n";
        body += indent+"else ";
    }
    body.back() = '\n';
    body += indent+INDENT+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_ENUM_VALUE)+";\n";
    return body;
}

std::string EnumType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    std::string prefix = valuePrefix();
    body += indent+"switch (value) {\n";
    for (const std::string &enumValue : values)
        body += indent+INDENT "case "+prefix+enumValue+": write(\"\\\""+enumValue+"\\\"\"); break;\n";
    body += indent+INDENT "default:\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(SerializerGenerator::Error::UNKNOWN_ENUM_VALUE)+";\n";
    body += indent+"}\n";
    return body;
}

void EnumType::addValue(const std::string &value) {
    values.push_back(value);
}

const std::vector<std::string> & EnumType::getValues() const {
    return values;
}
