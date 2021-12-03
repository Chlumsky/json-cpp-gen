
#pragma once

#include "../Type.h"
#include "../Configuration.h"

class StringType : public Type {

public:
    static const StringType STD_STRING;

    StringType(const std::string &name, const StringAPI &api);
    StringType(std::string &&name, const StringAPI &api);
    inline virtual bool isDirectType() const override { return false; }
    inline virtual bool isContainerType() const override { return false; }
    inline virtual bool isStringType() const override { return true; }
    inline virtual bool isBasicType() const override { return false; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return false; }
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return false; }
    inline virtual bool isObjectType() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateAppendChar(const char *subject, const char *x) const;
    std::string generateAppendCStr(const char *subject, const char *x) const;
    std::string generateIterateChars(const char *subject, const char *elementName, const char *body) const;

private:
    StringAPI api;

};
