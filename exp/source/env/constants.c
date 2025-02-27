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
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

void constants_initialize(Constants *constants) {
    assert(constants != nullptr);
    constants->count    = 0;
    constants->capacity = 0;
    constants->buffer   = NULL;
}

void constants_terminate(Constants *constants) {
    assert(constants != NULL);
    for (u16 i = 0; i < constants->count; ++i) {
        Value *constant = constants->buffer + i;
        value_destroy(constant);
    }

    constants->count    = 0;
    constants->capacity = 0;
    deallocate(constants->buffer);
    constants->buffer = NULL;
}

static bool constants_full(Constants *constants) {
    assert(constants != NULL);
    return (constants->count + 1) >= constants->capacity;
}

static void constants_grow(Constants *constants) {
    assert(constants != NULL);
    Growth64 g          = array_growth_u64(constants->capacity, sizeof(Value));
    constants->buffer   = reallocate(constants->buffer, g.alloc_size);
    constants->capacity = g.new_capacity;
}

Operand constants_append(Constants *constants, Value value) {
    assert(constants != NULL);
    for (u64 index = 0; index < constants->count; ++index) {
        Value *cursor = constants->buffer + index;
        if (value_equality(cursor, &value)) {
            value_destroy(&value);
            return operand_constant((u16)index);
        }
    }

    if (constants_full(constants)) { constants_grow(constants); }

    u64 index = constants->count;
    assert(constants != NULL);
    constants->buffer[constants->count] = value;
    constants->count += 1;

    if (index > u16_MAX) { PANIC("constant index out of bounds"); }
    return operand_constant((u16)index);
}

Value *constants_at(Constants *constants, u16 index) {
    assert(constants != NULL);
    assert(index < constants->count);
    return constants->buffer + index;
}

void print_constants(Constants const *constants, FILE *file, Context *context) {
    for (u64 i = 0; i < constants->count; ++i) {
        file_write_u64(i, file);
        file_write(": ", file);
        file_write("[", file);
        print_value(constants->buffer + i, file, context);
        file_write("]\n", file);
    }
}
