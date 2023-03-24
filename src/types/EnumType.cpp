
#include "EnumType.h"

#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

class EnumType::ParserSwitchTreeCaseGenerator : public ParserGenerator::SwitchTreeCaseGenerator {

public:
    inline explicit ParserSwitchTreeCaseGenerator(const EnumType *parent);
    virtual std::string operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int knownMinLength, const std::string &indent) override;

private:
    std::string prefix;

};

EnumType::ParserSwitchTreeCaseGenerator::ParserSwitchTreeCaseGenerator(const EnumType *parent) : prefix(parent->valuePrefix()) { }

std::string EnumType::ParserSwitchTreeCaseGenerator::operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int, const std::string &indent) {
    std::string body;
    body += indent+"if ("+valueType->generateEqualsStringLiteral(value, parserGenerator->getJsonMemberNameLiteral(caseLabel).c_str())+") {\n";
    body += indent+INDENT "value = "+prefix+caseLabel+";\n";
    body += indent+INDENT "return"+(parserGenerator->settings().noThrow ? " Error::OK" : "")+"; \n";
    body += indent+"}\n";
    return body;
}

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
    // TODO make str a class member to reduce the number of allocations
    body += indent+generator->stringType()->name().variableDeclaration("str")+";\n";
    body += generator->generateValueParse(generator->stringType(), "str", indent);
    ParserSwitchTreeCaseGenerator switchTreeCaseGenerator(this);
    body += generator->generateSwitchTree(&switchTreeCaseGenerator, StringSwitchTree::build(values.data(), values.size()).get(), generator->stringType(), "str", indent);
    body += indent+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_ENUM_VALUE)+";";
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

bool EnumType::isEnumClass() const {
    return enumClass;
}

bool EnumType::isFinalized() const {
    return finalized;
}

void EnumType::finalize() {
    finalized = true;
}
