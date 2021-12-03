
#pragma once

#include <string>
#include <vector>
#include "DirectType.h"

class EnumType : public DirectType {

public:
    explicit EnumType(const std::string &name, bool enumClass);
    explicit EnumType(std::string &&name, bool enumClass);
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return true; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    void addValue(const std::string &value);
    const std::vector<std::string> & getValues() const;

private:
    bool enumClass;
    std::vector<std::string> values;

};
