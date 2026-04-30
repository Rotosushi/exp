// SPDX-License-Identifier: GPL-3.0-or-later

#include <check.h>

#include "support/test_cli_option_parser.h"

SRunner *register_all_tests(void) {
    SRunner *runner = srunner_create(test_cli_option_parser_suite());
    return runner;
}

int main(int argc, char *argv[]) {
    int      number_failed = 0;
    SRunner *runner        = register_all_tests();
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? 0 : 1;
}