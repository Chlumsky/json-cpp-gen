
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"

class ArrayContainerTemplate : public ContainerTemplate<> {

public:
    static const ArrayContainerTemplate STD_VECTOR;
    static const ArrayContainerTemplate STD_DEQUE;
    static const ArrayContainerTemplate STD_LIST;

    ArrayContainerTemplate(const std::string &name, const ArrayContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<> > instantiate(TemplateInstanceCache *, const Type *elementType) const override;
    inline const ArrayContainerAPI & api() const { return containerAPI; }

private:
    ArrayContainerAPI containerAPI;

};
