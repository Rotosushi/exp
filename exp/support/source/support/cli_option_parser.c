// SPDX-License-Identifier: GPL-3.0-or-later

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "support/cli_option_parser.h"

void cli_option_parser_init(CliOptionParser *parser,
                            CliOption const *options,
                            int32_t          option_count) {
    assert(parser != NULL);
    assert(options != NULL);
    assert(option_count >= 0);
    parser->option_index = 1; // Start from 1 to skip the program name (argv[0])
    parser->suboption_index =
        0; // Start from 0 for parsing suboptions in short options (e.g., -abc)
    parser->option_count = option_count;
    parser->options      = options;
}

static OptionResult
parse_result(CliOptionParser *parser, int32_t option, char const *argument) {
    parser->option_index++; // Move to the next option for the next call
    return (OptionResult){.option = option, .argument = argument};
}

static OptionResult
parse_subresult(CliOptionParser *parser, int32_t option, char const *argument) {
    parser->suboption_index++; // Move to the next suboption for the next call
    return (OptionResult){.option = option, .argument = argument};
}

static size_t option_text_length(char const *option_text) {
    size_t length = 0;
    while (option_text[length] != '\0' && option_text[length] != '=' &&
           option_text[length] != ':' && option_text[length] != ' ') {
        length++;
    }
    return length;
}

static OptionResult parse_long_option_argument(CliOptionParser *parser,
                                               CliOption const *option,
                                               char const      *option_text,
                                               int32_t          argc,
                                               char const      *argv[]) {
    if (option->argument_kind == OPTION_ARGUMENT_NONE) {
        return parse_result(parser, option->short_name, NULL);
    }

    // Check if the option is in the form --option=value
    char const *argument        = NULL;
    size_t      current_arg_len = strlen(option_text);
    size_t      option_name_len = strlen(option->name);
    if (current_arg_len > option_name_len &&
        (option_text[option_name_len] == '=' ||
         option_text[option_name_len] == ':')) {
        argument = option_text + option_name_len + 1; // Skip the '=' or ':'
    }

    if (option->argument_kind == OPTION_ARGUMENT_OPTIONAL) {
        if (argument != NULL) {
            // Optional argument provided in the same argv element (e.g.,
            // --opt=value)
            return parse_result(parser, option->short_name, argument);
        }

        if (parser->option_index + 1 < argc) {
            char const *next_arg = argv[parser->option_index + 1];
            if (next_arg[0] != '-') {
                // Optional argument provided as the next argv element (e.g.,
                // --opt value)
                parser->option_index++; // Consume the argument
                return parse_result(parser, option->short_name, next_arg);
            }
        }
        // Optional argument not provided, return the option with NULL argument
        return parse_result(parser, option->short_name, NULL);
    }

    if (argument != NULL) {
        // Required argument provided in the same argv element (e.g.,
        // --output=file.txt)
        return parse_result(parser, option->short_name, argument);
    }

    // Otherwise, the argument should be the next argv element
    if (parser->option_index + 1 < argc) {
        argument = argv[parser->option_index + 1]; // Next argv element
        if (argument[0] == '-') {
            // Next argv element is another option, but we expected an argument
            return parse_result(parser,
                                CLI_OPTION_MISSING_ARGUMENT,
                                option->name); // Missing required argument
        }

        parser->option_index++; // Consume the argument
        return parse_result(parser, option->short_name, argument);
    }

    parser->option_index++; // Move to the next option for the next call
    return parse_result(parser,
                        CLI_OPTION_MISSING_ARGUMENT,
                        option->name); // Missing required argument
}

static OptionResult parse_long_option(CliOptionParser *parser,
                                      char const      *option_text,
                                      int32_t          argc,
                                      char const      *argv[]) {
    size_t option_name_len = option_text_length(option_text);
    for (int32_t i = 0; i < parser->option_count; ++i) {
        CliOption const *option = &parser->options[i];
        if (strncmp(option->name, option_text, option_name_len) == 0 &&
            option->name[option_name_len] == '\0') {
            return parse_long_option_argument(
                parser, option, option_text, argc, argv);
        }
    }

    return parse_result(
        parser, CLI_OPTION_UNRECOGNIZED, option_text); // Unrecognized option
}

static OptionResult parse_short_option_argument(CliOptionParser *parser,
                                                CliOption const *option,
                                                char const      *option_text,
                                                int32_t          argc,
                                                char const      *argv[]) {
    /// if the short option requires an argument it cannot be combined with
    /// other
    // options (e.g., -abc is not valid if -a requires an argument)
    // if the short option optionally takes an argument, how could we tell if
    // the next character is an argument or another option? (e.g., -O2 could be
    // -O with argument 2, or -O followed by -2)
    // So, for short options, they can only be combined if they do not take an
    // argument.
    if (option->argument_kind == OPTION_ARGUMENT_NONE) {
        if (option_text[parser->suboption_index + 1] != '\0') {
            // More short options combined (e.g., -abc)
            return parse_subresult(parser, option->short_name, NULL);
        }
        // no more suboptions, move to the next argv element for the next call
        return parse_result(parser, option->short_name, NULL);
    }

    // Short options that take arguments cannot be combined with other options
    // (e.g., -O2 is -O with an argument = 2)
    // For short options, the argument can either be provided in the same
    // argv element (e.g., -O2) or as the next argv element (e.g., -O 2).

    if (option->argument_kind == OPTION_ARGUMENT_OPTIONAL) {
        if (option_text[parser->suboption_index + 1] != '\0') {
            // Optional argument provided in the same argv element (e.g., -O2)
            char const *argument = option_text + parser->suboption_index + 1;
            return parse_result(parser, option->short_name, argument);
        }

        if (parser->option_index + 1 < argc) {
            char const *next_arg = argv[parser->option_index + 1];
            if (next_arg[0] != '-') {
                // Optional argument provided as the next argv element (e.g.,
                // -O 2)
                parser->option_index++; // Consume the argument
                return parse_result(parser, option->short_name, next_arg);
            }
        }
        // Optional argument not provided, return the option with NULL argument
        return parse_result(parser, option->short_name, NULL);
    }

    // option->argument_kind == OPTION_ARGUMENT_REQUIRED
    if (option_text[parser->suboption_index + 1] != '\0') {
        // Argument provided in the same argv element (e.g., -O2)
        char const *argument = option_text + parser->suboption_index + 1;
        return parse_result(parser, option->short_name, argument);
    }

    // Argument should be the next argv element
    if (parser->option_index + 1 < argc) {
        char const *argument =
            argv[parser->option_index + 1]; // Next argv element
        parser->option_index++;             // Consume the argument
        return parse_result(parser, option->short_name, argument);
    }

    parser->option_index++; // Move to the next option for the next call
    return parse_result(parser,
                        CLI_OPTION_MISSING_ARGUMENT,
                        option->name); // Missing required argument
}

static OptionResult parse_short_option(CliOptionParser *parser,
                                       char const      *option_text,
                                       int32_t          argc,
                                       char const      *argv[]) {
    for (int32_t i = 0; i < parser->option_count; ++i) {
        CliOption const *option = &parser->options[i];
        if (option->short_name == option_text[parser->suboption_index]) {
            return parse_short_option_argument(
                parser, option, option_text, argc, argv);
        }
    }

    return parse_result(
        parser, CLI_OPTION_UNRECOGNIZED, option_text); // Unrecognized option
}

OptionResult cli_option_parser_parse_option(CliOptionParser *parser,
                                            int32_t          argc,
                                            char const      *argv[],
                                            char const      *envp[]) {
    assert(parser != NULL);
    assert(argc >= 0);
    assert(argv != NULL);
    assert(envp == NULL ||
           envp[0] !=
               NULL); // envp may be NULL, but if not, it must be a valid array

    if (parser->option_index >= argc) {
        return parse_result(parser, CLI_OPTION_END, NULL); // No more options
    }

    char const *current_arg = argv[parser->option_index];
    if (current_arg[0] == '-') {
        // Long option (e.g., --help)
        if (current_arg[1] == '-') {
            char const *option_text = current_arg + 2; // Skip the "--"
            return parse_long_option(parser, option_text, argc, argv);
        }
        // Short option (e.g., -h)
        char const *option_text = current_arg + 1; // Skip the "-"
        return parse_short_option(parser, option_text, argc, argv);
    }

    // Positional argument
    return parse_result(parser, CLI_OPTION_POSITIONAL_ARGUMENT, current_arg);
}
