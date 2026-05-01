// SPDX-License-Identifier: MIT

#include "support/test_string_view.h"
#include "support/string_view.h"

START_TEST(test_string_view) {
    StringView view = string_view();
    ck_assert_int_eq(view.length, 0);
    ck_assert_ptr_eq(view.data, NULL);
}
END_TEST

START_TEST(test_string_view_create) {
    StringView view =
        string_view_create("Hello, World", sizeof("Hello, World") - 1);
    ck_assert_int_eq(view.length, sizeof("Hello, World") - 1);
    ck_assert_str_eq(view.data, "Hello, World");
}
END_TEST

START_TEST(test_string_view_from_cstr) {
    StringView view = string_view_from_cstr("Hello, World");
    ck_assert_int_eq(view.length, sizeof("Hello, World") - 1);
    ck_assert_str_eq(view.data, "Hello, World");
}
END_TEST

START_TEST(test_string_view_length) {
    StringView view = SV("Hello");
    ck_assert_int_eq(string_view_length(view), 5);
}
END_TEST

START_TEST(test_string_view_cstr) {
    StringView view = SV("Hello");
    ck_assert_str_eq(string_view_cstr(view), "Hello");
}
END_TEST

START_TEST(test_string_view_cmp) {
    StringView a = SV("Hello");
    StringView b = SV("Hello");
    StringView c = SV("World");

    ck_assert_int_eq(string_view_cmp(a, b), 0);
    ck_assert_int_lt(string_view_cmp(a, c), 0);
    ck_assert_int_gt(string_view_cmp(c, a), 0);
}
END_TEST

START_TEST(test_string_view_cmp_cstr) {
    StringView view = SV("Hello");

    ck_assert_int_eq(string_view_cmp_cstr(view, "Hello"), 0);
    ck_assert_int_lt(string_view_cmp_cstr(view, "World"), 0);
    ck_assert_int_gt(string_view_cmp_cstr(SV("World"), "Hello"), 0);
}
END_TEST

START_TEST(test_string_view_eq) {
    StringView a = SV("Hello");
    StringView b = SV("Hello");
    StringView c = SV("World");

    ck_assert(string_view_eq(a, b));
    ck_assert(!string_view_eq(a, c));
}
END_TEST

START_TEST(test_string_view_eq_cstr) {
    StringView view = SV("Hello");

    ck_assert(string_view_eq_cstr(view, "Hello"));
    ck_assert(!string_view_eq_cstr(view, "World"));
}
END_TEST

Suite *test_string_view_suite(void) {
    Suite *s = suite_create("string_view");

    TCase *tc_core = tcase_create("exp.support.StringView");
    tcase_add_test(tc_core, test_string_view);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.create");
    tcase_add_test(tc_core, test_string_view_create);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.from_cstr");
    tcase_add_test(tc_core, test_string_view_from_cstr);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.length");
    tcase_add_test(tc_core, test_string_view_length);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.cstr");
    tcase_add_test(tc_core, test_string_view_cstr);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.cmp");
    tcase_add_test(tc_core, test_string_view_cmp);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.cmp_cstr");
    tcase_add_test(tc_core, test_string_view_cmp_cstr);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.eq");
    tcase_add_test(tc_core, test_string_view_eq);
    suite_add_tcase(s, tc_core);

    tc_core = tcase_create("exp.support.StringView.eq_cstr");
    tcase_add_test(tc_core, test_string_view_eq_cstr);
    suite_add_tcase(s, tc_core);

    return s;
}

char *const test_string_view_names[] = {
    "exp.support.StringView",
    "exp.support.StringView.create",
    "exp.support.StringView.from_cstr",
    "exp.support.StringView.length",
    "exp.support.StringView.cstr",
    "exp.support.StringView.cmp",
    "exp.support.StringView.cmp_cstr",
    "exp.support.StringView.eq",
    "exp.support.StringView.eq_cstr",
};
size_t const test_string_view_names_count =
    sizeof(test_string_view_names) / sizeof(test_string_view_names[0]);

void test_string_view_list_all(FILE *out) {

    for (size_t i = 0; i < test_string_view_names_count; ++i) {
        fprintf(out, "%s\n", test_string_view_names[i]);
    }
}