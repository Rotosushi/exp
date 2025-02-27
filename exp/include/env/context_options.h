// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_OPTIONS_H
#define EXP_ENV_OPTIONS_H

#include "utility/bitset.h"
#include "utility/string.h"

typedef enum ContextOption {
    CONTEXT_OPTION_EMIT_IR_ASSEMBLY,
    CONTEXT_OPTION_EMIT_X86_64_ASSEMBLY,

    CONTEXT_OPTION_CREATE_ELF_OBJECT,
    CONTEXT_OPTION_CREATE_ELF_EXECUTABLE,

    CONTEXT_OPTION_CLEANUP_TARGET_ASSEMBLY,
    CONTEXT_OPTION_CLEANUP_ELF_OBJECT,
} ContextOption;

/**
 * @brief holds the options relevant for the given
 * context.
 *
 */
typedef struct ContextOptions {
    Bitset flags;
    String source;
    String ir_assembly;
    String assembly;
    String object;
    String output;
} ContextOptions;

void context_options_initialize(ContextOptions *context_options, Bitset flags,
                                StringView source, StringView output);
void context_options_terminate(ContextOptions *options);

bool context_options_emit_ir_assembly(ContextOptions *options);
bool context_options_emit_x86_64_assembly(ContextOptions *options);

bool context_options_create_elf_object(ContextOptions *options);
bool context_options_create_elf_executable(ContextOptions *options);

bool context_options_cleanup_target_assembly(ContextOptions *options);
bool context_options_cleanup_elf_object(ContextOptions *options);

#endif // !EXP_ENV_OPTIONS_H
