
#pragma once

namespace reverse_order_inputs {

using Uno = Eins;
typedef Two Zwei;

struct Three : Zwei {
    struct Triangle;

    int three;
    AUTO_EQ(Three);
};

}
