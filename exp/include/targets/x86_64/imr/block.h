// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_TARGETS_X86_64_IMR_BLOCK_H
#define EXP_TARGETS_X86_64_IMR_BLOCK_H

#include "targets/x86_64/imr/instruction.h"
#include "utility/string.h"

typedef struct x86_64_Block {
    u32 length;
    u32 capacity;
    x86_64_Instruction *buffer;
} x86_64_Block;

void x86_64_block_initialize(x86_64_Block *block);
void x86_64_block_terminate(x86_64_Block *block);

u32 x86_64_block_current_offset(x86_64_Block const *block);

void x86_64_block_insert(x86_64_Block *block, x86_64_Instruction instruction,
                         u32 offset);
void x86_64_block_prepend(x86_64_Block *block, x86_64_Instruction instruction);
void x86_64_block_append(x86_64_Block *block, x86_64_Instruction instruction);

struct Context;
void print_x86_64_block(String *buffer, x86_64_Block const *block,
                        struct Context *context);

#endif // EXP_TARGETS_X86_64_IMR_BLOCK_H
