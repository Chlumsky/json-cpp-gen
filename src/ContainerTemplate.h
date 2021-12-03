
#pragma once

#include <string>
#include <map>
#include <memory>
#include "TypeName.h"
#include "Type.h"

template <typename... T>
class ContainerType;

class TemplateInstanceCache;

// Represents a container type template such as std::vector, std::unique_ptr, etc.
template <typename... T>
class ContainerTemplate {

public:
    virtual ~ContainerTemplate() = default;
    inline const std::string & name() const { return templateName; }
    virtual TypeName instanceName(T... templateArgs) const = 0;
    int templateArgIndex(char c) const;
    virtual std::unique_ptr<ContainerType<T...> > instantiate(TemplateInstanceCache *instanceCache, T... templateArgs) const = 0;

protected:
    inline explicit ContainerTemplate(const std::string &name) : templateName(name) { }
    inline explicit ContainerTemplate(std::string &&name) : templateName((std::string &&) name) { }

private:
    std::string templateName;

};

template <typename... T>
int ContainerTemplate<T...>::templateArgIndex(char c) const {
    int pos = 0;
    char prev = '\0';
    for (char cur : templateName) {
        if (cur == ',')
            ++pos;
        else if (prev == '$' && cur == c)
            return pos;
        prev = cur;
    }
    return -1;
}
