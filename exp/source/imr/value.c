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

/**
 * @file imr/value.c
 */

#include "imr/value.h"
#include "env/context.h"
#include "imr/scalar.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void tuple_initialize(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    tuple->capacity = 0;
    tuple->length   = 0;
    tuple->elements = nullptr;
}

void tuple_terminate(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    deallocate(tuple->elements);
    tuple->elements = nullptr;
    tuple->capacity = 0;
    tuple->length   = 0;
}
/*
void tuple_assign(Tuple *A, Tuple *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    tuple_terminate(A);
    A->capacity = B->capacity;
    A->size     = B->size;
    A->elements = callocate(A->capacity, sizeof(*A->elements));

    memcpy(A->elements, B->elements, A->size);
}
*/

bool tuple_equal(Tuple *A, Tuple *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A->length != B->length) { return 0; }

    for (u32 index = 0; index < A->length; ++index) {
        if (!value_equal(A->elements + index, B->elements + index)) {
            return 0;
        }
    }

    return 1;
}

static bool tuple_full(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    return (tuple->length + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    Growth32 g = array_growth_u32(tuple->capacity, sizeof(*tuple->elements));
    tuple->elements = reallocate(tuple->elements, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *tuple, Value element) {
    EXP_ASSERT(tuple != nullptr);
    if (tuple_full(tuple)) { tuple_grow(tuple); }

    tuple->elements[tuple->length++] = element;
}

void value_initialize(Value *value) {
    EXP_ASSERT(value != nullptr);
    value->kind   = VALUE_UNINITIALIZED;
    value->scalar = scalar_uninitialized();
}

void value_terminate(Value *value) {
    EXP_ASSERT(value != nullptr);
    switch (value->kind) {
    case VALUE_TUPLE: {
        tuple_terminate(&value->tuple);
        break;
    }

    // values without dynamic storage
    default: return;
    }
}

void value_initialize_scalar(Value *value, Scalar scalar) {
    EXP_ASSERT(value != nullptr);
    value->kind   = VALUE_SCALAR;
    value->scalar = scalar;
}

void value_initialize_tuple(Value *value, Tuple tuple) {
    EXP_ASSERT(value != nullptr);
    value->kind  = VALUE_TUPLE;
    value->tuple = tuple;
}

/*
void value_assign(Value *target, Value *source) {
    if (target == source) { return; }

    switch (source->kind) {
    case VALUE_KIND_TUPLE: {
        value_terminate(target);
        *target = (Value){.kind = VALUE_KIND_TUPLE, .tuple =
tuple_initialize()}; tuple_assign(&target->tuple, &source->tuple); break;
    }

    // values without dynamic storage can be struct assigned.
    default: {
        *target = *source;
        break;
    }
    }
}
*/

bool value_equal(Value *A, Value *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A == B) { return true; }
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case VALUE_UNINITIALIZED: return false;

    case VALUE_SCALAR: {
        return scalar_equal(A->scalar, B->scalar);
    }

    case VALUE_TUPLE: {
        return tuple_equal(&A->tuple, &B->tuple);
    }

    default: EXP_UNREACHABLE();
    }
}

static void print_tuple(String *buffer, Tuple const *tuple) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(tuple != nullptr);
    string_append(buffer, SV("("));
    for (u32 i = 0; i < tuple->length; ++i) {
        print_value(buffer, tuple->elements + i);

        if (i < (tuple->length - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));
}

void print_value(String *buffer, Value const *value) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(value != nullptr);
    switch (value->kind) {
    case VALUE_UNINITIALIZED: string_append(buffer, SV("uninitialized")); break;
    case VALUE_SCALAR:        print_scalar(buffer, value->scalar); break;
    case VALUE_TUPLE:         print_tuple(buffer, &value->tuple); break;

    default: EXP_UNREACHABLE();
    }
}
