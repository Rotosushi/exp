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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env/cli_options.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/log.h"
#include "utility/panic.h"

static void print_version(FILE *file) {
    file_write(EXP_VERSION_STRING, file);
    file_write("\n", file);
}

static void print_help(FILE *file) {
    file_write("exp [options] <source-file>\n\n", file);
    file_write("\t-h print help.\n", file);
    file_write("\t-v print version.\n", file);
    file_write("\t-o <filename> set output filename.\n", file);
    file_write("\t-c emit an object file.\n", file);
    file_write("\t-s emit an assembly file.\n", file);
    file_write("\t-i emit an exp ir file.\n", file);
    file_write("\n", file);
}

void cli_options_initialize(CLIOptions *cli_options) {
    assert(cli_options != nullptr);
    cli_options->flags = bitset_create();
    // #TODO: make this the default. (currently the x64 backend is broken)
    //  bitset_set_bit(&cli_options->flags, CLI_EMIT_TARGET_ASSEMBLY);
    //  bitset_set_bit(&cli_options->flags, CLI_CREATE_ELF_OBJECT);
    //  bitset_set_bit(&cli_options->flags, CLI_CREATE_ELF_EXECUTABLE);
    //  bitset_set_bit(&cli_options->flags, CLI_CLEANUP_TARGET_ASSEMBLY);
    //  bitset_set_bit(&cli_options->flags, CLI_CLEANUP_ELF_OBJECT);
    string_initialize(&cli_options->source);
    string_initialize(&cli_options->output);
}

void cli_options_terminate(CLIOptions *cli_options) {
    assert(cli_options != nullptr);
    cli_options->flags = bitset_create();
    string_destroy(&cli_options->output);
    string_destroy(&cli_options->source);
}

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <getopt.h>

void parse_cli_options(CLIOptions *options, i32 argc, char const *argv[]) {
    assert(options != nullptr);
    cli_options_initialize(options);
    static char const *short_options = "hvo:cis";

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
            string_assign(&(options->output), string_view_from_cstring(optarg));
            break;
        }

        case 'c': {
            PANIC("#TODO:");
            break;
        }

        case 'i': {
            options->flags = bitset_create();
            bitset_set_bit(&options->flags, CLI_EMIT_IR_ASSEMBLY);
            break;
        }

        case 's': {
            PANIC("#TODO:");
            break;
        }

        default: {
            char buf[2] = {(char)option, '\0'};
            file_write("unknown option [", stderr);
            file_write(buf, stderr);
            file_write("]\n", stderr);
            break;
        }
        }
    }

    if (optind < argc) {
        string_assign(&(options->source),
                      string_view_from_cstring(argv[optind]));
    } else { // no input file given
        log_message(
            LOG_ERROR, NULL, 0, "an input file must be specified.\n", stderr);
        exit(EXIT_SUCCESS);
    }

    // use the input filename as the default
    // base of the output filename
    if (string_empty(&(options->output))) {
        string_assign_string(&options->output, &options->source);
        string_replace_extension(&options->output, SV(""));
    }
}

#else
#error "unsupported host OS"
#endif
