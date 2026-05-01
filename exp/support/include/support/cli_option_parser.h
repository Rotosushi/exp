// SPDX-License-Identifier: MIT

/*!
 * \file cli_option_parser.h
 * \brief Defines the CliOptionParser type and member functions.
 */

#ifndef EXP_SUPPORT_CLI_OPTION_PARSER_H
#define EXP_SUPPORT_CLI_OPTION_PARSER_H

#include <stdint.h>

#include "support/string_view.h"

/*! \enum CliOptionArgumentKind
 * \brief Denotes the kind of argument an option takes.
 */
typedef enum CliOptionArgumentKind {
    OPTION_ARGUMENT_NONE,     //!< The option does not take an argument
    OPTION_ARGUMENT_REQUIRED, //!< The option requires an argument
    OPTION_ARGUMENT_OPTIONAL, //!< The option may optionally take an argument
} CliOptionArgumentKind;

/*! \struct CliOption
 * \brief Represents a command-line option.
 */
typedef struct CliOption {
    char const
        *name; /*!< The long name of the option (e.g., "help" for --help) */

    char const *
        description; /*!< A brief description of the option for help messages */

    char const
        *environment_variable; /*!< The name of the environment variable that
                                * can be used to set the option's value (may be
                                * NULL if not applicable) */

    CliOptionArgumentKind argument_kind; /*!< The kind of argument the option
                                            takes (none, required, optional) */

    char short_name; /*!< The short name of the option (e.g., 'h' for --help) */
} CliOption;

/** \struct CliOptionParser
 * @brief Represents a command-line option parser.
 */
typedef struct CliOptionParser {
    int32_t option_index; /*!< The index of the next option to parse in argv */

    int32_t
        suboption_index; /*!< The index of the next suboption to parse for the
                          * current option (used for parsing single-character
                          * options that can be combined, e.g., -abc) */

    int32_t
        option_count; /*!< The total number of options in the options array */

    CliOption const *options; /*!< An array of CliOption structures representing
                                 the available options */
} CliOptionParser;

/*!
 * \def CLI_OPTION_END
 * \brief Indicates that there are no more options to parse.
 */
#define CLI_OPTION_END 0

/*!
 * \def CLI_OPTION_MISSING_ARGUMENT
 * \brief Indicates that a required argument for an option is missing.
 */
#define CLI_OPTION_MISSING_ARGUMENT -1

/*!
 * \def CLI_OPTION_UNRECOGNIZED
 * \brief Indicates that an unrecognized option was encountered.
 */
#define CLI_OPTION_UNRECOGNIZED '?'

/*!
 * \def CLI_OPTION_POSITIONAL_ARGUMENT
 * \brief Indicates that a positional argument (not an option) was encountered.
 */
#define CLI_OPTION_POSITIONAL_ARGUMENT '!'

/** \struct CliOptionResult
 * \brief Represents the result of parsing the next available command-line
 * option.
 */
typedef struct CliOptionResult {
    int32_t
        option; /*!< The short name of the parsed option (e.g., 'h' for --help),
                 * or a special value indicating the result of the parse  */

    char const *argument; /*!< The argument associated with the parsed option,
                           * or NULL if no argument was provided. */
} CliOptionResult;

void            cli_option_parser_init(CliOptionParser *parser,
                                       CliOption const *options,
                                       int32_t          option_count);
CliOptionResult cli_option_parser_parse_option(CliOptionParser *parser,
                                               int32_t          argc,
                                               char const      *argv[],
                                               char const      *envp[]);

#endif // !EXP_SUPPORT_CLI_OPTION_PARSER_H