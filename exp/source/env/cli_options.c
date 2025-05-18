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

#include "codegen/IR/target.h"
#include "codegen/x86/target.h"
#include "env/cli_options.h"
#include "support/config.h"
#include "support/io.h"
#include "support/message.h"

void cli_options_create(CLIOptions *restrict cli_options) {
    cli_options->context_options.prolix                     = false;
    cli_options->context_options.create_assembly_artifact   = true;
    cli_options->context_options.create_object_artifact     = true;
    cli_options->context_options.create_executable_artifact = true;
    cli_options->context_options.cleanup_assembly_artifact  = true;
    cli_options->context_options.cleanup_object_artifact    = true;
    string_initialize(&cli_options->source);
}

void cli_options_destroy(CLIOptions *restrict cli_options) {
    string_destroy(&cli_options->source);
}

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <getopt.h>

static void print_version(FILE *file) {
    file_write(SV(EXP_VERSION_STRING), file);
    file_write(SV("\n"), file);
}

static void print_help(FILE *file) {
    file_write(SV("exp [options] <source-file>\n\n"), file);
    file_write(SV("\t-h print help.\n"), file);
    file_write(SV("\t-v print version.\n"), file);
    file_write(SV("\t-c emit an object file.\n"), file);
    file_write(SV("\t-s emit an assembly file.\n"), file);
    file_write(SV("\t-t <x86_64|IR> select target.\n"), file);
    file_write(SV("\n"), file);
}

void parse_cli_options(i32         argc,
                       char const *argv[],
                       CLIOptions *restrict cli_options) {
    static char const *short_options = "hvpcst:";

    // Set the default target
    cli_options->context_options.target = x86_target;

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

        case 'p': {
            cli_options->context_options.prolix = true;
            break;
        }

        case 'c': {
            cli_options->context_options.create_executable_artifact = false;
            cli_options->context_options.cleanup_object_artifact    = false;
            break;
        }

        case 's': {
            cli_options->context_options.create_executable_artifact = false;
            cli_options->context_options.create_object_artifact     = false;
            cli_options->context_options.cleanup_assembly_artifact  = false;
            cli_options->context_options.cleanup_object_artifact    = false;
            break;
        }

        case 't': {
            StringView argument = string_view_from_cstring(optarg);
            if (string_view_equal(argument, x86_target->tag)) {
                cli_options->context_options.target = x86_target;
            } else if (string_view_equal(argument, ir_target->tag)) {
                cli_options->context_options.target = ir_target;
                // #TODO: ensure that -s is specified when the target is IR.
                // as this workaround is unintuitive.
                cli_options->context_options.create_executable_artifact = false;
                cli_options->context_options.create_object_artifact     = false;
                cli_options->context_options.cleanup_assembly_artifact  = false;
                cli_options->context_options.cleanup_object_artifact    = false;
            }
            break;
        }

        default: {
            char       buf[2]      = {(char)option, '\0'};
            StringView option_view = string_view(buf, 1);
            String     string      = string_create();
            string_append(&string, SV("unknown option ["));
            string_append(&string, option_view);
            string_append(&string, SV("]\n"));
            message(MESSAGE_ERROR, NULL, 0, string_to_view(&string), stderr);
            string_destroy(&string);
            break;
        }
        }
    }

    if (optind < argc) {
        string_assign(&(cli_options->source),
                      string_view_from_cstring(argv[optind]));
    } else { // no input file given
        message(MESSAGE_ERROR,
                NULL,
                0,
                SV("an input file must be specified.\n"),
                stderr);
        exit(EXIT_SUCCESS);
    }
}

#else
#error "unsupported host OS"
#endif
