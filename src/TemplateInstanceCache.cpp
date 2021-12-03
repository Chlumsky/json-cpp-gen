
#include "TemplateInstanceCache.h"

template <>
TemplateInstanceCache::Cache<const Type *> & TemplateInstanceCache::cache() {
    return containerCache;
}

template <>
TemplateInstanceCache::Cache<const StringType *, const Type *> & TemplateInstanceCache::cache() {
    return objectMapContainerCache;
}

template <>
TemplateInstanceCache::Cache<const Type *, int> & TemplateInstanceCache::cache() {
    return staticArrayContainerCache;
}
