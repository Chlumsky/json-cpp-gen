
#pragma once

#include "../Type.h"
#include "../Configuration.h"
#include "StringType.h"

class ConstStringType : public Type {

public:
    ConstStringType(const std::string &name, const StringType *stringType, const ConstStringAPI &api);
    ConstStringType(std::string &&name, const StringType *stringType, const ConstStringAPI &api);
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;
    std::string generateCopyFromString(const char *subject, const char *x) const;
    std::string generateMoveFromString(const char *subject, const char *x) const;
    std::string generateIterateChars(const char *subject, const char *iteratorName, const char *endIteratorName, const char *elementName, const char *body) const;

private:
    const StringType *stringType;
    ConstStringAPI api;

};
