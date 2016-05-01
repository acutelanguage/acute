#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wtype-limits"

#include <stdlib.h>
#include <check.h>

Suite* hash_suite(void);
Suite* refuse_suite(void);

void run_suite(Suite* s)
{
    int number_failed;
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    if(number_failed != 0) {
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    run_suite(hash_suite());
    run_suite(refuse_suite());
}
