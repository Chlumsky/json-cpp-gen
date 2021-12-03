
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"
#include "../types/StringType.h"

class ObjectMapContainerTemplate : public ContainerTemplate<const StringType *, const Type *> {

public:
    static const ObjectMapContainerTemplate STD_MAP;

    ObjectMapContainerTemplate(const std::string &name, const ObjectContainerAPI &api);
    virtual TypeName instanceName(const StringType *keyType, const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<const StringType *, const Type *> > instantiate(TemplateInstanceCache *, const StringType *keyType, const Type *elementType) const override;
    inline const ObjectContainerAPI & api() const { return containerAPI; }

private:
    ObjectContainerAPI containerAPI;

};
