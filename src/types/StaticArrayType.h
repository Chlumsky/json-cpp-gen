
#pragma once

#include "../Type.h"

class StaticArrayType : public Type {

public:
    StaticArrayType(const Type *elementType, int length);
    virtual ~StaticArrayType() = default;
    inline const Type *elementType() const { return elemType; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;

private:
    const Type *elemType;
    int length;

};
