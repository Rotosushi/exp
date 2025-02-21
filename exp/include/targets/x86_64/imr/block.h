// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file targets/x86_64/imr/block.h
 */

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
