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

void tuple_destroy(Tuple *restrict tuple) {
    deallocate(tuple->elements);
    tuple->elements = NULL;
    tuple->capacity = 0;
    tuple->size     = 0;
}

void tuple_assign(Tuple *restrict A, Tuple *restrict B) {
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

static bool tuple_full(Tuple *restrict tuple) {
    return (tuple->size + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *restrict tuple) {
    Growth64 g = array_growth_u64(tuple->capacity, sizeof(*tuple->elements));
    tuple->elements = reallocate(tuple->elements, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *restrict tuple, Operand element) {
    if (tuple_full(tuple)) { tuple_grow(tuple); }

    tuple->elements[tuple->size] = element;
    tuple->size += 1;
}

Value value_create() {
    Value value = {.kind = VALUE_KIND_UNINITIALIZED, .nil = 0};
    return value;
}

void value_destroy(Value *restrict value) {
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

Value value_create_i64(i64 i) {
    Value value = {.kind = VALUE_KIND_I64, .i64_ = i};
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
    if (A == B) { return 1; }

    switch (A->kind) {
    case VALUE_KIND_UNINITIALIZED: return B->kind == VALUE_KIND_UNINITIALIZED;
    case VALUE_KIND_NIL:           return B->kind == VALUE_KIND_NIL;

    case VALUE_KIND_BOOLEAN: {
        if (B->kind != VALUE_KIND_BOOLEAN) { return 0; }

        return A->boolean == B->boolean;
    }

    case VALUE_KIND_I64: {
        if (B->kind != VALUE_KIND_I64) { return 0; }

        return A->i64_ == B->i64_;
    }

    case VALUE_KIND_TUPLE: {
        return tuple_equal(&A->tuple, &B->tuple);
    }

    default: EXP_UNREACHABLE();
    }
}

static void print_tuple(Tuple const *restrict tuple,
                        FILE *restrict file,
                        Context *restrict context) {
    file_write("(", file);
    for (u64 i = 0; i < tuple->size; ++i) {
        print_operand(tuple->elements[i], file, context);

        if (i < (tuple->size - 1)) { file_write(", ", file); }
    }
    file_write(")", file);
}

void print_value(Value const *restrict v,
                 FILE *restrict file,
                 Context *restrict context) {
    switch (v->kind) {
    case VALUE_KIND_UNINITIALIZED:
    case VALUE_KIND_NIL:           file_write("()", file); break;

    case VALUE_KIND_BOOLEAN: {
        (v->boolean) ? file_write("true", file) : file_write("false", file);
        break;
    }

    case VALUE_KIND_I64:   file_write_i64(v->i64_, file); break;
    case VALUE_KIND_TUPLE: print_tuple(&v->tuple, file, context); break;

    default: EXP_UNREACHABLE();
    }
}
