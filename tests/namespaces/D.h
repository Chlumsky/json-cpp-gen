
#pragma once

#include "C.h"

namespace namespaces {

namespace b_sub2 = sub2;

struct Wrapper {
    AString name;
    alpha::Sirius sirius;
    Constellation orion;
    b_sub2::sub3::Integer total;

    AUTO_EQ(Wrapper);
};

}
