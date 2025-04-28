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
#include <stdlib.h>

#include "env/constants.h"
#include "env/context.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void constants_create(Constants *restrict constants) {
    exp_assert(constants != NULL);
    constants->count    = 0;
    constants->capacity = 0;
    constants->buffer   = NULL;
}

void constants_destroy(Constants *restrict constants) {
    exp_assert(constants != NULL);

    for (u64 i = 0; i < constants->count; ++i) {
        value_deallocate(constants->buffer[i]);
    }

    deallocate(constants->buffer);
    constants_create(constants);
}

static bool constants_full(Constants *restrict constants) {
    exp_assert(constants != NULL);
    return (constants->count + 1) >= constants->capacity;
}

static void constants_grow(Constants *restrict constants) {
    exp_assert(constants != NULL);
    Growth_u32 g        = array_growth_u32(constants->capacity, sizeof(Value));
    constants->buffer   = reallocate(constants->buffer, g.alloc_size);
    constants->capacity = g.new_capacity;
}

Operand constants_append(Constants *restrict constants, Value *value) {
    exp_assert(constants != NULL);

    // search for an existing constant of the same value,
    // so we don't allocate a ton of redundant Constants.
    for (u32 i = 0; i < constants->count; ++i) {
        Value *existing = constants->buffer[i];
        if (value_equal(existing, value)) {
            value_deallocate(value);
            return operand_constant(existing);
        }
    }

    if (constants_full(constants)) { constants_grow(constants); }

    constants->buffer[constants->count++] = value;

    return operand_constant(value);
}

Value const *constants_at(Constants *restrict constants, u32 index) {
    exp_assert(constants != NULL);
    exp_assert(index < constants->count);
    return constants->buffer[index];
}

void print_constants(String *restrict string,
                     Constants const *restrict constants,
                     Context *restrict context) {
    for (u16 i = 0; i < constants->count; ++i) {
        string_append_u64(string, i);
        string_append(string, SV(": "));
        string_append(string, SV("["));
        print_value(string, constants->buffer[i], context);
        string_append(string, SV("]\n"));
    }
}
