
#include "ArrayContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/ArrayContainerType.h"

static ArrayContainerAPI stdSequenceAPI() {
    ArrayContainerAPI api;
    api.clear = "$S.clear()";
    api.refAppended = "($S.emplace_back(), $S.back())";
    api.iterateElements = "for ($T const &$E : $S) { $F }";
    return api;
}

const ArrayContainerTemplate ArrayContainerTemplate::STD_VECTOR("std::vector<$T>", stdSequenceAPI());
const ArrayContainerTemplate ArrayContainerTemplate::STD_DEQUE("std::deque<$T>", stdSequenceAPI());
const ArrayContainerTemplate ArrayContainerTemplate::STD_LIST("std::list<$T>", stdSequenceAPI());

ArrayContainerTemplate::ArrayContainerTemplate(const std::string &name, const ArrayContainerAPI &api) : ContainerTemplate(name), containerAPI(api) { }

TypeName ArrayContainerTemplate::instanceName(const Type *elementType) const {
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<const Type *> > ArrayContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType) const {
    return std::unique_ptr<ContainerType<const Type *> >(new ArrayContainerType(this, elementType));
}
