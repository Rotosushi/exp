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
#include <string.h>

#include "env/context.h"
#include "imr/value.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

Tuple tuple_create() {
    Tuple tuple = {.capacity = 0, .size = 0, .elements = NULL};
    return tuple;
}

void tuple_destroy(Tuple *tuple) {
    deallocate(tuple->elements);
    tuple->elements = NULL;
    tuple->capacity = 0;
    tuple->size     = 0;
}

void tuple_assign(Tuple *A, Tuple *B) {
    tuple_destroy(A);
    A->capacity = B->capacity;
    A->size     = B->size;
    A->elements = callocate(A->capacity, sizeof(*A->elements));

    memcpy(A->elements, B->elements, A->size);
}

bool tuple_equal(Tuple *A, Tuple *B) {
    if (A->size != B->size) { return 0; }

    for (u64 i = 0; i < A->size; ++i) {
        if (!operand_equality(A->elements[i], B->elements[i])) { return 0; }
    }

    return 1;
}

static bool tuple_full(Tuple *tuple) {
    return (tuple->size + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *tuple) {
    Growth32 g = array_growth_u32(tuple->capacity, sizeof(*tuple->elements));
    tuple->elements = reallocate(tuple->elements, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *tuple, Operand element) {
    if (tuple_full(tuple)) { tuple_grow(tuple); }

    tuple->elements[tuple->size] = element;
    tuple->size += 1;
}

Value value_create() {
    Value value = {.kind = VALUE_KIND_UNINITIALIZED, .nil = 0};
    return value;
}

void value_destroy(Value *value) {
    switch (value->kind) {
    case VALUE_KIND_TUPLE: {
        tuple_destroy(&value->tuple);
        break;
    }

    // values without dynamic storage
    default: return;
    }
}

Value value_create_nil() {
    Value value = {.kind = VALUE_KIND_NIL, .nil = 0};
    return value;
}

Value value_create_boolean(bool b) {
    Value value = {.kind = VALUE_KIND_BOOLEAN, .boolean = b};
    return value;
}

Value value_create_i32(i32 i) {
    Value value = {.kind = VALUE_KIND_I32, .i32_ = i};
    return value;
}

Value value_create_tuple(Tuple tuple) {
    Value value = {.kind = VALUE_KIND_TUPLE, .tuple = tuple};
    return value;
}

void value_assign(Value *dest, Value *source) {
    if (dest == source) { return; }

    switch (source->kind) {
    case VALUE_KIND_TUPLE: {
        value_destroy(dest);
        *dest = (Value){.kind = VALUE_KIND_TUPLE, .tuple = tuple_create()};
        tuple_assign(&dest->tuple, &source->tuple);
        break;
    }

    // values without dynamic storage can be struct assigned.
    default: {
        *dest = *source;
        break;
    }
    }
}

bool value_equality(Value *A, Value *B) {
    if (A == B) { return true; }
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case VALUE_KIND_UNINITIALIZED:
    case VALUE_KIND_NIL:           return true;

    case VALUE_KIND_BOOLEAN: {
        return A->boolean == B->boolean;
    }

    case VALUE_KIND_I32: {
        return A->i32_ == B->i32_;
    }

    case VALUE_KIND_TUPLE: {
        return tuple_equal(&A->tuple, &B->tuple);
    }

    default: EXP_UNREACHABLE();
    }
}

static void print_tuple(String *buffer, Tuple const *tuple, Context *context) {
    string_append(buffer, SV("("));
    for (u64 i = 0; i < tuple->size; ++i) {
        Operand element = tuple->elements[i];
        print_operand(buffer, element.kind, element.data, context);

        if (i < (tuple->size - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));
}

void print_value(String *buffer, Value const *v, Context *context) {
    switch (v->kind) {
    case VALUE_KIND_UNINITIALIZED:
        string_append(buffer, SV("uninitialized"));
        break;
    case VALUE_KIND_NIL: string_append(buffer, SV("()")); break;

    case VALUE_KIND_BOOLEAN: {
        if (v->boolean) string_append(buffer, SV("true"));
        else string_append(buffer, SV("false"));
        break;
    }

    case VALUE_KIND_I32:   string_append_i64(buffer, v->i32_); break;
    case VALUE_KIND_TUPLE: print_tuple(buffer, &v->tuple, context); break;

    default: EXP_UNREACHABLE();
    }
}
