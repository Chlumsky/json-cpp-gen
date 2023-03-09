
#pragma once

#include "ContainerType.h"
#include "ArrayContainerType.h"

class FixedArrayContainerTemplate;

class FixedArrayContainerType : public ContainerType<> {

public:
    FixedArrayContainerType(const FixedArrayContainerTemplate *containerTemplate, const ArrayContainerType *arrayContainerType, const Type *elementType);
    const FixedArrayContainerTemplate * fixedArrayContainerTemplate() const;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateCopyFromArrayContainer(const char *subject, const char *x) const;
    std::string generateMoveFromArrayContainer(const char *subject, const char *x) const;
    std::string generateIterateElements(const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const;

private:
    const ArrayContainerType *arrayContainerType;

};
