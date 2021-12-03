
#include "ObjectContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/ObjectContainerType.h"

ObjectContainerTemplate::ObjectContainerTemplate(const std::string &name, const Type *keyType, const ObjectContainerAPI &api) : ContainerTemplate(name), kType(keyType), containerAPI(api) { }

TypeName ObjectContainerTemplate::instanceName(const Type *elementType) const {
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<> > ObjectContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType) const {
    return std::unique_ptr<ContainerType<> >(new ObjectContainerType(this, elementType));
}
