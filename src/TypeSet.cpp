
#include "TypeSet.h"

#include "types/StringType.h"
#include "types/StructureType.h"
#include "container-templates/ArrayContainerTemplate.h"
#include "container-templates/StaticArrayContainerTemplate.h"
#include "container-templates/ObjectMapContainerTemplate.h"
#include "container-templates/OptionalContainerTemplate.h"

TypeSet::TypeSet() {
    addBasicType("bool", BasicType::BOOL);
    addBasicType("char", BasicType::CHAR);
    addBasicType("signed char", BasicType::SIGNED_CHAR);
    addBasicType("unsigned char", BasicType::UNSIGNED_CHAR);
    addBasicType("short", BasicType::SHORT);
    addBasicType("short int", BasicType::SHORT);
    addBasicType("signed short", BasicType::SHORT);
    addBasicType("signed short int", BasicType::SHORT);
    addBasicType("unsigned short", BasicType::UNSIGNED_SHORT);
    addBasicType("unsigned short int", BasicType::UNSIGNED_SHORT);
    addBasicType("int", BasicType::INT);
    addBasicType("signed int", BasicType::INT);
    addBasicType("unsigned", BasicType::UNSIGNED_INT);
    addBasicType("unsigned int", BasicType::UNSIGNED_INT);
    addBasicType("long", BasicType::LONG);
    addBasicType("long int", BasicType::LONG);
    addBasicType("signed long", BasicType::LONG);
    addBasicType("signed long int", BasicType::LONG);
    addBasicType("unsigned long", BasicType::UNSIGNED_LONG);
    addBasicType("unsigned long int", BasicType::UNSIGNED_LONG);
    addBasicType("long long", BasicType::LONG_LONG);
    addBasicType("long long int", BasicType::LONG_LONG);
    addBasicType("signed long long", BasicType::LONG_LONG);
    addBasicType("signed long long int", BasicType::LONG_LONG);
    addBasicType("unsigned long long", BasicType::UNSIGNED_LONG_LONG);
    addBasicType("unsigned long long int", BasicType::UNSIGNED_LONG_LONG);
    addBasicType("float", BasicType::FLOAT);
    addBasicType("double", BasicType::DOUBLE);
    addBasicType("long double", BasicType::LONG_DOUBLE);
    addBasicType("size_t", BasicType::SIZE_T);
    addBasicType("std::size_t", BasicType::SIZE_T);
    addBasicType("ptrdiff_t", BasicType::PTRDIFF_T);
    addBasicType("std::ptrdiff_t", BasicType::PTRDIFF_T);
    addBasicType("intptr_t", BasicType::INTPTR_T);
    addBasicType("std::intptr_t", BasicType::INTPTR_T);
    addBasicType("uintptr_t", BasicType::UINTPTR_T);
    addBasicType("std::uintptr_t", BasicType::UINTPTR_T);
    addBasicType("wchar_t", BasicType::WCHAR_T);
    addBasicType("char8_t", BasicType::CHAR8_T);
    addBasicType("char16_t", BasicType::CHAR16_T);
    addBasicType("char32_t", BasicType::CHAR32_T);
    addBasicType("int8_t", BasicType::INT8_T);
    addBasicType("std::int8_t", BasicType::INT8_T);
    addBasicType("uint8_t", BasicType::UINT8_T);
    addBasicType("std::uint8_t", BasicType::UINT8_T);
    addBasicType("int16_t", BasicType::INT16_T);
    addBasicType("std::int16_t", BasicType::INT16_T);
    addBasicType("uint16_t", BasicType::UINT16_T);
    addBasicType("std::uint16_t", BasicType::UINT16_T);
    addBasicType("int32_t", BasicType::INT32_T);
    addBasicType("std::int32_t", BasicType::INT32_T);
    addBasicType("uint32_t", BasicType::UINT32_T);
    addBasicType("std::uint32_t", BasicType::UINT32_T);
    addBasicType("int64_t", BasicType::INT64_T);
    addBasicType("std::int64_t", BasicType::INT64_T);
    addBasicType("uint64_t", BasicType::UINT64_T);
    addBasicType("std::uint64_t", BasicType::UINT64_T);
    add(std::unique_ptr<Type>(new StringType(StringType::STD_STRING)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new ArrayContainerTemplate(ArrayContainerTemplate::STD_VECTOR)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new ArrayContainerTemplate(ArrayContainerTemplate::STD_DEQUE)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new ArrayContainerTemplate(ArrayContainerTemplate::STD_LIST)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<int> >(new StaticArrayContainerTemplate(StaticArrayContainerTemplate::STD_ARRAY)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<const Type *> >(new ObjectMapContainerTemplate(ObjectMapContainerTemplate::STD_MAP)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new OptionalContainerTemplate(OptionalContainerTemplate::STD_OPTIONAL)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new OptionalContainerTemplate(OptionalContainerTemplate::STD_AUTO_PTR)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new OptionalContainerTemplate(OptionalContainerTemplate::STD_UNIQUE_PTR)));
    addContainerTemplate(std::unique_ptr<ContainerTemplate<> >(new OptionalContainerTemplate(OptionalContainerTemplate::STD_SHARED_PTR)));
}

void TypeSet::addBasicType(const char *name, BasicType::Type type) {
    types.insert(std::make_pair(std::string(name), std::unique_ptr<Type>(new BasicType(type))));
}

Type *TypeSet::find(const std::string &name) {
    std::map<std::string, std::unique_ptr<Type> >::iterator it = types.find(name);
    if (it != types.end())
        return it->second.get();
    return nullptr;
}

const Type *TypeSet::find(const std::string &name) const {
    std::map<std::string, std::unique_ptr<Type> >::const_iterator it = types.find(name);
    if (it != types.end())
        return it->second.get();
    return nullptr;
}

void TypeSet::add(std::unique_ptr<Type> &&type) {
    add(type->name().fullName(), std::move(type));
}

void TypeSet::add(const std::string &name, std::unique_ptr<Type> &&type) {
    types[name] = std::move(type);
}

void TypeSet::addAnonymous(std::unique_ptr<Type> &&type) {
    anonymousTypes.push_back(std::move(type));
}

template <>
TypeSet::ContainerTemplateMap<> &TypeSet::containerTemplateMap() {
    return containerTemplates;
}

template <>
TypeSet::ContainerTemplateMap<int> &TypeSet::containerTemplateMap() {
    return staticArrayContainerTemplates;
}

template <>
TypeSet::ContainerTemplateMap<const Type *> &TypeSet::containerTemplateMap() {
    return objectMapContainerTemplates;
}

const Type *TypeSet::finalizeInheritance() {
    for (const std::map<std::string, std::unique_ptr<Type> >::value_type &type : types) {
        if (StructureType *structType = dynamic_cast<StructureType *>(type.second.get()))
            if (!structType->finalizeInheritance())
                return structType;
    }
    for (const std::unique_ptr<Type> &type : anonymousTypes) {
        if (StructureType *structType = dynamic_cast<StructureType *>(type.get()))
            if (!structType->finalizeInheritance())
                return structType;
    }
    return nullptr;
}
