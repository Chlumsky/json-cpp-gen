
#include "ContainerType.h"

#include "../TemplateInstanceCache.h"

template <>
const Type *ContainerType<>::actualType(TemplateInstanceCache *instanceCache) const {
    if (instanceCache) {
        const Type *actualElemType = elemType->actualType(instanceCache);
        if (actualElemType != elemType)
            return instanceCache->get(containerTemplate, actualElemType);
    }
    return this;
}

template <>
const Type *ContainerType<int>::actualType(TemplateInstanceCache *instanceCache) const {
    if (instanceCache) {
        const Type *actualElemType = elemType->actualType(instanceCache);
        if (actualElemType != elemType)
            return instanceCache->get(containerTemplate, actualElemType, std::get<0>(templateArgs));
    }
    return this;
}

template <>
const Type *ContainerType<const Type *>::actualType(TemplateInstanceCache *instanceCache) const {
    if (instanceCache) {
        const Type *actualElemType = elemType->actualType(instanceCache);
        const Type *actualArg0Type = std::get<0>(templateArgs) ? std::get<0>(templateArgs)->actualType(instanceCache) : nullptr;
        if (actualElemType != elemType || actualArg0Type != std::get<0>(templateArgs))
            return instanceCache->get(containerTemplate, actualElemType, actualArg0Type);
    }
    return this;
}
