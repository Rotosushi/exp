// SPDX-License-Identifier: GPL-3.0-or-later

#include "support/test_cli_option_parser.h"
#include "support/cli_option_parser.h"

static CliOption const options[] = {
    {
     .name                 = "help",
     .description          = "Show this help message and exit.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'h',
     },
    {
     .name                 = "version",
     .description          = "Show version information and exit.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'v',
     },
};

const size_t length = sizeof(options) / sizeof(options[0]);

START_TEST(test_cli_option_parser_init) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);
    ck_assert_int_eq(parser.option_index, 1);
    ck_assert_int_eq(parser.option_count, length);
    ck_assert_ptr_eq(parser.options, options);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_valid) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--help", "--version", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --help
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'h');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse --version
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'v');
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 0);
    ck_assert_ptr_eq(result.argument, NULL);
}

Suite *test_cli_option_parser_suite(void) {
    Suite *suite = suite_create("exp.support.CliOptionParser");

    TCase *cli_option_parser_init =
        tcase_create("exp.support.CliOptionParser.init");
    tcase_add_test(cli_option_parser_init, test_cli_option_parser_init);
    suite_add_tcase(suite, cli_option_parser_init);

    TCase *cli_option_parser_parse_option_valid =
        tcase_create("exp.support.CliOptionParser.parse_option_valid");
    tcase_add_test(cli_option_parser_parse_option_valid,
                   test_cli_option_parser_parse_option_valid);
    suite_add_tcase(suite, cli_option_parser_parse_option_valid);

    return suite;
}