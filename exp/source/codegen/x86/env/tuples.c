/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/env/tuples.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void x86_tuples_create(x86_Tuples *restrict tuples) {
    exp_assert(tuples != NULL);
    tuples->size     = 0;
    tuples->capacity = 0;
    tuples->buffer   = NULL;
}

void x86_tuples_destroy(x86_Tuples *restrict tuples) {
    exp_assert(tuples != NULL);

    for (u32 index = 0; index < tuples->size; ++index) {
        x86_Tuple *tuple = tuples->buffer + index;
        x86_tuple_layout_destroy(&tuple->layout);
    }

    deallocate(tuples->buffer);
    x86_tuples_create(tuples);
}

static bool x86_tuples_full(x86_Tuples const *restrict tuples) {
    return (tuples->size + 1) >= tuples->capacity;
}

static void x86_tuples_grow(x86_Tuples *restrict tuples) {
    Growth_u32 g = array_growth_u32(tuples->capacity, sizeof(*tuples->buffer));
    tuples->buffer   = reallocate(tuples->buffer, g.alloc_size);
    tuples->capacity = g.new_capacity;
}

x86_Tuple *x86_tuples_lookup(x86_Tuples *restrict tuples, Type const *type) {
    exp_assert(tuples != NULL);
    exp_assert(type != NULL);
    exp_assert(type->kind == TYPE_KIND_TUPLE);

    for (u32 index = 0; index < tuples->size; ++index) {
        x86_Tuple *tuple = tuples->buffer + index;
        if (tuple->type == type) { return tuple; }
    }

    if (x86_tuples_full(tuples)) { x86_tuples_grow(tuples); }

    x86_Tuple *tuple = tuples->buffer + tuples->size++;
    tuple->type      = type;
    x86_tuple_layout_create(&tuple->layout, &type->tuple_type);
    return tuple;
}
