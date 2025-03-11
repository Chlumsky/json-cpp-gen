
#pragma once

namespace reverse_order_inputs {

typedef One Eins;

struct Two : One {
    int two;
    AUTO_EQ(Two);
};

struct Eins::Singular::Unique {
    double singleton;
    AUTO_EQ(Unique);
};

}
