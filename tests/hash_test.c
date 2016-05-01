#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wtype-limits"

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "hash.h"

START_TEST(test_hash_init_1)
{
    hash_t hash = {0};
    ck_assert_int_eq(hash_init(&hash, 10), 1);
    ck_assert_int_eq(hash.capacity, 10);
    ck_assert_int_eq(hash.count, 0);
    ck_assert(hash.records != NULL);
}
END_TEST

START_TEST(test_hash_init_2)
{
    hash_t* hash = NULL;
    ck_assert_int_eq(hash_init(hash, 10), 0);
}
END_TEST

START_TEST(test_hash_insert_1)
{
    hash_t hash = {0};
    hash_init(&hash, 10);
    int a = 42;
    ck_assert_int_eq(hash_insert(&hash, "foo", &a), 1);
}
END_TEST

START_TEST(test_hash_get_1)
{
    hash_t hash = {0};
    hash_init(&hash, 10);
    int a = 42;
    hash_insert(&hash, "foo", &a);
    int* b = hash_get(&hash, "foo");
    ck_assert_int_eq(a, *b);
}
END_TEST

START_TEST(test_hash_get_2)
{
    hash_t hash = {0};
    hash_init(&hash, 10);
    void* val = hash_get(&hash, "bar");
    ck_assert(val == NULL);
}
END_TEST

START_TEST(test_hash_delete_1)
{
    hash_t hash = {0};
    hash_init(&hash, 10);
    int a = 42;
    ck_assert_int_eq(hash_insert(&hash, "foo", &a), 1);
    ck_assert_int_eq(hash_delete(&hash, "foo"), 1);
}
END_TEST

START_TEST(test_hash_delete_2)
{
    hash_t hash = {0};
    hash_init(&hash, 10);
    ck_assert_int_eq(hash_delete(&hash, "foo"), 0);
}
END_TEST

Suite* hash_suite(void)
{
    Suite *s = suite_create("hash");

    TCase* tc_hash_init = tcase_create("init");
    tcase_add_test(tc_hash_init, test_hash_init_1);
    tcase_add_test(tc_hash_init, test_hash_init_2);
    suite_add_tcase(s, tc_hash_init);

    TCase* tc_hash_insert = tcase_create("insert");
    tcase_add_test(tc_hash_insert, test_hash_insert_1);
    suite_add_tcase(s, tc_hash_insert);

    TCase* tc_hash_get = tcase_create("get");
    tcase_add_test(tc_hash_get, test_hash_get_1);
    tcase_add_test(tc_hash_get, test_hash_get_2);
    suite_add_tcase(s, tc_hash_get);

    TCase* tc_hash_delete = tcase_create("delete");
    tcase_add_test(tc_hash_delete, test_hash_delete_1);
    tcase_add_test(tc_hash_delete, test_hash_delete_2);
    suite_add_tcase(s, tc_hash_delete);

    return s;
}
