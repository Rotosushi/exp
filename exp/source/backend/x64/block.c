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
#include <assert.h>

#include "backend/x64/block.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

void x64_block_initialize(x64_Block *block) {
    assert(block != nullptr);
    block->length   = 0;
    block->capacity = 0;
    block->buffer   = nullptr;
}

void x64_block_terminate(x64_Block *block) {
    assert(block != NULL);
    block->length   = 0;
    block->capacity = 0;
    deallocate(block->buffer);
    block->buffer = NULL;
}

static bool x64_bytecode_full(x64_Block *block) {
    assert(block != nullptr);
    return (block->length + 1) >= block->capacity;
}

static void x64_bytecode_grow(x64_Block *block) {
    assert(block != nullptr);
    Growth64 g    = array_growth_u64(block->capacity, sizeof(x64_Instruction));
    block->buffer = reallocate(block->buffer, g.alloc_size);
    block->capacity = g.new_capacity;
}

u64 x64_block_current_offset(x64_Block *block) {
    assert(block != nullptr);
    return block->length;
}

void x64_block_insert(x64_Block *block,
                      x64_Instruction instruction,
                      u64 offset) {
    assert(block != nullptr);
    assert(offset < block->capacity);
    assert(offset <= block->length);
    if (x64_bytecode_full(block)) { x64_bytecode_grow(block); }

    for (u64 i = block->length; i > offset; --i) {
        block->buffer[i] = block->buffer[i - 1];
    }

    block->buffer[offset] = instruction;
    block->length += 1;
}

void x64_block_append(x64_Block *block, x64_Instruction instruction) {
    assert(block != nullptr);
    if (x64_bytecode_full(block)) { x64_bytecode_grow(block); }

    block->buffer[block->length] = instruction;
    block->length += 1;
}

void x64_block_prepend(x64_Block *block, x64_Instruction instruction) {
    assert(block != nullptr);
    if (x64_bytecode_full(block)) { x64_bytecode_grow(block); }

    // move all instructions forward one location
    for (u64 idx = block->length; idx > 0; --idx) {
        block->buffer[idx] = block->buffer[idx - 1];
    }

    block->buffer[0] = instruction;
    block->length += 1;
}

void x64_block_emit(x64_Block *block,
                    String *buffer,
                    struct x64_Context *context) {
    for (u16 i = 0; i < block->length; ++i) {
        string_append(buffer, SV("\t"));
        x64_instruction_emit(block->buffer[i], buffer, context);
        string_append(buffer, SV("\n"));
    }
}
