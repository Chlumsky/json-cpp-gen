
#pragma once

#include "../Type.h"
#include "../Configuration.h"
#include "StringType.h"

class ConstStringType : public Type {

public:
    ConstStringType(const std::string &name, const StringType *stringType, const ConstStringAPI &api);
    ConstStringType(std::string &&name, const StringType *stringType, const ConstStringAPI &api);
    inline virtual bool isDirectType() const override { return false; }
    inline virtual bool isContainerType() const override { return false; }
    inline virtual bool isStringType() const override { return false; } // TODO
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return false; }
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return false; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateCopyFromString(const char *subject, const char *x) const;
    std::string generateMoveFromString(const char *subject, const char *x) const;
    std::string generateIterateChars(const char *subject, const char *elementName, const char *body) const;

private:
    const StringType *stringType;
    ConstStringAPI api;

};
