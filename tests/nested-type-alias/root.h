
#pragma once

#include "components.h"

namespace nested_type_alias {

typedef Hermes::Myrtilus Lares;

enum Lares::Pelops : long int {
    ATREUS,
    THYESTES,
    AEGISTHUS,
    ORESTES
};

struct Mercury::Palaestra {
    Lares::Pelops value;
};

struct Root {
    Hermes::Aethalides ha;
    Hermes::Myrtilus hm;
    Hermes::Palaestra hp;
    Mercury::Aethalides ma;
    Mercury::Myrtilus mm;
    Mercury::Palaestra mp;
    Anubis::Aethalides aa;
    Anubis::Myrtilus am;
    Anubis::Palaestra ap;
};

}
