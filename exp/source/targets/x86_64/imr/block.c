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

#include "targets/x86_64/imr/block.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void x86_64_block_initialize(x86_64_Block *block) {
    EXP_ASSERT(block != nullptr);
    block->buffer   = nullptr;
    block->length   = 0;
    block->capacity = 0;
}

void x86_64_block_terminate(x86_64_Block *block) {
    EXP_ASSERT(block != nullptr);
    deallocate(block->buffer);
    x86_64_block_initialize(block);
}

u32 x86_64_block_current_offset(x86_64_Block const *block) {
    EXP_ASSERT(block != nullptr);
    return block->length - 1;
}

static bool x86_64_block_full(x86_64_Block const *block) {
    return (block->length + 1) >= block->capacity;
}

static void x86_64_block_grow(x86_64_Block *block) {
    Growth32 g = array_growth_u32(block->capacity, sizeof(x86_64_Instruction));
    block->buffer   = reallocate(block->buffer, g.alloc_size);
    block->capacity = g.new_capacity;
}

void x86_64_block_insert(x86_64_Block *block,
                         x86_64_Instruction instruction,
                         u32 offset) {
    EXP_ASSERT(block != nullptr);
    EXP_ASSERT(offset <= block->length);
    if (x86_64_block_full(block)) { x86_64_block_grow(block); }

    for (u32 index = block->length; index > offset; --index) {
        block->buffer[index] = block->buffer[index - 1];
    }

    block->buffer[offset] = instruction;
    block->length += 1;
}

void x86_64_block_prepend(x86_64_Block *block, x86_64_Instruction instruction) {
    EXP_ASSERT(block != nullptr);
    x86_64_block_insert(block, instruction, 0);
}

void x86_64_block_append(x86_64_Block *block, x86_64_Instruction instruction) {
    EXP_ASSERT(block != nullptr);
    x86_64_block_insert(block, instruction, block->length);
    block->length += 1;
}

void print_x86_64_block(String *buffer,
                        x86_64_Block const *block,
                        struct Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(block != nullptr);
    EXP_ASSERT(context != nullptr);
    for (u32 index = 0; index < block->length; ++index) {
        print_x86_64_instruction(buffer, block->buffer[index], context);
    }
}
