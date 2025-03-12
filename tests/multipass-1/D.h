
#pragma once

#include "C.h"

namespace multipass_1 {

typedef C D;

struct Wrapper {
    C c;

    AUTO_EQ(Wrapper);
};

}
