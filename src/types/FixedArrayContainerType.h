
#pragma once

#include "ContainerType.h"
#include "ArrayContainerType.h"

class FixedArrayContainerTemplate;

class FixedArrayContainerType : public ContainerType<const Type *> {

public:
    FixedArrayContainerType(const FixedArrayContainerTemplate *containerTemplate, const ArrayContainerType *arrayContainerType, const Type *elementType);
    const FixedArrayContainerTemplate * fixedArrayContainerTemplate() const;
    const Type * elementType() const;
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return true; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateCopyFromArrayContainer(const char *subject, const char *x) const;
    std::string generateMoveFromArrayContainer(const char *subject, const char *x) const;
    std::string generateIterateElements(const char *subject, const char *elementName, const char *body) const;

private:
    const ArrayContainerType *arrayContainerType;

};
