
#include "TypeSet.h"

#include <cctype>
#include "types/StringType.h"
#include "container-templates/ArrayContainerTemplate.h"
#include "container-templates/StaticArrayContainerTemplate.h"
#include "container-templates/ObjectMapContainerTemplate.h"
#include "container-templates/OptionalContainerTemplate.h"
#include "HeaderParser.h"

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

const std::string &TypeSet::resolveAlias(const std::string &alias) const {
    std::map<std::string, std::string>::const_iterator it = aliases.find(alias), recursiveIt = aliases.end();
    while (it != aliases.end()) {
        recursiveIt = it;
        it = aliases.find(it->second);
    }
    if (recursiveIt != aliases.end())
        return recursiveIt->second;
    return alias;
}

Type *TypeSet::find(const std::string &name) {
    std::map<std::string, std::unique_ptr<Type> >::iterator it = types.find(resolveAlias(name));
    if (it != types.end())
        return it->second.get();
    return nullptr;
}

const Type *TypeSet::find(const std::string &name) const {
    std::map<std::string, std::unique_ptr<Type> >::const_iterator it = types.find(resolveAlias(name));
    if (it != types.end())
        return it->second.get();
    return nullptr;
}

void TypeSet::add(std::unique_ptr<Type> &&type) {
    add(normalizeAbsoluteTypeName(type->name().fullName()), std::move(type));
}

void TypeSet::add(const std::string &name, std::unique_ptr<Type> &&type) {
    types[name] = std::move(type);
}

bool TypeSet::addAlias(const std::string &aliasName, const std::string &actualName) {
    if (aliasName.empty() || actualName.empty())
        return true;
    for (char c : actualName) {
        if (!(isalnum(c) || c == '_' || c == ':')) {
            // Not a simple lexical alias - create new alias type
            TypeAlias *aliasType = new TypeAlias(aliasName);
            add(std::unique_ptr<Type>(aliasType));
            aliasTypes.emplace_back(aliasType, actualName);
            return true;
        }
    }
    std::string &aliasValue = aliases[normalizeAbsoluteTypeName(aliasName)];
    std::string normalizedActualName = normalizeAbsoluteTypeName(actualName);
    if (&resolveAlias(normalizedActualName) == &aliasValue)
        return false;
    aliasValue = normalizedActualName;
    return true;
}

StructureType *TypeSet::newUnnamedStruct() {
    StructureType *structType = new StructureType(UNNAMED_PREFIX+std::to_string(unnamedTypes.size()), TypeName::VIRTUAL);
    unnamedTypes.push_back(std::unique_ptr<Type>(structType));
    return structType;
}

EnumType *TypeSet::newUnnamedEnum(bool enumClass, const std::string &enumNamespace) {
    EnumType *enumType = new EnumType(enumClass, enumNamespace, UNNAMED_PREFIX+std::to_string(unnamedTypes.size()), TypeName::VIRTUAL);
    unnamedTypes.push_back(std::unique_ptr<Type>(enumType));
    return enumType;
}

template <>
TypeSet::ContainerTemplateMap<> &TypeSet::containerTemplateMap() {
    return containerTemplates;
}

template <>
const TypeSet::ContainerTemplateMap<> &TypeSet::containerTemplateMap() const {
    return containerTemplates;
}

template <>
TypeSet::ContainerTemplateMap<int> &TypeSet::containerTemplateMap() {
    return staticArrayContainerTemplates;
}

template <>
const TypeSet::ContainerTemplateMap<int> &TypeSet::containerTemplateMap() const {
    return staticArrayContainerTemplates;
}

template <>
TypeSet::ContainerTemplateMap<const Type *> &TypeSet::containerTemplateMap() {
    return objectMapContainerTemplates;
}

template <>
const TypeSet::ContainerTemplateMap<const Type *> &TypeSet::containerTemplateMap() const {
    return objectMapContainerTemplates;
}

const Type *TypeSet::compile() {
    // Resolve type aliases
    for (const std::pair<TypeAlias *, std::string> &aliasType : aliasTypes) {
        if (const Type *type = parseType(*this, aliasType.second)) {
            if (!aliasType.first->finalize(type))
                return aliasType.first;
        } // else bad alias
    }
    aliasTypes.clear();
    int resultFlags;
    do { // Repeat in case of back and forth dependencies
        resultFlags = 0;
        for (const std::map<std::string, std::unique_ptr<Type> >::value_type &type : types)
            resultFlags |= type.second->compile(&templateInstanceCache);
        for (const std::unique_ptr<Type> &type : unnamedTypes)
            resultFlags |= type->compile(&templateInstanceCache);
    } while (resultFlags&Type::CHANGE_FLAG);
    // Cyclic dependency detected
    if (resultFlags&Type::BASE_DEPENDENCY_FLAG) {
        for (const std::map<std::string, std::unique_ptr<Type> >::value_type &type : types)
            if (type.second->compile(&templateInstanceCache)&Type::BASE_DEPENDENCY_FLAG)
                return type.second.get();
        for (const std::unique_ptr<Type> &type : unnamedTypes)
            if (type->compile(&templateInstanceCache)&Type::BASE_DEPENDENCY_FLAG)
                return type.get();
    }
    return nullptr;
}

std::string TypeSet::normalizeAbsoluteTypeName(const std::string &name) {
    if (name.size() > 2 && name[0] == ':' && name[1] == ':')
        return name.substr(2);
    return name;
}
