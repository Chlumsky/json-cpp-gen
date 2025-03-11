
#include <cmath>
#include <cstring>
#include "../common.h"
#include "1.h"
#include "2.h"
#include "3.h"
#include "4.h"
#include "5.h"
#include <reverse-order-inputs/Parser.h>
#include <reverse-order-inputs/Serializer.h>

using namespace reverse_order_inputs;

#define REVERSED_INPUTS

void test_reverse_order_inputs() {
    Five o = { }, i = {
        1,
        2,
        3,
        4,
        {
            sqrt(1.),
            sqrt(2.),
            sqrt(3.),
            sqrt(5.),
        },
        5,
        {
            10, 20, 30, 40,
            100, 200, 300,
            1000, 2000,
            10000,
            100000, -1
        }
    };

    std::string json;
    CHECK_RESULT(Serializer::serialize(json, i));
    CHECK_RESULT(Parser::parse(o, json.c_str()));
    CHECK(REVERSED_INPUTS o == i);
}
