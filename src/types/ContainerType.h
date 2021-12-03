
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

protected:
    const ContainerTemplate<T...> *containerTemplate;
    std::tuple<T...> templateArgs;

    inline explicit ContainerType(const ContainerTemplate<T...> *containerTemplate, T... templateArgs) : Type(containerTemplate->instanceName(templateArgs...)), containerTemplate(containerTemplate), templateArgs(templateArgs...) { }

};
