
#pragma once

#include "ContainerType.h"

class ArrayContainerTemplate;

class ArrayContainerType : public ContainerType<const Type *> {

public:
    ArrayContainerType(const ArrayContainerTemplate *containerTemplate, const Type *elementType);
    const ArrayContainerTemplate * arrayContainerTemplate() const;
    const Type * elementType() const;
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return true; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateRefAppended(const char *subject) const;
    std::string generateIterateElements(const char *subject, const char *elementName, const char *body) const;

};
