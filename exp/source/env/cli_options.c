/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env/cli_options.h"
#include "support/config.h"
#include "support/io.h"
#include "support/log.h"

#define SET_BIT(B, r) ((B) |= (u64)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u64)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

static void print_version(FILE *file) {
    file_write(SV(EXP_VERSION_STRING), file);
    file_write(SV("\n"), file);
}

static void print_help(FILE *file) {
    file_write(SV("exp [options] <source-file>\n\n"), file);
    file_write(SV("\t-h print help.\n"), file);
    file_write(SV("\t-v print version.\n"), file);
    file_write(SV("\t-o <filename> set output filename.\n"), file);
    file_write(SV("\t-c emit an object file.\n"), file);
    file_write(SV("\t-s emit an assembly file.\n"), file);
    file_write(SV("\n"), file);
}

CLIOptions cli_options_create() {
    CLIOptions cli_options = {.flags = 0};
    SET_BIT(cli_options.flags, CLI_DO_ASSEMBLE);
    SET_BIT(cli_options.flags, CLI_DO_LINK);
    SET_BIT(cli_options.flags, CLI_DO_CLEANUP);
    return cli_options;
}

void cli_options_destroy(CLIOptions *restrict cli_options) {
    cli_options->flags = 0;
    string_destroy(&cli_options->output);
    string_destroy(&cli_options->source);
}

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <getopt.h>

CLIOptions parse_cli_options(i32 argc, char const *argv[]) {
    CLIOptions options               = cli_options_create();
    static char const *short_options = "hvo:cs";

    i32 option = 0;
    while ((option = getopt(argc, (char *const *)argv, short_options)) != -1) {
        switch (option) {
        case 'h': {
            print_help(stdout);
            exit(EXIT_SUCCESS);
            break;
        }

        case 'v': {
            print_version(stdout);
            exit(EXIT_SUCCESS);
            break;
        }

        case 'o': {
            string_assign(&(options.output), string_view_from_cstring(optarg));
            break;
        }

        case 'c': {
            CLR_BIT(options.flags, CLI_DO_CLEANUP);
            CLR_BIT(options.flags, CLI_DO_LINK);
            break;
        }

        case 's': {
            CLR_BIT(options.flags, CLI_DO_CLEANUP);
            CLR_BIT(options.flags, CLI_DO_ASSEMBLE);
            CLR_BIT(options.flags, CLI_DO_LINK);
            break;
        }

        default: {
            char buf[2] = {(char)option, '\0'};
            file_write(SV("unknown option ["), stderr);
            file_write(string_view_from_str(buf, sizeof(buf) - 1), stderr);
            file_write(SV("]\n"), stderr);
            break;
        }
        }
    }

    if (optind < argc) {
        string_assign(&(options.source),
                      string_view_from_cstring(argv[optind]));
    } else { // no input file given
        exp_log(LOG_ERROR,
                NULL,
                0,
                SV("an input file must be specified.\n"),
                stderr);
        exit(EXIT_SUCCESS);
    }

    // use the input filename as the default
    // base of the output filename
    if (string_empty(&(options.output))) {
        string_assign_string(&options.output, &options.source);
        string_replace_extension(&options.output, SV(""));
    }

    return options;
}

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT

#else
#error "unsupported host OS"
#endif
