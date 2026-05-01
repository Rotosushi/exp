// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXP_SUPPORT_CLI_OPTION_PARSER_H
#define EXP_SUPPORT_CLI_OPTION_PARSER_H

#include <stdint.h>

#include "support/string_view.h"

/**
 * @brief Denotes the kind of argument an option takes.
 *
 * - OPTION_ARGUMENT_NONE: The option does not take an argument.
 * - OPTION_ARGUMENT_REQUIRED: The option requires an argument.
 * - OPTION_ARGUMENT_OPTIONAL: The option may take an argument, but it's not
 *                             required.
 */
typedef enum CliOptionArgumentKind {
    OPTION_ARGUMENT_NONE,
    OPTION_ARGUMENT_REQUIRED,
    OPTION_ARGUMENT_OPTIONAL,
} CliOptionArgumentKind;

/**
 * @brief Represents a command-line option.
 *
 * @var name: The long name of the option (e.g., "help" for --help).
 * @var description: A brief description of the option for help messages.
 * @var environment_variable: The name of the environment variable that can be
 * used to set the option's value.
 * @var argument_kind: The kind of argument the option takes (none, required,
 *                    optional).
 * @var short_name: The short name of the option (e.g., 'h' for --help).
 */
typedef struct CliOption {
    char const           *name;
    char const           *description;
    char const           *environment_variable;
    CliOptionArgumentKind argument_kind;
    char                  short_name;
} CliOption;

/**
 * @brief Represents a command-line option parser.
 *
 * @var option_index The index of the next option to parse in argv.
 * @var option_count The total number of options in the options array.
 * @var suboption_index The index of the next suboption to parse for the current
 * option (used for parsing single-character options that can be combined, e.g.,
 * -abc).
 * @var options An array of CliOption structures representing the available
 *              options that the parser can recognize.
 */
typedef struct CliOptionParser {
    int32_t          option_index;
    int32_t          suboption_index;
    int32_t          option_count;
    CliOption const *options;
} CliOptionParser;

#define CLI_OPTION_END                 0
#define CLI_OPTION_MISSING_ARGUMENT    -1
#define CLI_OPTION_UNRECOGNIZED        '?'
#define CLI_OPTION_POSITIONAL_ARGUMENT '!'

/**
 * @brief Represents the result of parsing the next available command-line
 * option.
 *
 * @var option The short name of the parsed option (e.g., 'h' for --help),
 *  0 if no more options are available. ? if an unrecognized option was
 *  encountered. and ! if a positional argument was encountered.
 *  -1 if the option was recognized but a required argument was missing.
 *
 * @var argument The argument provided for the option, or NULL if no argument
 *               was provided or if the option does not take an argument. If
 *               the option was not recognized, this will contain the
 *               unrecognized option.
 */
typedef struct OptionResult {
    int32_t     option;
    char const *argument;
} OptionResult;

/**
 * @brief Initializes a CliOptionParser with the given options and option
 * count.
 * @param parser The CliOptionParser to initialize.
 * @param options An array of CliOption structures representing the available
 * options.
 * @param option_count The number of options in the options array.
 */
void cli_option_parser_init(CliOptionParser *parser,
                            CliOption const *options,
                            int32_t          option_count);

/**
 * @brief Parses the next command-line option.
 *
 * @param parser The CliOptionParser to use for parsing.
 * @param argc The number of arguments in argv.
 * @param argv The array of command-line arguments.
 * @param envp The array of environment variables. (may be NULL)
 * @return An OptionResult representing the parsed option.
 */
OptionResult cli_option_parser_parse_option(CliOptionParser *parser,
                                            int32_t          argc,
                                            char const      *argv[],
                                            char const      *envp[]);

#endif // !EXP_SUPPORT_CLI_OPTION_PARSER_H