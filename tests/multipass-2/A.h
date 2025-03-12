
#pragma once

#include "../common.h"

namespace multipass_2 {

#define IGNORE_THIS_DEFINITION /* Comment edge case:

struct A {
    std::string fakeDefinitionOfA;
    This may look like a struct definition to json-cpp-gen but is in fact still a comment
};

#define END_OF_COMMENT */

// However, the following struct A is outside comment /*

struct A {
    struct Alpha;
    struct Beta;
    struct Gamma;
};

// */

}
