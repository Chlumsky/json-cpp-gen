
#pragma once

namespace nested_type_alias {

struct Hermes {
    struct Aethalides {
        char value[4];
    };
    struct Myrtilus;
    struct Palaestra;
};

typedef Hermes Mercury;
using Anubis = Mercury;

struct Anubis::Myrtilus {
    enum Pelops : long int;
    int value;
};

}
