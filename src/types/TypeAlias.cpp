
#include "TypeAlias.h"

TypeAlias::TypeAlias(const std::string &name) : Type(TypeName(name)), type(nullptr) { }

TypeAlias::TypeAlias(std::string &&name) : Type(TypeName((std::string &&) name)), type(nullptr) { }

TypeAlias::TypeAlias(const std::string &name, const Type *type) : Type(TypeName(name)), type(type) { }

TypeAlias::TypeAlias(std::string &&name, const Type *type) : Type(TypeName((std::string &&) name)), type(type) { }

std::string TypeAlias::generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const {
    return type ? type->generateParserFunctionBody(generator, indent) : std::string();
}

std::string TypeAlias::generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const {
    return type ? type->generateSerializerFunctionBody(generator, indent) : std::string();
}

const Type *TypeAlias::actualType() const {
    return type ? type->actualType() : nullptr;
}

const StringType *TypeAlias::stringType() const {
    return type ? type->stringType() : nullptr;
}

const OptionalContainerType *TypeAlias::optionalContainerType() const {
    return type ? type->optionalContainerType() : nullptr;
}

const StructureType *TypeAlias::structureType() const {
    return type ? type->structureType() : nullptr;
}

const EnumType *TypeAlias::enumType() const {
    return type ? type->enumType() : nullptr;
}

bool TypeAlias::isFinalized() const {
    return !!type;
}

bool TypeAlias::finalize(const Type *type) {
    // Detect alias cycle
    class CanaryType : public Type {
    public:
        inline CanaryType() : Type(TypeName()) { }
        inline virtual std::string generateParserFunctionBody(ParserGenerator *, const std::string &) const override { return std::string(); }
        inline virtual std::string generateSerializerFunctionBody(SerializerGenerator *, const std::string &) const override { return std::string(); }
    } cycleCanary;
    this->type = &cycleCanary;
    if (type->actualType() == &cycleCanary) {
        this->type = nullptr;
        return false;
    }
    this->type = type;
    return true;
}
