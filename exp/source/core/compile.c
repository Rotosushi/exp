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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdlib.h>

#include "core/analyze.h"
#include "core/assemble.h"
#include "core/codegen.h"
#include "core/compile.h"
#include "core/link.h"
#include "env/cli_options.h"
#include "env/context.h"
#include "scanning/parser.h"
#include "support/io.h"
#include "support/message.h"

static void print_compile_actions(Context *restrict context) {
    if (context_create_ir_artifact(context)) {
        trace(SV("create ir artifact:"), stdout);
        trace(context_ir_path(context), stdout);
    }

    if (context_create_assembly_artifact(context)) {
        trace(SV("create assembly artifact:"), stdout);
        trace(context_assembly_path(context), stdout);
    }

    if (context_create_object_artifact(context)) {
        trace(SV("create object artifact:"), stdout);
        trace(context_object_path(context), stdout);
    }

    if (context_create_executable_artifact(context)) {
        trace(SV("create executable artifact:"), stdout);
        trace(context_executable_path(context), stdout);
    }

    if (context_cleanup_ir_artifact(context)) {
        trace(SV("cleanup ir artifact"), stdout);
    }

    if (context_cleanup_assembly_artifact(context)) {
        trace(SV("cleanup assembly artifact"), stdout);
    }

    if (context_cleanup_object_artifact(context)) {
        trace(SV("cleanup object artifact"), stdout);
    }
}

static i32 compile_context(Context *restrict c) {
    if (parse_source(c) == EXIT_FAILURE) { return EXIT_FAILURE; }

    if (analyze(c) == EXIT_FAILURE) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

i32 compile(i32 argc, char const *argv[]) {
    CLIOptions cli_options = parse_cli_options(argc, argv);
    Context    context     = context_create(&cli_options);

    if (context_prolix(&context)) {
        message(MESSAGE_STATUS, NULL, 0, SV("prolix mode enabled"), stdout);
        print_compile_actions(&context);
    }

    if (context_trace(&context)) {
        message(MESSAGE_STATUS, NULL, 0, SV("trace mode enabled"), stdout);
    }

    i32 result = compile_context(&context);

    if ((result != EXIT_FAILURE) && context_create_ir_artifact(&context)) {
        result |= codegen_ir(&context);
    }

    if ((result != EXIT_FAILURE) &&
        context_create_assembly_artifact(&context)) {
        result |= codegen_assembly(&context);
    }

    if ((result != EXIT_FAILURE) && context_create_object_artifact(&context)) {
        result |= assemble(&context);
    }

    if ((result != EXIT_FAILURE) &&
        context_create_executable_artifact(&context)) {
        result |= link(&context);
    }

    if (context_cleanup_assembly_artifact(&context) &&
        (result != EXIT_FAILURE)) {
        StringView asm_path = context_assembly_path(&context);
        file_remove(asm_path.ptr);
    }

    if (context_cleanup_object_artifact(&context) && (result != EXIT_FAILURE)) {
        StringView obj_path = context_object_path(&context);
        file_remove(obj_path.ptr);
    }

    context_destroy(&context);
    cli_options_destroy(&cli_options);
    return result;
}
