
#pragma once

#include "ContainerType.h"

class StaticArrayContainerTemplate;

class StaticArrayContainerType : public ContainerType<const Type *, int> {

public:
    StaticArrayContainerType(const StaticArrayContainerTemplate *containerTemplate, const Type *elementType, int length);
    const StaticArrayContainerTemplate * staticArrayContainerTemplate() const;
    const Type * elementType() const;
    int length() const;
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return true; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateRefByIndex(const char *subject, const char *index) const;

};
