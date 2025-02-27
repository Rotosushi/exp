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
#include "frontend/parser.h"

static i32 compile_context(Context *context) {
    if (parse_source(context) == EXIT_FAILURE) { return EXIT_FAILURE; }

    if (analyze(context) == EXIT_FAILURE) { return EXIT_FAILURE; }

    codegen(context);

    if (context_create_elf_object(context)) { return assemble(context); }
    return EXIT_SUCCESS;
}

static void context_flags_from_cli_flags(Bitset *context_flags,
                                         Bitset *cli_flags) {
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_EMIT_IR_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_EMIT_IR_ASSEMBLY));
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_EMIT_X86_64_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_EMIT_X86_64_ASSEMBLY));
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_CREATE_ELF_OBJECT,
                      bitset_check_bit(cli_flags, CLI_CREATE_ELF_OBJECT));
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_CREATE_ELF_EXECUTABLE,
                      bitset_check_bit(cli_flags, CLI_CREATE_ELF_EXECUTABLE));
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_CLEANUP_TARGET_ASSEMBLY,
                      bitset_check_bit(cli_flags, CLI_CLEANUP_X86_64_ASSEMBLY));
    bitset_assign_bit(context_flags,
                      CONTEXT_OPTION_CLEANUP_ELF_OBJECT,
                      bitset_check_bit(cli_flags, CLI_CLEANUP_ELF_OBJECT));
}

i32 compile(i32 argc, char const *argv[]) {
    CLIOptions cli_options;
    parse_cli_options(&cli_options, argc, argv);

    Bitset context_flags = bitset_create();
    context_flags_from_cli_flags(&context_flags, &cli_options.flags);

    Context context;
    context_initialize(&context,
                       context_flags,
                       string_to_view(&cli_options.source),
                       string_to_view(&cli_options.output));

    i32 result = compile_context(&context);

    if ((result != EXIT_FAILURE) && context_create_elf_executable(&context)) {
        result |= link(&context);
    }

    if ((result != EXIT_FAILURE) && context_cleanup_x86_64_assembly(&context)) {
        StringView asm_path = context_assembly_path(&context);
        file_remove(asm_path.ptr);
    }

    if ((result != EXIT_FAILURE) && context_cleanup_elf_object(&context)) {
        StringView obj_path = context_object_path(&context);
        file_remove(obj_path.ptr);
    }

    context_terminate(&context);
    cli_options_terminate(&cli_options);
    return result;
}
