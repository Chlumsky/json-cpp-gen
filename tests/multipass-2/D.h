
#pragma once

#include "C.h"

namespace multipass_2 {

struct D : C { };

struct C::Alpha {
    int alpha;
    AUTO_EQ(Alpha);
};

struct B::Beta {
    Alpha alpha;
    AUTO_EQ(Beta);
};

struct A::Gamma : Alpha {
    Beta beta;
    AUTO_EQ(Gamma);
};

struct Wrapper : D::Alpha {
    D::Gamma gamma;
    AUTO_EQ(Wrapper);
};

}
