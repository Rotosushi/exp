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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>

#include "env/constants.h"
#include "env/context.h"
#include "support/allocation.h"
#include "support/array_growth.h"

Constants constants_create() {
    Constants constants;
    constants.count    = 0;
    constants.capacity = 0;
    constants.buffer   = NULL;
    return constants;
}

void constants_destroy(Constants *restrict constants) {
    assert(constants != NULL);
    for (u64 i = 0; i < constants->count; ++i) {
        Value *constant = constants->buffer + i;
        value_destroy(constant);
    }

    constants->count    = 0;
    constants->capacity = 0;
    deallocate(constants->buffer);
    constants->buffer = NULL;
}

static bool constants_full(Constants *restrict constants) {
    assert(constants != NULL);
    return (constants->count + 1) >= constants->capacity;
}

static void constants_grow(Constants *restrict constants) {
    assert(constants != NULL);
    Growth_u32 g        = array_growth_u32(constants->capacity, sizeof(Value));
    constants->buffer   = reallocate(constants->buffer, g.alloc_size);
    constants->capacity = g.new_capacity;
}

Operand constants_append(Constants *restrict constants, Value value) {
    assert(constants != NULL);
    for (u32 i = 0; i < constants->count; ++i) {
        Value *v = constants->buffer + i;
        if (value_equality(v, &value)) {
            value_destroy(&value);
            return operand_constant(i);
        }
    }

    if (constants_full(constants)) { constants_grow(constants); }

    u32 index = constants->count;
    assert(constants != NULL);
    constants->buffer[constants->count] = value;
    constants->count += 1;

    return operand_constant(index);
}

Value *constants_at(Constants *restrict constants, u32 index) {
    assert(constants != NULL);
    assert(index < constants->count);
    return constants->buffer + index;
}

void print_constants(String *restrict string,
                     Constants const *restrict constants,
                     Context *restrict context) {
    for (u16 i = 0; i < constants->count; ++i) {
        string_append_u64(string, i);
        string_append(string, SV(": "));
        string_append(string, SV("["));
        print_value(string, constants->buffer + i, context);
        string_append(string, SV("]\n"));
    }
}
