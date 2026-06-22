
#include "../common.h"
#include "structures.h"
#include <type-aliases/Parser.h>
#include <type-aliases/Serializer.h>

using namespace type_aliases;

void test_type_aliases() {
    MainStruct i = { }, o = { };
    i.b = 0xec;
    i.i32 = -98765;
    i.i16 = -23456;
    i.u16 = 65432;
    i.m = -987654321;

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    DUMP_JSON("type-aliases", json);
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(o == i);
}
