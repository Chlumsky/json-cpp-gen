
#pragma once

#include "StringType.h"
#include "ContainerType.h"

class ObjectContainerTemplate;

class ObjectContainerType : public ContainerType<> {

public:
    ObjectContainerType(const ObjectContainerTemplate *containerTemplate, const Type *elementType);
    const ObjectContainerTemplate * objectContainerTemplate() const;
    const Type * keyType() const;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateRefByKey(const char *subject, const char *keyName) const;
    std::string generateIterateElements(const char *subject, const char *pairName, const char *keyName, const char *elementName, const char *body) const;

};
