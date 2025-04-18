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
#include "imr/bytecode.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void bytecode_create(Bytecode *restrict bytecode) {
    exp_assert(bytecode != NULL);
    bytecode->length   = 0;
    bytecode->capacity = 0;
    bytecode->buffer   = NULL;
}

void bytecode_destroy(Bytecode *restrict bytecode) {
    exp_assert(bytecode != NULL);
    bytecode->length   = 0;
    bytecode->capacity = 0;
    deallocate(bytecode->buffer);
    bytecode->buffer = NULL;
}

static bool bytecode_full(Bytecode *restrict bytecode) {
    return bytecode->capacity <= (bytecode->length + 1);
}

static void bytecode_grow(Bytecode *restrict bytecode) {
    Growth_u32 g = array_growth_u32(bytecode->capacity, sizeof(Instruction));
    bytecode->buffer   = reallocate(bytecode->buffer, g.alloc_size);
    bytecode->capacity = g.new_capacity;
}

void bytecode_append(Bytecode *restrict bytecode, Instruction I) {
    if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

    bytecode->buffer[bytecode->length] = I;
    bytecode->length += 1;
}

void print_bytecode(String *restrict string,
                    Bytecode const *restrict bc,
                    struct Context *restrict context) {
    // walk the entire buffer and print each instruction
    for (u32 i = 0; i < bc->length; ++i) {
        string_append(string, SV("  "));
        string_append_u64(string, i);
        string_append(string, SV(": "));
        print_instruction(string, bc->buffer[i], context);
        string_append(string, SV("\n"));
    }
}
