
#include <cmath>
#include <cstring>
#include "../common.h"
#include "D.h"
#include <multipass-2/Parser.h>
#include <multipass-2/Serializer.h>

using namespace multipass_2;

#define MULTIPASS2

void test_multipass_2() {
    Wrapper o = { }, i = { 17, 31, 52 };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("multipass-2", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(MULTIPASS2 o == i);
}
