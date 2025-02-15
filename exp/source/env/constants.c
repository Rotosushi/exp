/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "env/constants.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void constants_initialize(Constants *constants) {
    EXP_ASSERT(constants != nullptr);
    constants->count    = 0;
    constants->capacity = 0;
    constants->buffer   = NULL;
}

void constants_terminate(Constants *constants) {
    EXP_ASSERT(constants != NULL);
    for (u32 i = 0; i < constants->count; ++i) {
        Value *constant = constants->buffer + i;
        value_terminate(constant);
    }

    constants->count    = 0;
    constants->capacity = 0;
    deallocate(constants->buffer);
    constants->buffer = NULL;
}

Value *constants_at(Constants *constants, u32 constant) {
    EXP_ASSERT(constants != nullptr);
    EXP_ASSERT(constant < constants->count);
    return constants->buffer + constant;
}

static bool constants_full(Constants *constants) {
    EXP_ASSERT(constants != NULL);
    return (constants->count + 1) >= constants->capacity;
}

static void constants_grow(Constants *constants) {
    EXP_ASSERT(constants != NULL);
    Growth32 g          = array_growth_u32(constants->capacity, sizeof(Value));
    constants->buffer   = reallocate(constants->buffer, g.alloc_size);
    constants->capacity = g.new_capacity;
}

u32 constants_append_tuple(Constants *constants, Tuple tuple) {
    EXP_ASSERT(constants != NULL);

    Value value;
    value_initialize_tuple(&value, tuple);

    for (u32 index = 0; index < constants->count; ++index) {
        Value *cursor = constants->buffer + index;
        EXP_ASSERT(cursor != nullptr);

        if (value_equality(cursor, &value)) {
            value_terminate(&value);
            return index;
        }
    }

    if (constants_full(constants)) { constants_grow(constants); }

    u32 index                = constants->count++;
    constants->buffer[index] = value;

    return index;
}
