
#pragma once

#include <tuple>
#include "../Type.h"
#include "../ContainerTemplate.h"

template <typename... T>
class ContainerType : public Type {

public:
    virtual ~ContainerType() = default;
    inline virtual bool isDirectType() const override { return false; }
    inline virtual bool isContainerType() const override { return true; }
    inline virtual bool isStringType() const override { return false; }
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return false; }
    inline const Type * elementType() const { return elemType; }

protected:
    const ContainerTemplate<T...> *containerTemplate;
    const Type *elemType;
    std::tuple<T...> templateArgs;

    inline ContainerType(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs) : Type(containerTemplate->instanceName(elementType, templateArgs...)), containerTemplate(containerTemplate), elemType(elementType), templateArgs(templateArgs...) { }

};
