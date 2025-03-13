
#pragma once

#include "B.h"

namespace namespaces {

namespace subspace_B {

namespace sub2::sub3 {

using Integer = int;

}

}

namespace beta = subspace_B;

using namespace beta;

struct Constellation {
    subsubspace::Orion orion;
    AUTO_EQ(Constellation);
};

}
