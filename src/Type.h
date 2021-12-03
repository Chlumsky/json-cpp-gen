
#pragma once

#include <string>
#include "TypeName.h"

class ParserGenerator;
class SerializerGenerator;

class Type {

public:
    virtual ~Type() = default;
    inline const TypeName & name() const { return typeName; }
    virtual bool isDirectType() const = 0;
    virtual bool isContainerType() const = 0;
    virtual bool isStringType() const = 0;
    virtual bool isBasicType() const = 0;
    virtual bool isStructure() const = 0;
    virtual bool isEnum() const = 0;
    virtual bool isOptionalType() const = 0;
    virtual bool isArrayType() const = 0;
    virtual bool isObjectType() const = 0;
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const = 0;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const = 0;

protected:
    inline explicit Type(const TypeName &name) : typeName(name) { }
    inline explicit Type(TypeName &&name) : typeName((TypeName &&) name) { }

private:
    TypeName typeName;

};
