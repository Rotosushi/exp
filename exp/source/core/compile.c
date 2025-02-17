/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <stdlib.h>

#include "core/analyze.h"
#include "core/assemble.h"
#include "core/codegen.h"
#include "core/compile.h"
#include "core/link.h"
#include "env/cli_options.h"
#include "env/context.h"
#include "frontend/parser.h"
#include "utility/assert.h"
#include "utility/io.h"

static ExpResult compile_context(Context *context) {
    EXP_ASSERT(context != nullptr);
    if (parse_source(context) == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (analyze_context(context) == EXIT_FAILURE) { return EXIT_FAILURE; }

    codegen(context);

    if (context_create_elf_object(context)) { return assemble(context); }
    return EXIT_SUCCESS;
}

static void context_flags_from_cli_flags(Bitset *context_flags,
                                         Bitset *cli_flags) {
    EXP_ASSERT(context_flags != nullptr);
    EXP_ASSERT(cli_flags != nullptr);
    bitset_assign_bit(context_flags, CONTEXT_OPTION_EMIT_IR_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_EMIT_IR_ASSEMBLY));
    bitset_assign_bit(context_flags, CONTEXT_OPTION_EMIT_X86_64_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_EMIT_X86_64_ASSEMBLY));
    bitset_assign_bit(context_flags, CONTEXT_OPTION_CREATE_ELF_OBJECT,
                      bitset_check_bit(cli_flags, CLI_CREATE_ELF_OBJECT));
    bitset_assign_bit(context_flags, CONTEXT_OPTION_CREATE_ELF_EXECUTABLE,
                      bitset_check_bit(cli_flags, CLI_CREATE_ELF_EXECUTABLE));
    bitset_assign_bit(context_flags, CONTEXT_OPTION_CLEANUP_TARGET_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_CLEANUP_X86_64_ASSEMBLY));
    bitset_assign_bit(context_flags, CONTEXT_OPTION_CLEANUP_ELF_OBJECT,
                      bitset_check_bit(cli_flags, CLI_CLEANUP_ELF_OBJECT));
}

i32 compile(i32 argc, char const *argv[]) {
    EXP_ASSERT(argv != nullptr);
    CLIOptions cli_options;
    parse_cli_options(&cli_options, argc, argv);

    Bitset context_flags = bitset_create();
    context_flags_from_cli_flags(&context_flags, &cli_options.flags);

    Context context;
    context_initialize(&context, context_flags,
                       string_to_view(&cli_options.source),
                       string_to_view(&cli_options.output));

    ExpResult result = compile_context(&context);

    if ((result != EXP_FAILURE) && context_create_elf_executable(&context)) {
        result |= link(&context);
    }

    if ((result != EXP_FAILURE) && context_cleanup_x86_64_assembly(&context)) {
        StringView asm_path = context_assembly_path(&context);
        file_remove(asm_path);
    }

    if ((result != EXP_FAILURE) && context_cleanup_elf_object(&context)) {
        StringView obj_path = context_object_path(&context);
        file_remove(obj_path);
    }

    context_terminate(&context);
    cli_options_terminate(&cli_options);

    if (result == EXP_SUCCESS) return EXIT_SUCCESS;
    else return EXIT_FAILURE;
}
