// SPDX-License-Identifier: GPL-3.0-or-later

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <check.h>

#include "support/cli_option_parser.h"

#include "support/test_cli_option_parser.h"
#include "support/test_string_view.h"

static CliOption const test_options[] = {
    {
     .name                 = "all",
     .description          = "Run all tests.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'a',
     },
    {
     .name                 = "list_content",
     .description          = "List all available tests.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'l',
     }
};
static const size_t test_options_length =
    sizeof(test_options) / sizeof(test_options[0]);

static SRunner *register_all_tests(void) {
    SRunner *runner = srunner_create(test_cli_option_parser_suite());
    srunner_add_suite(runner, test_string_view_suite());
    return runner;
}

static void list_all_tests(FILE *out) {
    test_cli_option_parser_list_all(out);
    test_string_view_list_all(out);
}

int main(int argc, char const *argv[], char const *envp[]) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, test_options, test_options_length);

    int      number_failed = 0;
    SRunner *runner        = register_all_tests();
    // srunner_set_fork_status(runner, CK_NOFORK);

    while (true) {
        CliOptionResult result =
            cli_option_parser_parse_option(&parser, argc, argv, envp);

        switch (result.option) {
        case CLI_OPTION_END:
            // No more options to parse
            return EXIT_SUCCESS;

        case CLI_OPTION_UNRECOGNIZED:
            fprintf(
                stderr, "Error: Unrecognized option '%s'\n", result.argument);
            break;

        case CLI_OPTION_MISSING_ARGUMENT:
            fprintf(stderr,
                    "Error: Missing required argument for option '%s'\n",
                    result.argument);
            break;

        case CLI_OPTION_POSITIONAL_ARGUMENT:
            srunner_run(runner, NULL, result.argument, CK_NORMAL);
            break;

        case 'a': // --all
            srunner_run_all(runner, CK_NORMAL);
            return EXIT_SUCCESS;

        case 'l': // --list_content
            list_all_tests(stdout);
            return EXIT_SUCCESS;

        default: return EXIT_FAILURE; // Unhandled option, exit with failure
        }
    }

    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? 0 : 1;
}