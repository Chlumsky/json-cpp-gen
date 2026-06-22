
#pragma once

#include <cstdint>
#include "opaque.h"

namespace type_aliases {

typedef unsigned char byte;

using int32 = std::int32_t;

struct MainStruct {
    byte b;
    int32 i32;
    int16 i16;
    uint16 u16;
    mint m;
};

// Equality operators

inline bool operator==(const MainStruct &a, const MainStruct &b) {
    return (
        a.b == b.b &&
        a.i32 == b.i32 &&
        a.i16 == b.i16 &&
        a.u16 == b.u16 &&
        a.m == b.m
    );
}

}
