
#include "common.h"

int testFailures = 0;

void test_basic_case();
void test_nested_type_alias();
void test_reverse_order_inputs();

int main() {
    test_basic_case();
    test_nested_type_alias();
    test_reverse_order_inputs();
    if (testFailures)
        fprintf(stderr, "TESTS FAILED %d TIMES!\n", testFailures);
    else
        fprintf(stderr, "All tests successful\n");
    return testFailures;
}
