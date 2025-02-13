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
// #include <EXP_ASSERT.h>
// #include <string.h>

#include "imr/value.h"
#include "env/context.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void tuple_initialize(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    tuple->capacity = 0;
    tuple->size     = 0;
    tuple->elements = nullptr;
}

void tuple_terminate(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    deallocate(tuple->elements);
    tuple->elements = nullptr;
    tuple->capacity = 0;
    tuple->size     = 0;
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
    if (A->size != B->size) { return 0; }

    for (u32 i = 0; i < A->size; ++i) {
        if (!operand_equality(A->elements[i], B->elements[i])) { return 0; }
    }

    return 1;
}

static bool tuple_full(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    return (tuple->size + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *tuple) {
    EXP_ASSERT(tuple != nullptr);
    Growth32 g = array_growth_u32(tuple->capacity, sizeof(*tuple->elements));
    tuple->elements = reallocate(tuple->elements, g.alloc_size);
    tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *tuple, Operand element) {
    EXP_ASSERT(tuple != nullptr);
    if (tuple_full(tuple)) { tuple_grow(tuple); }

    tuple->elements[tuple->size] = element;
    tuple->size += 1;
}

void value_initialize(Value *value) {
    EXP_ASSERT(value != nullptr);
    value->kind = VALUE_KIND_UNINITIALIZED;
    //    value->nil  = 0;
}

void value_terminate(Value *value) {
    EXP_ASSERT(value != nullptr);
    switch (value->kind) {
    case VALUE_KIND_TUPLE: {
        tuple_terminate(&value->tuple);
        break;
    }

    // values without dynamic storage
    default: return;
    }
}

/*
void value_initialize_nil(Value *value) {
    EXP_ASSERT(value != nullptr);
    value->kind = VALUE_KIND_NIL;
    value->nil  = 0;
}

void value_initialize_boolean(Value *value, bool bool_) {
    EXP_ASSERT(value != nullptr);
    value->kind    = VALUE_KIND_BOOLEAN;
    value->boolean = bool_;
}
*/

void value_initialize_i32(Value *value, i32 i32_) {
    EXP_ASSERT(value != nullptr);
    value->kind = VALUE_KIND_I32;
    value->i32_ = i32_;
}

void value_initialize_tuple(Value *value, Tuple tuple) {
    EXP_ASSERT(value != nullptr);
    value->kind  = VALUE_KIND_TUPLE;
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

bool value_equality(Value *A, Value *B) {
    EXP_ASSERT(A != nullptr);
    EXP_ASSERT(B != nullptr);
    if (A == B) { return true; }
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case VALUE_KIND_UNINITIALIZED: return true;

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
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(tuple != nullptr);
    EXP_ASSERT(context != nullptr);
    string_append(buffer, SV("("));
    for (u32 i = 0; i < tuple->size; ++i) {
        Operand element = tuple->elements[i];
        print_operand(buffer, element.kind, element.data, context);

        if (i < (tuple->size - 1)) { string_append(buffer, SV(", ")); }
    }
    string_append(buffer, SV(")"));
}

void print_value(String *buffer, Value const *value, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(value != nullptr);
    EXP_ASSERT(context != nullptr);
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED:
        string_append(buffer, SV("uninitialized"));
        break;

        /*
            case VALUE_KIND_NIL: string_append(buffer, SV("()")); break;

        case VALUE_KIND_BOOLEAN: {
            if (v->boolean) string_append(buffer, SV("true"));
            else string_append(buffer, SV("false"));
            break;
        }
            */

    case VALUE_KIND_I32:   string_append_i64(buffer, value->i32_); break;
    case VALUE_KIND_TUPLE: print_tuple(buffer, &value->tuple, context); break;

    default: EXP_UNREACHABLE();
    }
}
