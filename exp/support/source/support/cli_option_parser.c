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
    parser->option_count = option_count;
    parser->options      = options;
}

static size_t option_name_length(char const *option_name) {
    size_t length = 0;
    while (option_name[length] != '\0' && option_name[length] != '=' &&
           option_name[length] != ':' && option_name[length] != ' ') {
        length++;
    }
    return length;
}

static OptionResult parse_option_argument(CliOptionParser *parser,
                                          CliOption const *option) {
    if (option->argument_kind == OPTION_ARGUMENT_NONE) {
        return (OptionResult){.option = option->short_name, .argument = NULL};
    }

    // Check if the option is in the form --option=value
    char const *argument    = NULL;
    char const *current_arg = parser->options[parser->option_index - 1].name;
    size_t      current_arg_len = strlen(current_arg);
    size_t      option_name_len = strlen(option->name);
    if (current_arg_len > option_name_len &&
        (current_arg[option_name_len] == '=' ||
         current_arg[option_name_len] == ':')) {
        argument = current_arg + option_name_len + 1; // Skip the '=' or ':'
    } else {
        // Otherwise, the argument should be the next argv element
        if (parser->option_index < parser->option_count) {
            argument =
                parser->options[parser->option_index].name; // Next argv element
            parser->option_index++; // Move to the next option for the next call
            return (OptionResult){.option   = option->short_name,
                                  .argument = argument};
        }
        return (OptionResult){.option   = '?',
                              .argument = NULL}; // Missing required argument
    }
}

static OptionResult parse_long_option(CliOptionParser *parser,
                                      char const      *option_name) {
    size_t option_name_len = option_name_length(option_name);
    for (int32_t i = 0; i < parser->option_count; ++i) {
        CliOption const *option = &parser->options[i];
        if (strncmp(option->name, option_name, option_name_len) == 0 &&
            option->name[option_name_len] == '\0') {
            return parse_option_argument(parser, option);
        }
    }
    return (OptionResult){.option   = '?',
                          .argument = option_name}; // Unrecognized option
}

static OptionResult parse_short_option(CliOptionParser *parser,
                                       char             short_name) {
    for (int32_t i = 0; i < parser->option_count; ++i) {
        if (parser->options[i].short_name == short_name) {
            return parse_option_argument(parser, &parser->options[i]);
        }
    }
    return (OptionResult){.option   = '?',
                          .argument = NULL}; // Unrecognized option
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
        return (OptionResult){.option = 0, .argument = NULL}; // No more options
    }

    char const *current_arg = argv[parser->option_index];
    if (current_arg[0] == '-') {
        // Long option (e.g., --help)
        if (current_arg[1] == '-') {
            char const *option_name = current_arg + 2; // Skip the "--"
            parser->option_index++;
            return parse_long_option(parser, option_name);
        }
        // Short option (e.g., -h)
        else {
            char short_name = current_arg[1]; // Get the character after '-'
            parser->option_index++;
            return parse_short_option(parser, short_name);
        }
    }
    // Positional argument
    parser->option_index++;
    return (OptionResult){.option = '!', .argument = current_arg};
}
