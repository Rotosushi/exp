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
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/unreachable.h"

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
    Growth_u64 g = array_growth_u64(tuple->capacity, sizeof(*tuple->elements));
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

Value value_create_nil() { return (Value){.kind = VALUE_KIND_NIL, .nil = 0}; }

Value value_create_boolean(bool b) {
    return (Value){.kind = VALUE_KIND_BOOLEAN, .boolean = b};
}

Value value_create_u8(u8 u) { return (Value){.kind = VALUE_KIND_U8, .u8_ = u}; }

Value value_create_u16(u16 u) {
    return (Value){.kind = VALUE_KIND_U16, .u16_ = u};
}

Value value_create_u32(u32 u) {
    return (Value){.kind = VALUE_KIND_U32, .u32_ = u};
}

Value value_create_u64(u64 u) {
    return (Value){.kind = VALUE_KIND_U64, .u64_ = u};
}

Value value_create_i8(i8 i) { return (Value){.kind = VALUE_KIND_I8, .i8_ = i}; }

Value value_create_i16(i16 i) {
    return (Value){.kind = VALUE_KIND_I16, .i16_ = i};
}

Value value_create_i32(i32 i) {
    return (Value){.kind = VALUE_KIND_I32, .i32_ = i};
}

Value value_create_i64(i64 i) {
    return (Value){.kind = VALUE_KIND_I64, .i64_ = i};
}

Value value_create_tuple(Tuple tuple) {
    return (Value){.kind = VALUE_KIND_TUPLE, .tuple = tuple};
}

void value_assign(Value *dest, Value *source) {
    if (dest == source) { return; }

    switch (source->kind) {
    // deep copy values with dynamic storage
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
    case VALUE_KIND_UNINITIALIZED: return true;
    case VALUE_KIND_NIL:           return true;

    case VALUE_KIND_BOOLEAN: {
        return A->boolean == B->boolean;
    }

    case VALUE_KIND_U8: {
        return A->u8_ == B->u8_;
    }

    case VALUE_KIND_U16: {
        return A->u16_ == B->u16_;
    }

    case VALUE_KIND_U32: {
        return A->u32_ == B->u32_;
    }

    case VALUE_KIND_U64: {
        return A->u64_ == B->u64_;
    }

    case VALUE_KIND_I8: {
        return A->i8_ == B->i8_;
    }

    case VALUE_KIND_I16: {
        return A->i16_ == B->i16_;
    }

    case VALUE_KIND_I32: {
        return A->i32_ == B->i32_;
    }

    case VALUE_KIND_I64: {
        return A->i64_ == B->i64_;
    }

    case VALUE_KIND_TUPLE: {
        return tuple_equal(&A->tuple, &B->tuple);
    }

    default: EXP_UNREACHABLE();
    }
}

bool value_is_index(const Value *v) {
    switch (v->kind) {
    case VALUE_KIND_U8:
    case VALUE_KIND_U16:
    case VALUE_KIND_U32:
    case VALUE_KIND_U64: return true;
    case VALUE_KIND_I8:  return v->i8_ >= 0;
    case VALUE_KIND_I16: return v->i16_ >= 0;
    case VALUE_KIND_I32: return v->i32_ >= 0;
    case VALUE_KIND_I64: return v->i64_ >= 0;

    default: return false;
    }
}

u64 value_as_index(const Value *v) {
    exp_assert(value_is_index(v));
    switch (v->kind) {
    case VALUE_KIND_U8:  return v->u8_;
    case VALUE_KIND_U16: return v->u16_;
    case VALUE_KIND_U32: return v->u32_;
    case VALUE_KIND_U64: return v->u64_;
    case VALUE_KIND_I8:  return (u64)v->i8_;
    case VALUE_KIND_I16: return (u64)v->i16_;
    case VALUE_KIND_I32: return (u64)v->i32_;
    case VALUE_KIND_I64: return (u64)v->i64_;

    default: EXP_UNREACHABLE();
    }
}

static void print_tuple(String *restrict string,
                        Tuple const *restrict tuple,
                        Context *restrict context) {
    string_append(string, SV("("));
    for (u64 i = 0; i < tuple->size; ++i) {
        print_operand(string, tuple->elements[i], context);

        if (i < (tuple->size - 1)) { string_append(string, SV(", ")); }
    }
    string_append(string, SV(")"));
}

void print_value(String *restrict string,
                 Value const *restrict v,
                 Context *restrict context) {
    switch (v->kind) {
    case VALUE_KIND_UNINITIALIZED:
        string_append(string, SV("uninitialized"));
        break;

    case VALUE_KIND_NIL: string_append(string, SV("()")); break;

    case VALUE_KIND_BOOLEAN: {
        string_append(string, v->boolean ? SV("true") : SV("false"));
        break;
    }
    case VALUE_KIND_U8:  string_append_u64(string, v->u8_); break;
    case VALUE_KIND_U16: string_append_u64(string, v->u16_); break;
    case VALUE_KIND_U32: string_append_u64(string, v->u32_); break;
    case VALUE_KIND_U64: string_append_u64(string, v->u64_); break;
    case VALUE_KIND_I8:  string_append_i64(string, v->i8_); break;
    case VALUE_KIND_I16: string_append_i64(string, v->i16_); break;
    case VALUE_KIND_I32: string_append_i64(string, v->i32_); break;
    case VALUE_KIND_I64: string_append_i64(string, v->i64_); break;

    case VALUE_KIND_TUPLE: print_tuple(string, &v->tuple, context); break;

    default: EXP_UNREACHABLE();
    }
}
