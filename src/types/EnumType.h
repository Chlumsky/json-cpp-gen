
#pragma once

#include <string>
#include <vector>
#include "DirectType.h"

class EnumType : public DirectType {

public:
    EnumType(bool enumClass, const std::string &enumNamespace, const std::string &name, TypeName::Substance nameSubstance = TypeName::ACTUAL);
    EnumType(bool enumClass, const std::string &enumNamespace, std::string &&name, TypeName::Substance nameSubstance = TypeName::ACTUAL);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    inline virtual const EnumType *enumType() const override { return this; }
    virtual EnumType *enumPrototype() override;
    void addValue(const std::string &value);
    const std::vector<std::string> &getValues() const;
    bool isEnumClass() const;
    bool isFinalized() const;
    void finalize();

private:
    class ParserSwitchTreeCaseGenerator;

    bool enumClass;
    std::string valuePrefix;
    std::vector<std::string> values;
    bool finalized = false;

};
