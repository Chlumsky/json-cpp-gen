
#pragma once

namespace reverse_order_inputs {

typedef Uno Jedna;
using Duo = Zwei;
typedef Three Drei;

struct Four : Three {
    int four;
    AUTO_EQ(Four);
};

struct MinusOne : Eins {
    signed char sign;
    AUTO_EQ(MinusOne);
};

}
