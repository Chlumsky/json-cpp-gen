
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
        Eins::Singular::Unique u1;
        Ichi::Singular::Unique u2;
        Jedna::Singular::Unique u3;
        One::Singular::Unique u5;
        AUTO_EQ(Singletons);
    } stons;

    int five;
    PreviousNumbers previous;

    AUTO_EQ(Five);

};

}
