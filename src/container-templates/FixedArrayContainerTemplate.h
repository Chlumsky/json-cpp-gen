
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"
#include "ArrayContainerTemplate.h"

class FixedArrayContainerTemplate : public ContainerTemplate<const Type *> {

public:
    FixedArrayContainerTemplate(const std::string &name, const ArrayContainerTemplate *arrayContainerTemplate, const FixedArrayContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<const Type *> > instantiate(TemplateInstanceCache *instanceCache, const Type *elementType) const override;
    inline const FixedArrayContainerAPI & api() const { return containerAPI; }

private:
    const ArrayContainerTemplate *arrayContainerTemplate;
    FixedArrayContainerAPI containerAPI;

};
