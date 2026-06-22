
#pragma once

#include "DirectType.h"

/// Custom named integer type that is considered distinct from basic types like int, int32_t, etc.

class IntegerType : public DirectType {

public:
    IntegerType(bool signedness, const std::string &name);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;

private:
    bool signedness;

};
