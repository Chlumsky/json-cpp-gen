
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"
#include "ArrayContainerTemplate.h"

class StaticArrayContainerTemplate : public ContainerTemplate<int> {

public:
    static const StaticArrayContainerTemplate STD_ARRAY;

    StaticArrayContainerTemplate(const std::string &name, const StaticArrayContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType, int length) const override;
    virtual std::unique_ptr<ContainerType<int> > instantiate(TemplateInstanceCache *, const Type *elementType, int length) const override;
    inline const StaticArrayContainerAPI & api() const { return containerAPI; }

private:
    StaticArrayContainerAPI containerAPI;

};
