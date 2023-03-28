
#pragma once

#include "ContainerType.h"

class OptionalContainerTemplate;

class OptionalContainerType : public ContainerType<> {

public:
    OptionalContainerType(const OptionalContainerTemplate *containerTemplate, const Type *elementType);
    const OptionalContainerTemplate *optionalContainerTemplate() const;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    inline virtual const OptionalContainerType *optionalContainerType() const override { return this; }
    std::string generateClear(const char *subject) const;
    std::string generateRefInitialized(const char *subject) const;
    std::string generateHasValue(const char *subject) const;
    std::string generateGetValue(const char *subject) const;

};
