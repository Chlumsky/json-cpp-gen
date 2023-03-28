
#pragma once

#include <string>
#include "TypeName.h"

class ParserGenerator;
class SerializerGenerator;

class Type;
class StringType;
class StructureType;
class EnumType;
class OptionalContainerType;

class Type {

public:
    static constexpr int CHANGE_FLAG = 0x01;
    static constexpr int BASE_DEPENDENCY_FLAG = 0x02;

    virtual ~Type() = default;
    inline const TypeName &name() const { return typeName; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const = 0;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const = 0;
    inline virtual const Type *actualType() const { return this; }
    inline virtual const StringType *stringType() const { return nullptr; }
    inline virtual const OptionalContainerType *optionalContainerType() const { return nullptr; }
    inline virtual const StructureType *structureType() const { return nullptr; }
    inline virtual const EnumType *enumType() const { return nullptr; }
    inline virtual StructureType *structurePrototype() { return nullptr; }
    inline virtual EnumType *enumPrototype() { return nullptr; }
    inline virtual int compile() { return 0; }

protected:
    inline explicit Type(const TypeName &name) : typeName(name) { }
    inline explicit Type(TypeName &&name) : typeName((TypeName &&) name) { }

private:
    TypeName typeName;

};
