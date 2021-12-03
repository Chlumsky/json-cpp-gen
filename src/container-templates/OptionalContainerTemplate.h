
#pragma once

#include "../ContainerTemplate.h"
#include "../Configuration.h"

class OptionalContainerTemplate : public ContainerTemplate<const Type *> {

public:
    static const OptionalContainerTemplate STD_OPTIONAL;
    static const OptionalContainerTemplate STD_AUTO_PTR;
    static const OptionalContainerTemplate STD_UNIQUE_PTR;
    static const OptionalContainerTemplate STD_SHARED_PTR;

    OptionalContainerTemplate(const std::string &name, const OptionalContainerAPI &api);
    virtual TypeName instanceName(const Type *elementType) const override;
    virtual std::unique_ptr<ContainerType<const Type *> > instantiate(TemplateInstanceCache *, const Type *elementType) const override;
    inline const OptionalContainerAPI & api() const { return containerAPI; }

private:
    OptionalContainerAPI containerAPI;

};
