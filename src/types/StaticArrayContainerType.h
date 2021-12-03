
#pragma once

#include "ContainerType.h"

class StaticArrayContainerTemplate;

class StaticArrayContainerType : public ContainerType<int> {

public:
    StaticArrayContainerType(const StaticArrayContainerTemplate *containerTemplate, const Type *elementType, int length);
    const StaticArrayContainerTemplate * staticArrayContainerTemplate() const;
    int length() const;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateRefByIndex(const char *subject, const char *index) const;

};
