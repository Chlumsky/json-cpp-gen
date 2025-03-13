
#pragma once

#include <string>
#include <vector>
#include "../common.h"

namespace namespaces {

namespace subspace_A { }

namespace alpha = subspace_A;

namespace subspace_A {

using namespace ::std;

typedef string AString;

struct Sirius {
    enum Star {
        A,
        B
    };

    vector<Star> stars;

    AUTO_EQ(Sirius);
};

}

using alpha::AString;

}
