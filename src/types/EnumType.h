
#pragma once

#include <string>
#include <vector>
#include "DirectType.h"

class EnumType : public DirectType {

public:
    EnumType(const std::string &name, bool enumClass);
    EnumType(std::string &&name, bool enumClass);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    void addValue(const std::string &value);
    const std::vector<std::string> &getValues() const;
    bool isEnumClass() const;
    bool isFinalized() const;
    void finalize();

private:
    class ParserSwitchTreeCaseGenerator;

    bool enumClass;
    std::vector<std::string> values;
    bool finalized = false;

    std::string valuePrefix() const;

};
