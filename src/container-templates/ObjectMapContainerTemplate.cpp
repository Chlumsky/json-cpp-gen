
#include "ObjectMapContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/ObjectMapContainerType.h"

static ObjectContainerAPI stdMapAPI() {
    ObjectContainerAPI api;
    api.clear = "$S.clear()";
    api.refByKey = "$S[$K]";
    api.iterateElements = "for (const std::pair<$U, $T> &$I : $S) { $U const &$K = $I.first; $T const &$V = $I.second; $F }";
    return api;
}

const ObjectMapContainerTemplate ObjectMapContainerTemplate::STD_MAP("std::map<$K, $T>", stdMapAPI());

ObjectMapContainerTemplate::ObjectMapContainerTemplate(const std::string &name, const ObjectContainerAPI &api) : ContainerTemplate(name), containerAPI(api) { }

TypeName ObjectMapContainerTemplate::instanceName(const Type *elementType, const Type *keyType) const {
    Replacer r[] = {
        { 'K', keyType->name().body().c_str() },
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<const Type *> > ObjectMapContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType, const Type *keyType) const {
    return std::unique_ptr<ContainerType<const Type *> >(new ObjectMapContainerType(this, elementType, keyType));
}
