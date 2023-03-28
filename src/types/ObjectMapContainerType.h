
#pragma once

#include "StringType.h"
#include "ContainerType.h"

class ObjectMapContainerTemplate;

class ObjectMapContainerType : public ContainerType<const Type *> {

public:
    ObjectMapContainerType(const ObjectMapContainerTemplate *containerTemplate, const Type *elementType, const Type *keyType);
    const ObjectMapContainerTemplate *objectMapContainerTemplate() const;
    const Type *keyType() const;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    virtual bool isIncomplete() const override;
    virtual const Type *actualType(TemplateInstanceCache *instanceCache) const override;
    std::string generateClear(const char *subject) const;
    std::string generateRefByKey(const char *subject, const char *keyName) const;
    std::string generateIterateElements(const char *subject, const char *iteratorName, const char *endIteratorName, const char *keyName, const char *elementName, const char *body) const;

};
