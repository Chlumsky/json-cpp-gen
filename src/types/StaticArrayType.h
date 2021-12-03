
#pragma once

#include "../Type.h"

class StaticArrayType : public Type {

public:
    StaticArrayType(const Type *elementType, int length);
    virtual ~StaticArrayType() = default;
    inline virtual bool isDirectType() const override { return false; }
    inline virtual bool isContainerType() const override { return false; }
    inline virtual bool isStringType() const override { return false; }
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return false; }
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return true; }
    inline virtual bool isObjectType() const override { return false; }
    inline const Type * elementType() const { return elemType; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;

private:
    const Type *elemType;
    int length;

};
