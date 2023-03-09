
#include "ArrayContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/ArrayContainerType.h"

static ArrayContainerAPI stdSequenceAPI(const std::string &templateName) {
    ArrayContainerAPI api;
    api.clear = "$S.clear()";
    api.refAppended = "($S.resize($S.size()+1), $S.back())";
    api.iterateElements = "for ("+templateName+"<$T>::const_iterator $I = $S.begin(), $Z = $S.end(); $I != $Z; ++$I) { $T const &$E = *$I; $F }";
    return api;
}

const ArrayContainerTemplate ArrayContainerTemplate::STD_VECTOR("std::vector<$T>", stdSequenceAPI("std::vector"));
const ArrayContainerTemplate ArrayContainerTemplate::STD_DEQUE("std::deque<$T>", stdSequenceAPI("std::deque"));
const ArrayContainerTemplate ArrayContainerTemplate::STD_LIST("std::list<$T>", stdSequenceAPI("std::list"));

ArrayContainerTemplate::ArrayContainerTemplate(const std::string &name, const ArrayContainerAPI &api) : ContainerTemplate(name), containerAPI(api) { }

TypeName ArrayContainerTemplate::instanceName(const Type *elementType) const {
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<> > ArrayContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType) const {
    return std::unique_ptr<ContainerType<> >(new ArrayContainerType(this, elementType));
}
