
#include "common.h"

int testFailures = 0;

void test_basic_case();
void test_nested_type_alias();

int main() {
    test_basic_case();
    test_nested_type_alias();
    if (testFailures)
        fprintf(stderr, "TESTS FAILED %d TIMES!\n", testFailures);
    else
        fprintf(stderr, "All tests successful\n");
    return testFailures;
}
