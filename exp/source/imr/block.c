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
#include <stddef.h>

#include "env/context.h"
#include "imr/block.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void block_create(Block *restrict block) {
    EXP_ASSERT(block != NULL);
    block->length   = 0;
    block->capacity = 0;
    block->buffer   = NULL;
}

void block_destroy(Block *restrict block) {
    EXP_ASSERT(block != NULL);
    block->length   = 0;
    block->capacity = 0;
    deallocate(block->buffer);
    block->buffer = NULL;
}

static bool block_full(Block *restrict block) {
    EXP_ASSERT(block != NULL);
    return block->capacity <= (block->length + 1);
}

static void block_grow(Block *restrict block) {
    EXP_ASSERT(block != NULL);
    Growth_u32 g    = array_growth_u32(block->capacity, sizeof(Instruction));
    block->buffer   = reallocate(block->buffer, g.alloc_size);
    block->capacity = g.new_capacity;
}

void block_append(Block *restrict block, Instruction I) {
    EXP_ASSERT(block != NULL);
    if (block_full(block)) { block_grow(block); }

    block->buffer[block->length] = I;
    block->length += 1;
}

void print_block(String *restrict string,
                 Block const *restrict block,
                 struct Context *restrict context) {
    EXP_ASSERT(string != NULL);
    EXP_ASSERT(block != NULL);
    EXP_ASSERT(context != NULL);
    // walk the entire buffer and print each instruction
    for (u32 i = 0; i < block->length; ++i) {
        string_append(string, SV("  "));
        string_append_u64(string, i);
        string_append(string, SV(": "));
        print_instruction(string, block->buffer[i], context);
        string_append(string, SV("\n"));
    }
}
