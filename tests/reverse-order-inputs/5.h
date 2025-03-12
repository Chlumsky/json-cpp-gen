
#pragma once

namespace reverse_order_inputs {

using Ichi = Jedna;
typedef Duo Dva;
using Tri = Drei;
typedef Four Vier;

struct Five : Vier {
    struct PreviousNumbers {
        Vier prev1;
        Tri prev2;
        Dva prev3;
        Ichi prev4;
        MinusOne prev6;
        AUTO_EQ(PreviousNumbers);
    };
    struct Singletons {
        Jedna::Singular::Unique u1;
        Duo::Singular::Unique u2;
        Three::Singular::Unique u3;
        Singular::Unique u5;
        AUTO_EQ(Singletons);
    } stons;

    int five;
    PreviousNumbers previous;

    AUTO_EQ(Five);

};

}
