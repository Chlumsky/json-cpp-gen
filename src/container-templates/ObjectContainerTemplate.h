
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"

class ObjectContainerTemplate : public ContainerTemplate<> {

public:
    ObjectContainerTemplate(const std::string &name, const Type *keyType, const ObjectContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<> > instantiate(TemplateInstanceCache *, const Type *elementType) const override;
    inline const Type *keyType() const { return kType; }
    inline const ObjectContainerAPI &api() const { return containerAPI; }

private:
    const Type *kType;
    ObjectContainerAPI containerAPI;

};
