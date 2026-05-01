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
    {
     .name                 = "output",
     .description          = "Specify the output file.",
     .environment_variable = "OUTPUT_FILE",
     .argument_kind        = OPTION_ARGUMENT_REQUIRED,
     .short_name           = 'o',
     },
    {
     .name                 = "optimize",
     .description          = "Enable optimization.",
     .environment_variable = "OPTIMIZE",
     .argument_kind        = OPTION_ARGUMENT_OPTIONAL,
     .short_name           = 'O',
     },
    {
     .name                 = "jflag",
     .description          = "An interesting flag.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'j',
     },
    {
     .name                 = "kflag",
     .description          = "An interesting flag.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'k',
     },
    {
     .name                 = "lflag",
     .description          = "An interesting flag.",
     .environment_variable = NULL,
     .argument_kind        = OPTION_ARGUMENT_NONE,
     .short_name           = 'l',
     }
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

START_TEST(test_cli_option_parser_parse_option_long_argument_none) {
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
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_argument_required) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--output=output.txt", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --output=output.txt
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'o');
    ck_assert_str_eq(result.argument, "output.txt");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_argument_required_next) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--output", "output.txt", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --output=output.txt
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'o');
    ck_assert_str_eq(result.argument, "output.txt");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_argument_required_missing) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--output", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;
    // Parse --output (missing required argument)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_MISSING_ARGUMENT);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_argument_optional_present) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--optimize=2", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --optimize=2
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_str_eq(result.argument, "2");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(
    test_cli_option_parser_parse_option_long_argument_optional_present_next) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--optimize", "2", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --optimize 2
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_str_eq(result.argument, "2");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_argument_optional_missing) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--optimize", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --optimize (optional argument missing)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_long_unrecognized) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "--unknown", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse --unknown
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_UNRECOGNIZED);
    ck_assert_str_eq(result.argument, "unknown");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_short_argument_none) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-h", "-v", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -h
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'h');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse -v
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'v');
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_short_argument_none_combined) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-hjkl", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -h
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'h');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse -j
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'j');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse -k
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'k');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse -l
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'l');
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_short_argument_required) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-ooutput.txt", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -ooutput.txt
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'o');
    ck_assert_str_eq(result.argument, "output.txt");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_short_argument_required_next) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-o", "output.txt", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -o output.txt
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'o');
    ck_assert_str_eq(result.argument, "output.txt");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(
    test_cli_option_parser_parse_option_short_argument_required_missing) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-o", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -o (missing required argument)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_MISSING_ARGUMENT);
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(
    test_cli_option_parser_parse_option_short_argument_optional_present) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-O2", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -O2
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_str_eq(result.argument, "2");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(
    test_cli_option_parser_parse_option_short_argument_optional_present_next) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 3; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-O", "2", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -O 2
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_str_eq(result.argument, "2");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(
    test_cli_option_parser_parse_option_short_argument_optional_missing) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-O", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -O (optional argument missing)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'O');
    ck_assert_ptr_eq(result.argument, NULL);

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_short_unrecognized) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "-x", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -x
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_UNRECOGNIZED);
    ck_assert_str_eq(result.argument, "x");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_positional) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 2; // number of arguments (excluding NULL)
    char const *argv[] = {"program", "positional_arg", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse positional_arg (not an option)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_POSITIONAL_ARGUMENT);
    ck_assert_str_eq(result.argument, "positional_arg");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

START_TEST(test_cli_option_parser_parse_option_mixed) {
    CliOptionParser parser;
    cli_option_parser_init(&parser, options, length);

    int32_t     argc   = 4; // number of arguments (excluding NULL)
    char const *argv[] = {
        "program", "-h", "--output=output.txt", "positional_arg", NULL};
    char const *envp[] = {"PATH=/usr/bin", NULL};

    OptionResult result;

    // Parse -h
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'h');
    ck_assert_ptr_eq(result.argument, NULL);

    // Parse --output=output.txt
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, 'o');
    ck_assert_str_eq(result.argument, "output.txt");

    // Parse positional_arg (not an option)
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_POSITIONAL_ARGUMENT);
    ck_assert_str_eq(result.argument, "positional_arg");

    // No more options
    result = cli_option_parser_parse_option(&parser, argc, argv, envp);
    ck_assert_int_eq(result.option, CLI_OPTION_END);
    ck_assert_ptr_eq(result.argument, NULL);
}
END_TEST

Suite *test_cli_option_parser_suite(void) {
    Suite *suite = suite_create("exp.support.CliOptionParser");

    TCase *cli_option_parser_init =
        tcase_create("exp.support.CliOptionParser.init");
    tcase_add_test(cli_option_parser_init, test_cli_option_parser_init);
    suite_add_tcase(suite, cli_option_parser_init);

    TCase *cli_option_parser_parse_option_long_argument_none = tcase_create(
        "exp.support.CliOptionParser.parse_option.long_argument_none");
    tcase_add_test(cli_option_parser_parse_option_long_argument_none,
                   test_cli_option_parser_parse_option_long_argument_none);
    suite_add_tcase(suite, cli_option_parser_parse_option_long_argument_none);

    TCase *cli_option_parser_parse_option_long_argument_required = tcase_create(
        "exp.support.CliOptionParser.parse_option.long_argument_required");
    tcase_add_test(cli_option_parser_parse_option_long_argument_required,
                   test_cli_option_parser_parse_option_long_argument_required);
    suite_add_tcase(suite,
                    cli_option_parser_parse_option_long_argument_required);

    TCase *cli_option_parser_parse_option_long_argument_required_next =
        tcase_create("exp.support.CliOptionParser.parse_option.long_argument_"
                     "required_next");
    tcase_add_test(
        cli_option_parser_parse_option_long_argument_required_next,
        test_cli_option_parser_parse_option_long_argument_required_next);
    suite_add_tcase(suite,
                    cli_option_parser_parse_option_long_argument_required_next);

    TCase *cli_option_parser_parse_option_long_argument_required_missing =
        tcase_create("exp.support.CliOptionParser.parse_option.long_argument_"
                     "required_missing");
    tcase_add_test(
        cli_option_parser_parse_option_long_argument_required_missing,
        test_cli_option_parser_parse_option_long_argument_required_missing);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_long_argument_required_missing);

    TCase *cli_option_parser_parse_option_long_argument_optional_present =
        tcase_create("exp.support.CliOptionParser.parse_option.long_argument_"
                     "optional_present");
    tcase_add_test(
        cli_option_parser_parse_option_long_argument_optional_present,
        test_cli_option_parser_parse_option_long_argument_optional_present);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_long_argument_optional_present);

    TCase *cli_option_parser_parse_option_long_argument_optional_present_next =
        tcase_create("exp.support.CliOptionParser.parse_option.long_argument_"
                     "optional_present_next");
    tcase_add_test(
        cli_option_parser_parse_option_long_argument_optional_present_next,
        test_cli_option_parser_parse_option_long_argument_optional_present_next);
    suite_add_tcase(
        suite,
        cli_option_parser_parse_option_long_argument_optional_present_next);

    TCase *cli_option_parser_parse_option_long_argument_optional_missing =
        tcase_create("exp.support.CliOptionParser.parse_option.long_argument_"
                     "optional_missing");
    tcase_add_test(
        cli_option_parser_parse_option_long_argument_optional_missing,
        test_cli_option_parser_parse_option_long_argument_optional_missing);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_long_argument_optional_missing);

    TCase *cli_option_parser_parse_option_long_unrecognized = tcase_create(
        "exp.support.CliOptionParser.parse_option.long_unrecognized");
    tcase_add_test(cli_option_parser_parse_option_long_unrecognized,
                   test_cli_option_parser_parse_option_long_unrecognized);
    suite_add_tcase(suite, cli_option_parser_parse_option_long_unrecognized);

    TCase *cli_option_parser_parse_option_short_argument_none = tcase_create(
        "exp.support.CliOptionParser.parse_option.short_argument_none");
    tcase_add_test(cli_option_parser_parse_option_short_argument_none,
                   test_cli_option_parser_parse_option_short_argument_none);
    suite_add_tcase(suite, cli_option_parser_parse_option_short_argument_none);

    TCase *cli_option_parser_parse_option_short_argument_none_combined =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "none_combined");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_none_combined,
        test_cli_option_parser_parse_option_short_argument_none_combined);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_short_argument_none_combined);

    TCase *cli_option_parser_parse_option_short_argument_required =
        tcase_create(
            "exp.support.CliOptionParser.parse_option.short_argument_required");
    tcase_add_test(cli_option_parser_parse_option_short_argument_required,
                   test_cli_option_parser_parse_option_short_argument_required);
    suite_add_tcase(suite,
                    cli_option_parser_parse_option_short_argument_required);

    TCase *cli_option_parser_parse_option_short_argument_required_next =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "required_next");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_required_next,
        test_cli_option_parser_parse_option_short_argument_required_next);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_short_argument_required_next);

    TCase *cli_option_parser_parse_option_short_argument_required_missing =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "required_missing");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_required_missing,
        test_cli_option_parser_parse_option_short_argument_required_missing);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_short_argument_required_missing);

    TCase *cli_option_parser_parse_option_short_argument_optional_present =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "optional_present");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_optional_present,
        test_cli_option_parser_parse_option_short_argument_optional_present);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_short_argument_optional_present);

    TCase *cli_option_parser_parse_option_short_argument_optional_present_next =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "optional_present_next");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_optional_present_next,
        test_cli_option_parser_parse_option_short_argument_optional_present_next);
    suite_add_tcase(
        suite,
        cli_option_parser_parse_option_short_argument_optional_present_next);

    TCase *cli_option_parser_parse_option_short_argument_optional_missing =
        tcase_create("exp.support.CliOptionParser.parse_option.short_argument_"
                     "optional_missing");
    tcase_add_test(
        cli_option_parser_parse_option_short_argument_optional_missing,
        test_cli_option_parser_parse_option_short_argument_optional_missing);
    suite_add_tcase(
        suite, cli_option_parser_parse_option_short_argument_optional_missing);

    TCase *cli_option_parser_parse_option_short_unrecognized = tcase_create(
        "exp.support.CliOptionParser.parse_option.short_unrecognized");
    tcase_add_test(cli_option_parser_parse_option_short_unrecognized,
                   test_cli_option_parser_parse_option_short_unrecognized);
    suite_add_tcase(suite, cli_option_parser_parse_option_short_unrecognized);

    TCase *cli_option_parser_parse_option_positional =
        tcase_create("exp.support.CliOptionParser.parse_option.positional");
    tcase_add_test(cli_option_parser_parse_option_positional,
                   test_cli_option_parser_parse_option_positional);
    suite_add_tcase(suite, cli_option_parser_parse_option_positional);

    TCase *cli_option_parser_parse_option_mixed =
        tcase_create("exp.support.CliOptionParser.parse_option.mixed");
    tcase_add_test(cli_option_parser_parse_option_mixed,
                   test_cli_option_parser_parse_option_mixed);
    suite_add_tcase(suite, cli_option_parser_parse_option_mixed);

    return suite;
}

static char *test_cli_option_parser_names[] = {
    "exp.support.CliOptionParser.init",
    "exp.support.CliOptionParser.parse_option.long_argument_none",
    "exp.support.CliOptionParser.parse_option.long_argument_required",
    "exp.support.CliOptionParser.parse_option.long_argument_required_next",
    "exp.support.CliOptionParser.parse_option.long_argument_required_missing",
    "exp.support.CliOptionParser.parse_option.long_argument_optional_present",
    "exp.support.CliOptionParser.parse_option.long_argument_optional_present_"
    "next",
    "exp.support.CliOptionParser.parse_option.long_argument_optional_missing",
    "exp.support.CliOptionParser.parse_option.long_unrecognized",
    "exp.support.CliOptionParser.parse_option.short_argument_none",
    "exp.support.CliOptionParser.parse_option.short_argument_required",
    "exp.support.CliOptionParser.parse_option.short_argument_required_next",
    "exp.support.CliOptionParser.parse_option.short_argument_required_missing",
    "exp.support.CliOptionParser.parse_option.short_argument_optional_present",
    "exp.support.CliOptionParser.parse_option.short_argument_optional_present_"
    "next",
    "exp.support.CliOptionParser.parse_option.short_argument_optional_missing",
    "exp.support.CliOptionParser.parse_option.short_unrecognized",
    "exp.support.CliOptionParser.parse_option.positional",
    "exp.support.CliOptionParser.parse_option.mixed",
};

static int test_cli_option_parser_count =
    sizeof(test_cli_option_parser_names) /
    sizeof(test_cli_option_parser_names[0]);

void test_cli_option_parser_list_all(FILE *out) {
    for (int i = 0; i < test_cli_option_parser_count; ++i) {
        fprintf(out, "%s\n", test_cli_option_parser_names[i]);
    }
}