
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"
#include "../types/StringType.h"

class ObjectContainerTemplate : public ContainerTemplate<const Type *> {

public:
    ObjectContainerTemplate(const std::string &name, const StringType *keyType, const ObjectContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<const Type *> > instantiate(TemplateInstanceCache *, const Type *elementType) const override;
    inline const StringType * keyType() const { return keyStringType; }
    inline const ObjectContainerAPI & api() const { return containerAPI; }

private:
    const StringType *keyStringType;
    ObjectContainerAPI containerAPI;

};
