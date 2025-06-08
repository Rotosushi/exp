// Copyright (C) 2024 Cade Weinberg
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
#ifndef EXP_BACKEND_X86_BYTECODE_H
#define EXP_BACKEND_X86_BYTECODE_H

#include "codegen/x86/imr/instruction.h"
#include "support/string.h"

/*
 * #NOTE: So, in order to support local jumps we are going to
 * shift the perspective on a functions body. We are going to
 * consider it a set of blocks of bytecode. where a single block
 * represents a unit of code with no jumps at all. That is,
 * a single throughline of code. And a jump is what terminates
 * a basic block, joining it to the other blocks within the body
 * of the function. (Yes, this idea is taken straight from LLVM.
 * it's a really good idea!)
 * I just had the thought of adding in two blocks which are more
 * implicit however, the "prefix" and "postfix" blocks which manage
 * the functions stack frame and locally save/restored registers.
 * This allows easy access to the functions prefix and postfix sections
 * without having to shuffle the whole block of instructions. (the
 * previous method of adding in the stack save/restore code was awkward
 * and felt hacky.)
 */

typedef struct x86_Block {
    u32              length;
    u32              capacity;
    x86_Instruction *buffer;
} x86_Block;

void x86_block_create(x86_Block *restrict block);
void x86_block_destroy(x86_Block *restrict block);

u32 x86_block_current_offset(x86_Block *restrict block);

void x86_block_insert(x86_Block *restrict block, x86_Instruction I, u32 offset);
void x86_block_prepend(x86_Block *restrict block, x86_Instruction I);
void x86_block_append(x86_Block *restrict block, x86_Instruction I);

void print_x86_block(String *restrict buffer, x86_Block const *restrict block);

#endif // !EXP_BACKEND_X86_BYTECODE_H
