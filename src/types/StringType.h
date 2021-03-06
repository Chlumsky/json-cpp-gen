
#pragma once

#include "../Type.h"
#include "../Configuration.h"

class StringType : public Type {

public:
    static const StringType STD_STRING;

    StringType(const std::string &name, const StringAPI &api);
    StringType(std::string &&name, const StringAPI &api);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateClear(const char *subject) const;
    std::string generateAppendChar(const char *subject, const char *x) const;
    std::string generateAppendCStr(const char *subject, const char *x) const;
    std::string generateIterateChars(const char *subject, const char *elementName, const char *body) const;

private:
    StringAPI api;

};
