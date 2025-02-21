/**
 * Copyright (C) 2025 Cade Weinberg
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

/**
 * @file env/context_options.c
 */
#include "env/context_options.h"
#include "utility/assert.h"

void context_options_initialize(ContextOptions *context_options, Bitset flags,
                                StringView source, StringView output) {
    EXP_ASSERT(context_options != nullptr);

    context_options->flags = flags;

    string_initialize(&context_options->source);
    string_initialize(&context_options->ir_assembly);
    string_initialize(&context_options->assembly);
    string_initialize(&context_options->object);
    string_initialize(&context_options->output);

    if (!string_view_empty(source)) {
        string_assign(&context_options->source, source);

        string_assign(&context_options->ir_assembly, source);
        string_replace_extension(&context_options->ir_assembly, SV(".expir"));

        string_assign(&context_options->assembly, source);
        string_replace_extension(&context_options->assembly, SV(".s"));

        string_assign(&context_options->object, source);
        string_replace_extension(&context_options->object, SV(".o"));
    }

    if (!string_view_empty(output)) {
        string_assign(&context_options->output, output);
        string_replace_extension(&context_options->output, SV(""));
    }
}

void context_options_terminate(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    options->flags = bitset_create();
    string_terminate(&options->source);
    string_terminate(&options->assembly);
    string_terminate(&options->object);
    string_terminate(&options->output);
}

bool context_options_emit_ir_assembly(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags, CONTEXT_OPTION_EMIT_IR_ASSEMBLY);
}

bool context_options_emit_x86_64_assembly(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags,
                            CONTEXT_OPTION_EMIT_X86_64_ASSEMBLY);
}

bool context_options_create_elf_object(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags, CONTEXT_OPTION_CREATE_ELF_OBJECT);
}

bool context_options_create_elf_executable(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags,
                            CONTEXT_OPTION_CREATE_ELF_EXECUTABLE);
}

bool context_options_cleanup_target_assembly(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags,
                            CONTEXT_OPTION_CLEANUP_TARGET_ASSEMBLY);
}

bool context_options_cleanup_elf_object(ContextOptions *options) {
    EXP_ASSERT(options != nullptr);
    return bitset_check_bit(&options->flags, CONTEXT_OPTION_CLEANUP_ELF_OBJECT);
}
