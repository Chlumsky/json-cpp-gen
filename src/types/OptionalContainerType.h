
#pragma once

#include "ContainerType.h"

class OptionalContainerTemplate;

class OptionalContainerType : public ContainerType<const Type *> {

public:
    OptionalContainerType(const OptionalContainerTemplate *containerTemplate, const Type *elementType);
    const OptionalContainerTemplate * optionalContainerTemplate() const;
    const Type * elementType() const;
    inline virtual bool isOptionalType() const override { return true; }
    inline virtual bool isArrayType() const override { return false; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateRefInitialized(const char *subject) const;
    std::string generateHasValue(const char *subject) const;
    std::string generateGetValue(const char *subject) const;

};
