
#include "common.h"

int testFailures = 0;

void test_basic_case();

int main() {
    test_basic_case();
    if (testFailures)
        fprintf(stderr, "TESTS FAILED %d TIMES!\n", testFailures);
    else
        fprintf(stderr, "All tests successful\n");
    return testFailures;
}
