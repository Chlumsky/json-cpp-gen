
#include <cstring>
#include "../common.h"
#include "root.h"
#include <nested-type-alias/Parser.h>
#include <nested-type-alias/Serializer.h>

using namespace nested_type_alias;

#define NESTED_ALIAS

void test_nested_type_alias() {
    Root o, i = {
        "#ha",
        -713,
        Lares::AEGISTHUS,
        "#ma",
        -1500000000,
        Mercury::Myrtilus::ORESTES,
        "#aa",
        87500,
        Hermes::Myrtilus::THYESTES
    };
    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(NESTED_ALIAS !memcmp(o.ha.value, i.ha.value, sizeof(Hermes::Aethalides::value)));
    CHECK(NESTED_ALIAS o.hm.value == i.hm.value);
    CHECK(NESTED_ALIAS o.hp.value == i.hp.value);
    CHECK(NESTED_ALIAS !memcmp(o.ma.value, i.ma.value, sizeof(Mercury::Aethalides::value)));
    CHECK(NESTED_ALIAS o.mm.value == i.mm.value);
    CHECK(NESTED_ALIAS o.mp.value == i.mp.value);
    CHECK(NESTED_ALIAS !memcmp(o.aa.value, i.aa.value, sizeof(Anubis::Aethalides::value)));
    CHECK(NESTED_ALIAS o.am.value == i.am.value);
    CHECK(NESTED_ALIAS o.ap.value == i.ap.value);
    CHECK(NESTED_ALIAS !memcmp(&o, &i, sizeof(Root)));
}
