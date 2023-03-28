
#pragma once

#include <string>
#include "../Type.h"

class TypeAlias : public Type {

public:
    explicit TypeAlias(const std::string &name);
    explicit TypeAlias(std::string &&name);
    TypeAlias(const std::string &name, const Type *type);
    TypeAlias(std::string &&name, const Type *type);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    virtual bool isIncomplete() const override;
    virtual const Type *actualType() const override;
    virtual const StringType *stringType() const override;
    virtual const OptionalContainerType *optionalContainerType() const override;
    virtual const StructureType *structureType() const override;
    virtual const EnumType *enumType() const override;
    bool isFinalized() const;
    bool finalize(const Type *type);

private:
    const Type *type;

};
