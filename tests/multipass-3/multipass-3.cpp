
#include <cmath>
#include <cstring>
#include "../common.h"
#include "D.h"
#include <multipass-3/Parser.h>
#include <multipass-3/Serializer.h>

using namespace multipass_3;

#define MULTIPASS3

void test_multipass_3() {
    Wrapper o = { }, i = { 75, 331, 1560 };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("multipass-3", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(MULTIPASS3 o == i);
}
