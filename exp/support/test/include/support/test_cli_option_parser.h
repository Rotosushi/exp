// SPDX-License-Identifier: MIT

#ifndef EXP_SUPPORT_TEST_TEST_CLI_OPTION_PARSER_H
#define EXP_SUPPORT_TEST_TEST_CLI_OPTION_PARSER_H

#include <check.h>
#include <stdio.h>

Suite *test_cli_option_parser_suite(void);
void   test_cli_option_parser_list_all(FILE *out);

#endif // !EXP_SUPPORT_TEST_TEST_CLI_OPTION_PARSER_H