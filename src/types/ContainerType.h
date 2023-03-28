
#pragma once

#include <tuple>
#include "../Type.h"
#include "../ContainerTemplate.h"

template <typename... T>
class ContainerType : public Type {

public:
    virtual ~ContainerType() = default;
    inline virtual bool isIncomplete() const override { return elemType->isIncomplete(); }
    virtual const Type *actualType(TemplateInstanceCache *instanceCache) const override;
    inline const Type *elementType() const { return elemType; }

protected:
    const ContainerTemplate<T...> *containerTemplate;
    const Type *elemType;
    std::tuple<T...> templateArgs;

    inline ContainerType(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs) : Type(containerTemplate->instanceName(elementType, templateArgs...)), containerTemplate(containerTemplate), elemType(elementType), templateArgs(templateArgs...) { }

};
