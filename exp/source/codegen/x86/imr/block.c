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

#include "codegen/x86/imr/block.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_block_create(x86_Block *restrict block) {
    exp_assert(block != NULL);
    block->length   = 0;
    block->capacity = 0;
    block->buffer   = NULL;
}

void x86_block_destroy(x86_Block *restrict block) {
    exp_assert(block != NULL);
    deallocate(block->buffer);
    x86_block_create(block);
}

u32 x86_block_current_offset(x86_Block *restrict block) {
    exp_assert(block != NULL);
    return block->length;
}

static bool x86_bytecode_full(x86_Block *restrict block) {
    return (block->length + 1) >= block->capacity;
}

static void x86_bytecode_grow(x86_Block *restrict block) {
    Growth_u32 g  = array_growth_u32(block->capacity, sizeof(x86_Instruction));
    block->buffer = reallocate(block->buffer, g.alloc_size);
    block->capacity = g.new_capacity;
}

void x86_block_insert(x86_Block *restrict block,
                      x86_Instruction instruction,
                      u32             offset) {
    exp_assert(block != NULL);
    exp_assert(offset <= block->capacity);
    exp_assert(offset <= block->length);
    if (x86_bytecode_full(block)) { x86_bytecode_grow(block); }

    for (u32 index = block->length; index > offset; --index) {
        block->buffer[index] = block->buffer[index - 1];
    }

    block->buffer[offset] = instruction;
    block->length += 1;
}

void x86_block_append(x86_Block *restrict block, x86_Instruction instruction) {
    exp_assert(block != NULL);
    x86_block_insert(block, instruction, block->length);
}

void x86_block_prepend(x86_Block *restrict block, x86_Instruction instruction) {
    exp_assert(block != NULL);
    x86_block_insert(block, instruction, 0);
}

void print_x86_block(String *restrict buffer, x86_Block const *restrict block) {
    exp_assert(buffer != NULL);
    exp_assert(block != NULL);
    for (u32 index = 0; index < block->length; ++index) {
        string_append(buffer, SV("\t"));
        print_x86_instruction(buffer, block->buffer[index]);
        string_append(buffer, SV("\n"));
    }
}
