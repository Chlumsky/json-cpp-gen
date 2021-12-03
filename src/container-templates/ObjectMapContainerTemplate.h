
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"
#include "../types/StringType.h"

class ObjectMapContainerTemplate : public ContainerTemplate<const Type *> {

public:
    static const ObjectMapContainerTemplate STD_MAP;

    ObjectMapContainerTemplate(const std::string &name, const ObjectContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType, const Type *keyType) const override;
    virtual std::unique_ptr<ContainerType<const Type *> > instantiate(TemplateInstanceCache *, const Type *elementType, const Type *keyType) const override;
    inline const ObjectContainerAPI & api() const { return containerAPI; }

private:
    ObjectContainerAPI containerAPI;

};
