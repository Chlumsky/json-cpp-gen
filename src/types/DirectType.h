
#pragma once

#include "../Type.h"

/// Basic type or structure.
class DirectType : public Type {

public:
    virtual ~DirectType() = default;

protected:
    inline explicit DirectType(const TypeName &name) : Type(name) { }
    inline explicit DirectType(TypeName &&name) : Type((TypeName &&) name) { }

};
