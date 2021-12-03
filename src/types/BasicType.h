
#pragma once

#include "DirectType.h"

class BasicType : public DirectType {

public:
    enum Type {
        VOID,
        BOOL,
        CHAR,
        SIGNED_CHAR,
        UNSIGNED_CHAR,
        SHORT,
        UNSIGNED_SHORT,
        INT,
        UNSIGNED_INT,
        LONG,
        UNSIGNED_LONG,
        LONG_LONG,
        UNSIGNED_LONG_LONG,
        FLOAT,
        DOUBLE,
        LONG_DOUBLE,
        SIZE_T,
        PTRDIFF_T,
        INTPTR_T,
        UINTPTR_T,
        WCHAR_T,
        CHAR8_T,
        CHAR16_T,
        CHAR32_T,
        INT8_T,
        UINT8_T,
        INT16_T,
        UINT16_T,
        INT32_T,
        UINT32_T,
        INT64_T,
        UINT64_T
    };

    static const char * getTypeName(Type type);

    BasicType(Type type = VOID);
    inline virtual bool isBasicType() const override { return true; }
    inline virtual bool isStructure() const override { return false; }
    inline virtual bool isEnum() const override { return false; }
    virtual std::string generateParserFunctionBody(ParserGenerator *generator, const std::string &indent) const override;
    virtual std::string generateSerializerFunctionBody(SerializerGenerator *generator, const std::string &indent) const override;

private:
    Type type;

};
