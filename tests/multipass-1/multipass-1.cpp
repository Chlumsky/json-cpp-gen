
#include <cmath>
#include <cstring>
#include "../common.h"
#include "D.h"
#include <multipass-1/Parser.h>
#include <multipass-1/Serializer.h>

using namespace multipass_1;

#define MULTIPASS1

void test_multipass_1() {
    Wrapper o = { }, i = { -841 };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("multipass-1", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));

    i.c.value = 520;
    CHECK_RESULT(Serializer::serialize(json, i.c));
    DUMP_JSON("multipass-1", json);
    CHECK_RESULT(Parser::parse(o.c, json.c_str()));
    CHECK(MULTIPASS1 o.c == i.c);
}
