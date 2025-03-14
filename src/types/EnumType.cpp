
#include "EnumType.h"

#include <cassert>
#include "../ParserGenerator.h"
#include "../SerializerGenerator.h"

class EnumType::ParserSwitchTreeCaseGenerator : public ParserGenerator::SwitchTreeCaseGenerator {

public:
    inline explicit ParserSwitchTreeCaseGenerator(const EnumType *parent);
    virtual std::string operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int knownMinLength, const std::string &indent) override;

private:
    const EnumType *parent;

};

EnumType::ParserSwitchTreeCaseGenerator::ParserSwitchTreeCaseGenerator(const EnumType *parent) : parent(parent) { }

std::string EnumType::ParserSwitchTreeCaseGenerator::operator()(ParserGenerator *parserGenerator, const std::string &caseLabel, const StringType *valueType, const char *value, int, const std::string &indent) {
    std::string body;
    body += indent+"if ("+valueType->generateEqualsStringLiteral(value, parserGenerator->getJsonMemberNameLiteral(caseLabel).c_str())+") {\n";
    body += indent+INDENT "value = "+Generator::safeName(parent->valuePrefix+caseLabel)+";\n";
    body += indent+INDENT "return"+(parserGenerator->settings().noThrow ? " Error::OK" : "")+";\n";
    body += indent+"}\n";
    return body;
}

EnumType::EnumType(bool enumClass, const std::string &enumNamespace, const std::string &name, TypeName::Substance nameSubstance) :
    DirectType(TypeName(name, nameSubstance)),
    enumClass(enumClass),
    valuePrefix(enumClass && nameSubstance == TypeName::ACTUAL ? this->name().body()+"::" : enumNamespace)
{ }

EnumType::EnumType(bool enumClass, const std::string &enumNamespace, std::string &&name, TypeName::Substance nameSubstance) :
    DirectType(TypeName((std::string &&) name, nameSubstance)),
    enumClass(enumClass),
    valuePrefix(enumClass && nameSubstance == TypeName::ACTUAL ? this->name().body()+"::" : enumNamespace)
{ }

std::string EnumType::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    std::string body;
    if (values.empty())
        return indent+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_ENUM_VALUE)+";\n";
    body += generator->generateValueParse(generator->stringType(), ParserGenerator::COMMON_STRING_BUFFER, indent);
    ParserSwitchTreeCaseGenerator switchTreeCaseGenerator(this);
    body += generator->generateSwitchTree(&switchTreeCaseGenerator, StringSwitchTree::build(values.data(), values.size()).get(), generator->stringType(), ParserGenerator::COMMON_STRING_BUFFER, indent);
    body += indent+generator->generateErrorStatement(ParserGenerator::Error::UNKNOWN_ENUM_VALUE)+";";
    return body;
}

std::string EnumType::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    std::string body;
    body += indent+"switch (value) {\n";
    for (const std::string &enumValue : values)
        body += indent+INDENT "case "+Generator::safeName(valuePrefix+enumValue)+": "+generator->stringType()->generateAppendStringLiteral(SerializerGenerator::OUTPUT_STRING, ("\"\\\""+enumValue+"\\\"\"").c_str())+"; break;\n";
    body += indent+INDENT "default:\n";
    body += indent+INDENT INDENT+generator->generateErrorStatement(SerializerGenerator::Error::UNKNOWN_ENUM_VALUE)+";\n";
    body += indent+"}\n";
    return body;
}

bool EnumType::isIncomplete() const {
    return !complete;
}

EnumType *EnumType::incompleteEnumType() {
    return complete ? nullptr : this;
}

void EnumType::addValue(const std::string &value) {
    assert(!complete);
    values.push_back(value);
}

void EnumType::completeValues() {
    complete = true;
}

const std::vector<std::string> &EnumType::getValues() const {
    return values;
}

bool EnumType::isEnumClass() const {
    return enumClass;
}
