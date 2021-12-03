
#include "TemplateInstanceCache.h"

template <>
TemplateInstanceCache::Cache<> & TemplateInstanceCache::cache() {
    return containerCache;
}

template <>
TemplateInstanceCache::Cache<int> & TemplateInstanceCache::cache() {
    return staticArrayContainerCache;
}

template <>
TemplateInstanceCache::Cache<const Type *> & TemplateInstanceCache::cache() {
    return objectMapContainerCache;
}
