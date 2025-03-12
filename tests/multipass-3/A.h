
#pragma once

#include "../common.h"

namespace multipass_3 {

struct ABase {
    struct B { int b; };
    struct C {
        int c;
        AUTO_EQ(C);
    };
    struct Q {
        using C = ABase::C;
        struct D {
            int d;
            AUTO_EQ(D);
        };
        int q;
        AUTO_EQ(Q);
    };
};

}
