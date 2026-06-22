
#include "common.h"

int testFailures = 0;

void test_basic_case();
void test_nested_type_alias();
void test_reverse_order_inputs();
void test_multipass_1();
void test_multipass_2();
void test_multipass_3();
void test_namespaces();
void test_type_aliases();

int main() {
    test_basic_case();
    test_nested_type_alias();
    test_reverse_order_inputs();
    test_multipass_1();
    test_multipass_2();
    test_multipass_3();
    test_namespaces();
    test_type_aliases();
    if (testFailures)
        fprintf(stderr, "\nTESTS FAILED %d TIMES!\n", testFailures);
    else
        fprintf(stderr, "\nAll tests successful\n");
    return testFailures;
}
