/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include <stdlib.h>

#include "env/cli_options.h"
#include "utility/assert.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/log.h"
#include "utility/panic.h"
#include "utility/result.h"
#include "utility/string.h"

#ifndef EXP_HOST_SYSTEM_LINUX
    #error "unsupported host OS"
#endif

#include <getopt.h>

static void print_version(File *file) {
    EXP_ASSERT(file != nullptr);
    String buffer;
    string_initialize(&buffer);
    string_append(&buffer, SV(EXP_VERSION_STRING));
    string_append(&buffer, SV("\n"));
    file_write(string_to_view(&buffer), file);
    string_terminate(&buffer);
}

static void print_help(File *file) {
    EXP_ASSERT(file != nullptr);
    String buffer;
    string_initialize(&buffer);
    string_append(&buffer, SV("exp [options] <source-file>\n\n"));
    string_append(&buffer, SV("\t-h print help.\n"));
    string_append(&buffer, SV("\t-v print version.\n"));
    string_append(&buffer, SV("\t-o <filename> set output filename.\n"));
    string_append(&buffer, SV("\t-c emit an object file.\n"));
    string_append(&buffer, SV("\t-s emit an assembly file.\n"));
    string_append(&buffer, SV("\t-i emit an exp ir file.\n"));
    string_append(&buffer, SV("\n"));
    file_write(string_to_view(&buffer), file);
    string_terminate(&buffer);
}

static void print_unknown_option(char option, File *file) {
    EXP_ASSERT(file != nullptr);
    String buffer;
    string_initialize(&buffer);
    char buf[2] = {option, '\0'};
    string_append(&buffer, SV("unknown option ["));
    string_append(&buffer, string_view_from_str(buf, 1));
    string_append(&buffer, SV("]\n"));
    file_write(string_to_view(&buffer), file);
    string_terminate(&buffer);
}

void cli_options_initialize(CLIOptions *cli_options) {
    EXP_ASSERT(cli_options != nullptr);
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
    EXP_ASSERT(cli_options != nullptr);
    cli_options->flags = bitset_create();
    string_terminate(&cli_options->output);
    string_terminate(&cli_options->source);
}

void parse_cli_options(CLIOptions *options, i32 argc, char const *argv[]) {
    EXP_ASSERT(options != nullptr);
    EXP_ASSERT(argv != nullptr);
    cli_options_initialize(options);
    static char const *short_options = "hvo:cis";

    i32 option = 0;
    while ((option = getopt(argc, (char *const *)argv, short_options)) != -1) {
        switch (option) {
        case 'h': {
            print_help(program_output);
            exit(EXP_SUCCESS);
            break;
        }

        case 'v': {
            print_version(program_output);
            exit(EXP_SUCCESS);
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
            print_unknown_option((char)option, program_error);
            break;
        }
        }
    }

    if (optind < argc) {
        string_assign(&(options->source),
                      string_view_from_cstring(argv[optind]));
    } else { // no input file given
        log_message(LOG_ERROR, nullptr, 0,
                    SV("an input file must be specified.\n"), program_error);
        exit(EXP_SUCCESS);
    }

    // use the input filename as the default
    // base of the output filename
    if (string_empty(&(options->output))) {
        string_assign_string(&options->output, &options->source);
        string_replace_extension(&options->output, SV(""));
    }
}
