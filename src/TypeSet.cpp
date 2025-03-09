
#include "TypeSet.h"

#include <cctype>
#include "types/TypeAlias.h"
#include "types/StringType.h"
#include "types/StaticArrayType.h"
#include "container-templates/ArrayContainerTemplate.h"
#include "container-templates/StaticArrayContainerTemplate.h"
#include "container-templates/ObjectMapContainerTemplate.h"
#include "container-templates/OptionalContainerTemplate.h"
#include "HeaderParser.h"

TypeSet::TypeSet() : rootNamespace(new Namespace(nullptr)) {
    rootNamespace->establishSymbol("std", true);
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
    addStdBasicType("size_t", BasicType::SIZE_T);
    addStdBasicType("ptrdiff_t", BasicType::PTRDIFF_T);
    addStdBasicType("intptr_t", BasicType::INTPTR_T);
    addStdBasicType("uintptr_t", BasicType::UINTPTR_T);
    addStdBasicType("wchar_t", BasicType::WCHAR_T);
    addStdBasicType("char8_t", BasicType::CHAR8_T);
    addStdBasicType("char16_t", BasicType::CHAR16_T);
    addStdBasicType("char32_t", BasicType::CHAR32_T);
    addStdBasicType("int8_t", BasicType::INT8_T);
    addStdBasicType("uint8_t", BasicType::UINT8_T);
    addStdBasicType("int16_t", BasicType::INT16_T);
    addStdBasicType("uint16_t", BasicType::UINT16_T);
    addStdBasicType("int32_t", BasicType::INT32_T);
    addStdBasicType("uint32_t", BasicType::UINT32_T);
    addStdBasicType("int64_t", BasicType::INT64_T);
    addStdBasicType("uint64_t", BasicType::UINT64_T);
    rootNamespace->establishSymbol(StringType::STD_STRING.name().body(), false)->type = std::unique_ptr<Type>(new StringType(StringType::STD_STRING));
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
    rootNamespace->establishSymbol(name, false)->type = std::unique_ptr<Type>(new BasicType(type));
}

void TypeSet::addStdBasicType(const char *name, BasicType::Type type) {
    addBasicType(name, type);
    rootNamespace->findSymbol("std", false)->ns->establishSymbol(name, false)->type = std::unique_ptr<Type>(new BasicType(type));
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

Namespace &TypeSet::root() {
    return *rootNamespace;
}

const Type *TypeSet::find(const std::string &name) const {
    return symbolType(rootNamespace->findSymbol(name, false));
}

bool TypeSet::addAlias(const std::string &aliasName, const std::string &actualName) {
    if (aliasName.empty() || actualName.empty())
        return true;
    for (char c : actualName) {
        if (!(isalnum(c) || c == '_' || c == ':')) {
            // Not a simple lexical alias - create new alias type
            if (SymbolPtr symbol = rootNamespace->establishSymbol(aliasName, false)) {
                symbol->type = std::unique_ptr<Type>(new TypeAlias(aliasName));
                return true;
            } else
                return false;
        }
    }
    std::string &aliasValue = aliases[normalizeAbsoluteTypeName(aliasName)];
    std::string normalizedActualName = normalizeAbsoluteTypeName(actualName);
    if (&resolveAlias(normalizedActualName) == &aliasValue)
        return false;
    aliasValue = normalizedActualName;
    return true;
}

SymbolPtr TypeSet::newUnnamedStruct() {
    SymbolPtr symbol(new Symbol);
    symbol->type = std::unique_ptr<Type>(new StructureType(UNNAMED_PREFIX+std::to_string(unnamedTypes.size()), TypeName::VIRTUAL));
    unnamedTypes.push_back(symbol);
    return symbol;
}

SymbolPtr TypeSet::newUnnamedEnum(bool enumClass, const std::string &enumNamespace) {
    SymbolPtr symbol(new Symbol);
    symbol->type = std::unique_ptr<Type>(new EnumType(enumClass, enumNamespace, UNNAMED_PREFIX+std::to_string(unnamedTypes.size()), TypeName::VIRTUAL));
    unnamedTypes.push_back(symbol);
    return symbol;
}

SymbolPtr TypeSet::getStaticArray(const Type *elementType, int arrayLength) {
    SymbolPtr &symbol = staticArrayTypeCache[std::make_pair(elementType, arrayLength)];
    if (!symbol)
        symbol = SymbolPtr(new Symbol);
    if (!symbol->type)
        symbol->type = std::unique_ptr<Type>(new StaticArrayType(elementType, arrayLength));
    return symbol;
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
    int resultFlags;
    do { // Repeat in case of back and forth dependencies
        resultFlags = rootNamespace->compileTypes(&templateInstanceCache);
        for (const SymbolPtr &symbol : unnamedTypes)
            resultFlags |= symbol->type->compile(&templateInstanceCache);
    } while (resultFlags&Type::CHANGE_FLAG);
    // Cyclic dependency detected
    if (resultFlags&Type::BASE_DEPENDENCY_FLAG) {
        const Type *dependentType = nullptr;
        if (rootNamespace->compileTypes(&templateInstanceCache, Type::BASE_DEPENDENCY_FLAG, &dependentType)&Type::BASE_DEPENDENCY_FLAG)
            return dependentType;
        for (const SymbolPtr &symbol : unnamedTypes)
            if (symbol->type->compile(&templateInstanceCache)&Type::BASE_DEPENDENCY_FLAG)
                return symbol->type.get();
    }
    return nullptr;
}

std::string TypeSet::normalizeAbsoluteTypeName(const std::string &name) {
    if (name.size() > 2 && name[0] == ':' && name[1] == ':')
        return name.substr(2);
    return name;
}
