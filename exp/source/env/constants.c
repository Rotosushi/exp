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
#include "utility/alloc.h"
#include "utility/array_growth.h"

void constants_initialize(Constants *constants) {
    assert(constants != nullptr);
    constants->count    = 0;
    constants->capacity = 0;
    constants->buffer   = NULL;
}

void constants_terminate(Constants *constants) {
    assert(constants != NULL);
    for (u64 i = 0; i < constants->count; ++i) {
        Value *constant = constants->buffer[i];
        value_terminate(constant);
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

Value *constants_append_tuple(Constants *constants, Tuple tuple) {
    assert(constants != NULL);

    Value *value = callocate(1, sizeof(Value));
    value_initialize_tuple(value, tuple);

    for (u64 index = 0; index < constants->count; ++index) {
        Value *cursor = constants->buffer[index];
        assert(cursor != nullptr);

        if (value_equality(cursor, value)) {
            value_terminate(value);
            return cursor;
        }
    }

    if (constants_full(constants)) { constants_grow(constants); }

    u64 index                = constants->count++;
    constants->buffer[index] = value;

    return constants->buffer[index];
}
