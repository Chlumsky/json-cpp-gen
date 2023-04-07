
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Type.h"
#include "types/BasicType.h"
#include "types/ContainerType.h"
#include "types/StructureType.h"
#include "types/EnumType.h"
#include "types/TypeAlias.h"
#include "ContainerTemplate.h"
#include "TemplateInstanceCache.h"

#define UNNAMED_PREFIX "Unnamed"

class TypeSet {

public:
    TypeSet();
    Type *find(const std::string &name);
    const Type *find(const std::string &name) const;
    void add(std::unique_ptr<Type> &&type);
    void add(const std::string &name, std::unique_ptr<Type> &&type);
    bool addAlias(const std::string &aliasName, const std::string &actualName);
    StructureType *newUnnamedStruct();
    EnumType *newUnnamedEnum(bool enumClass, const std::string &enumNamespace);

    template <typename... T>
    ContainerTemplate<T...> *findContainerTemplate(const std::string &name);
    template <typename... T>
    const ContainerTemplate<T...> *findContainerTemplate(const std::string &name) const;
    template <typename... T>
    void addContainerTemplate(std::unique_ptr<ContainerTemplate<T...> > &&containerTemplate);
    template <typename... T>
    const ContainerType<T...> *getContainerType(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs);

    // Returns null on success, otherwise the type that failed
    const Type *compile();

private:
    template <typename... T>
    using ContainerTemplateMap = std::map<std::string, std::unique_ptr<ContainerTemplate<T...> > >;

    std::map<std::string, std::unique_ptr<Type> > types;
    std::vector<std::unique_ptr<Type> > unnamedTypes;
    TemplateInstanceCache templateInstanceCache;
    ContainerTemplateMap<> containerTemplates;
    ContainerTemplateMap<int> staticArrayContainerTemplates;
    ContainerTemplateMap<const Type *> objectMapContainerTemplates;
    std::map<std::string, std::string> aliases;

    std::vector<std::pair<TypeAlias *, std::string> > aliasTypes;

    void addBasicType(const char *name, BasicType::Type type);
    const std::string &resolveAlias(const std::string &alias) const;
    template <typename... T>
    ContainerTemplateMap<T...> &containerTemplateMap();
    template <typename... T>
    const ContainerTemplateMap<T...> &containerTemplateMap() const;

    static std::string normalizeAbsoluteTypeName(const std::string &name);

};

template <typename... T>
ContainerTemplate<T...> *TypeSet::findContainerTemplate(const std::string &name) {
    ContainerTemplateMap<T...> &map = containerTemplateMap<T...>();
    typename ContainerTemplateMap<T...>::iterator it = map.find(resolveAlias(name));
    if (it != map.end())
        return it->second.get();
    return nullptr;
}

template <typename... T>
const ContainerTemplate<T...> *TypeSet::findContainerTemplate(const std::string &name) const {
    const ContainerTemplateMap<T...> &map = containerTemplateMap<T...>();
    typename ContainerTemplateMap<T...>::const_iterator it = map.find(resolveAlias(name));
    if (it != map.end())
        return it->second.get();
    return nullptr;
}

template <typename... T>
void TypeSet::addContainerTemplate(std::unique_ptr<ContainerTemplate<T...> > &&containerTemplate) {
    const std::string &name = containerTemplate->name();
    size_t templateNameEnd = name.find_first_of('<');
    containerTemplateMap<T...>()[templateNameEnd == std::string::npos ? name : name.substr(0, templateNameEnd)] = std::move(containerTemplate);
}

template <typename... T>
const ContainerType<T...> *TypeSet::getContainerType(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs) {
    if (containerTemplate)
        return templateInstanceCache.get(containerTemplate, elementType, templateArgs...);
    return nullptr;
}
