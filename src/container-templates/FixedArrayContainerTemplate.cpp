
#include "FixedArrayContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/ArrayContainerType.h"
#include "../types/FixedArrayContainerType.h"
#include "../TemplateInstanceCache.h"

FixedArrayContainerTemplate::FixedArrayContainerTemplate(const std::string &name, const ArrayContainerTemplate *arrayContainerTemplate, const FixedArrayContainerAPI &api) : ContainerTemplate(name), arrayContainerTemplate(arrayContainerTemplate), containerAPI(api) { }

TypeName FixedArrayContainerTemplate::instanceName(const Type *elementType) const {
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<> > FixedArrayContainerTemplate::instantiate(TemplateInstanceCache *instanceCache, const Type *elementType) const {
    return std::unique_ptr<ContainerType<> >(new FixedArrayContainerType(this, static_cast<const ArrayContainerType *>(instanceCache->get(arrayContainerTemplate, elementType)), elementType));
}
