
#include "OptionalContainerTemplate.h"

#include "../pattern-fill.h"
#include "../types/OptionalContainerType.h"

static OptionalContainerAPI stdOptionalAPI() {
    OptionalContainerAPI api;
    api.clear = "$S.reset()";
    api.refInitialized = "($S = $T()).value()";
    api.hasValue = "$S.has_value()";
    api.getValue = "$S.value()";
    return api;
}

static OptionalContainerAPI stdSmartPtrAPI() {
    OptionalContainerAPI api;
    api.clear = "$S.reset()";
    api.refInitialized = "($S.reset(new $T), *$S)";
    api.hasValue = "$S.get()";
    api.getValue = "*$S";
    return api;
}

const OptionalContainerTemplate OptionalContainerTemplate::STD_OPTIONAL("std::optional<$T>", stdOptionalAPI());
const OptionalContainerTemplate OptionalContainerTemplate::STD_AUTO_PTR("std::auto_ptr<$T>", stdSmartPtrAPI());
const OptionalContainerTemplate OptionalContainerTemplate::STD_UNIQUE_PTR("std::unique_ptr<$T>", stdSmartPtrAPI());
const OptionalContainerTemplate OptionalContainerTemplate::STD_SHARED_PTR("std::shared_ptr<$T>", stdSmartPtrAPI());

OptionalContainerTemplate::OptionalContainerTemplate(const std::string &name, const OptionalContainerAPI &api) : ContainerTemplate(name), containerAPI(api) { }

TypeName OptionalContainerTemplate::instanceName(const Type *elementType) const {
    Replacer r[] = {
        { 'T', elementType->name().body().c_str() }
    };
    return TypeName(fillPattern(ContainerTemplate::name(), r, ARRAY_LENGTH(r)));
}

std::unique_ptr<ContainerType<const Type *> > OptionalContainerTemplate::instantiate(TemplateInstanceCache *, const Type *elementType) const {
    return std::unique_ptr<ContainerType<const Type *> >(new OptionalContainerType(this, elementType));
}
