
#pragma once

#include "../Type.h"

/// Basic type or structure.
class DirectType : public Type {

public:
    virtual ~DirectType() = default;
    inline virtual bool isDirectType() const override { return true; }
    inline virtual bool isContainerType() const override { return false; }
    inline virtual bool isStringType() const override { return false; }
    inline virtual bool isOptionalType() const override { return false; }
    inline virtual bool isArrayType() const override { return false; }
    inline virtual bool isObjectType() const override { return false; }

protected:
    inline explicit DirectType(const TypeName &name) : Type(name) { }
    inline explicit DirectType(TypeName &&name) : Type((TypeName &&) name) { }

};
