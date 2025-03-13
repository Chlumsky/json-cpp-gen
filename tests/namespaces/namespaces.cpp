
#include <cmath>
#include <cstring>
#include "../common.h"
#include "D.h"
#include <namespaces/Parser.h>
#include <namespaces/Serializer.h>

using namespace namespaces;

#define NAMESPACES

void test_namespaces() {
    Wrapper o = { }, i = { "WRAPPER", { { subspace_A::Sirius::B, alpha::Sirius::A } }, "ORION", 711 };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("namespaces", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(NAMESPACES o == i);
}
