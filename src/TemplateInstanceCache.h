
#pragma once

#include <map>
#include <memory>
#include "Type.h"
#include "Namespace.h"
#include "types/StringType.h"
#include "ContainerTemplate.h"

class TemplateInstanceCache {

public:
    template <typename... T>
    const SymbolPtr get(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs);

private:
    template <typename... T>
    using Cache = std::map<std::tuple<const ContainerTemplate<T...> *, const Type *, T...>, SymbolPtr>;

    Cache<> containerCache;
    Cache<int> staticArrayContainerCache;
    Cache<const Type *> objectMapContainerCache;

    template <typename... T>
    Cache<T...> &cache();

};

template <typename... T>
const SymbolPtr TemplateInstanceCache::get(const ContainerTemplate<T...> *containerTemplate, const Type *elementType, T... templateArgs) {
    Cache<T...> &c = cache<T...>();
    std::tuple<const ContainerTemplate<T...> *, const Type *, T...> key(containerTemplate, elementType, templateArgs...);
    typename Cache<T...>::iterator it = c.find(key);
    if (it != c.end())
        return it->second;
    SymbolPtr symbol(new Symbol);
    symbol->type = containerTemplate->instantiate(this, elementType, templateArgs...);
    c.insert(std::make_pair(key, symbol));
    return symbol;
}
