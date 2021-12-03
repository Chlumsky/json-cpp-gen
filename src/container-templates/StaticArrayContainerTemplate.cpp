
#include "StaticArrayContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/StaticArrayContainerType.h"

static StaticArrayContainerAPI stdArrayAPI() {
    StaticArrayContainerAPI api;
    api.refByIndex = "$S[$I]";
    return api;
}

const StaticArrayContainerTemplate StaticArrayContainerTemplate::STD_ARRAY("std::array<$T, $N>", stdArrayAPI());

StaticArrayContainerTemplate::StaticArrayContainerTemplate(const std::string &name, const StaticArrayContainerAPI &api) : ContainerTemplate(name), containerAPI(api) { }

TypeName StaticArrayContainerTemplate::instanceName(const Type *elementType, int length) const {
    std::string lengthStr = std::to_string(length);
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() },
        { 'N', lengthStr.c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<const Type *, int> > StaticArrayContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType, int length) const {
    return std::unique_ptr<ContainerType<const Type *, int> >(new StaticArrayContainerType(this, elementType, length));
}
