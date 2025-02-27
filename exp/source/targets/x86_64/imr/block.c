/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

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

void x86_64_block_insert(x86_64_Block *block, x86_64_Instruction instruction,
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

void print_x86_64_block(String *buffer, x86_64_Block const *block,
                        struct Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(block != nullptr);
    EXP_ASSERT(context != nullptr);
    for (u32 index = 0; index < block->length; ++index) {
        print_x86_64_instruction(buffer, block->buffer[index], context);
    }
}
