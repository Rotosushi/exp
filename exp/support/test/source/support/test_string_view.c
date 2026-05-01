// SPDX-License-Identifier: MIT

#include "support/test_string_view.h"
#include "support/string_view.h"

START_TEST(test_string_view) {
    StringView view = string_view();
    ck_assert_int_eq(view.length, 0);
    ck_assert_ptr_eq(view.data, NULL);
}
END_TEST

Suite *test_string_view_suite(void) {
    Suite *s = suite_create("string_view");

    TCase *tc_core = tcase_create("exp.support.StringView");
    tcase_add_test(tc_core, test_string_view);
    suite_add_tcase(s, tc_core);

    return s;
}

char *const test_string_view_names[] = {
    "test_string_view",
};
size_t const test_string_view_names_count =
    sizeof(test_string_view_names) / sizeof(test_string_view_names[0]);

void test_string_view_list_all(FILE *out) {

    for (size_t i = 0; i < test_string_view_names_count; ++i) {
        fprintf(out, "%s\n", test_string_view_names[i]);
    }
}