
#pragma once

#include "StringType.h"
#include "ContainerType.h"

class ObjectMapContainerTemplate;

class ObjectMapContainerType : public ContainerType<const StringType *, const Type *> {

public:
    ObjectMapContainerType(const ObjectMapContainerTemplate *containerTemplate, const StringType *keyType, const Type *elementType);
    const ObjectMapContainerTemplate * objectMapContainerTemplate() const;
    const StringType * keyType() const;
    const Type * elementType() const;
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return false; }
    inline virtual bool isObjectType() const override { return true; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateRefByKey(const char *subject, const char *keyName) const;
    std::string generateIterateElements(const char *subject, const char *pairName, const char *keyName, const char *elementName, const char *body) const;

};
