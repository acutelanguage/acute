#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wtype-limits"

#include <stdlib.h>
#include <check.h>
#include "refuse.h"
#include "deque.h"

struct mock
{
    int a;
};

START_TEST(test_refuse_init)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    ck_assert(refuse.mod != NULL);
    ck_assert(refuse.dec != NULL);
}
END_TEST

START_TEST(test_refuse_alloc_1)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    struct mock* t = refuse_alloc(&refuse, sizeof(struct mock));
    ck_assert(t != NULL);
    refhdr_t* hdr = (refhdr_t*)((char*)t - sizeof(refhdr_t));
    ck_assert_int_eq(hdr->retainCount, 0);
    ck_assert_int_eq(hdr->new, 1);
    ck_assert_int_eq(hdr->dirty, 0);
}
END_TEST

START_TEST(test_refuse_destroy)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    ck_assert(refuse.dec != NULL);
    ck_assert(refuse.mod != NULL);
    refuse_destroy(&refuse);
    ck_assert(refuse.dec == NULL);
    ck_assert(refuse.mod == NULL);
}
END_TEST

START_TEST(test_refuse_retain_1)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    struct mock* t = refuse_alloc(&refuse, sizeof(struct mock));
    t->a = 42;
    refuse_retain(&refuse, t);
    refhdr_t* hdr = (refhdr_t*)((char*)t - sizeof(refhdr_t));
    ck_assert_int_eq(hdr->retainCount, 0);
    ck_assert_int_eq(hdr->new, 1);
    ck_assert_int_eq(hdr->dirty, 0);
    refhdr_t* popped = refdeque_pop_back(refuse.mod);
    struct mock* s = (struct mock*)((char*)popped + sizeof(refhdr_t));
    ck_assert_int_eq(s->a, 42);
}
END_TEST

START_TEST(test_refuse_retain_2)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    struct mock* t = refuse_alloc(&refuse, sizeof(struct mock));
    refuse_retain(&refuse, t);
    refhdr_t* hdr = (refhdr_t*)((char*)t - sizeof(refhdr_t));
    ck_assert_int_eq(hdr->retainCount, 0);
    ck_assert_int_eq(hdr->new, 1);
    ck_assert_int_eq(hdr->dirty, 0);
    refuse_reconcile(&refuse);
    ck_assert_int_eq(hdr->new, 0);
    ck_assert_int_eq(hdr->retainCount, 1);
}
END_TEST

START_TEST(test_refuse_retain_3)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    struct mock* t = refuse_alloc(&refuse, sizeof(struct mock));
    refhdr_t* hdr = (refhdr_t*)((char*)t - sizeof(refhdr_t));
    refuse_retain(&refuse, t);
    refuse_reconcile(&refuse);
    refuse_set_dirty(&refuse, t);
    ck_assert_int_eq(hdr->dirty, 1);
    ck_assert_int_eq(hdr->retainCount, 1);
    refuse_reconcile(&refuse);
    ck_assert_int_eq(hdr->retainCount, 1);
}
END_TEST

START_TEST(test_refuse_release_1)
{
    refuse_t refuse = {0};
    refuse_init(&refuse);
    struct mock* t = refuse_alloc(&refuse, sizeof(struct mock));
    refuse_retain(&refuse, t);
    refuse_reconcile(&refuse);
    refhdr_t* hdr = (refhdr_t*)((char*)t - sizeof(refhdr_t));
    ck_assert_int_eq(hdr->retainCount, 1);
    refuse_release(&refuse, t);
    ck_assert_int_eq(hdr->retainCount, 1);
    refuse_reconcile(&refuse);
    // This is not safe, as the memory has been deallocated, but we're
    // in a controlled environment.
    ck_assert_int_eq(hdr->retainCount, 0);
}
END_TEST

Suite* refuse_suite(void)
{
    Suite *s = suite_create("refuse");

    TCase* tc_refuse_init = tcase_create("refuse_init");
    tcase_add_test(tc_refuse_init, test_refuse_init);
    suite_add_tcase(s, tc_refuse_init);
    
    TCase* tc_refuse_destroy = tcase_create("refuse_destroy");
    tcase_add_test(tc_refuse_destroy, test_refuse_destroy);
    suite_add_tcase(s, tc_refuse_destroy);
    
    TCase* tc_refuse_alloc = tcase_create("refuse_alloc");
    tcase_add_test(tc_refuse_alloc, test_refuse_alloc_1);
    suite_add_tcase(s, tc_refuse_alloc);
    
    TCase* tc_refuse_retain = tcase_create("refuse_retain");
    tcase_add_test(tc_refuse_retain, test_refuse_retain_1);
    tcase_add_test(tc_refuse_retain, test_refuse_retain_2);
    tcase_add_test(tc_refuse_retain, test_refuse_retain_3);
    suite_add_tcase(s, tc_refuse_retain);
    
    TCase* tc_refuse_release = tcase_create("refuse_release");
    tcase_add_test(tc_refuse_release, test_refuse_release_1);
    suite_add_tcase(s, tc_refuse_release);

    return s;
}

