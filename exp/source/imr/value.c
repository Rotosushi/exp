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

#include "imr/value.h"
#include "env/context.h"
#include "support/allocation.h"
#include "support/assert.h"
#include "support/unreachable.h"

Value *value_allocate() {
    Value *value = callocate(1, sizeof(Value));
    return value;
}

void value_deallocate(Value *restrict value) {
    switch (value->kind) {
    case VALUE_KIND_TUPLE: {
        tuple_destroy(&value->tuple);
        break;
    }

    case VALUE_KIND_FUNCTION: {
        function_destroy(&value->function);
    }

    // values without dynamic storage
    default: break;
    }

    deallocate(value);
}

Value *value_allocate_nil() {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_NIL;
    value->nil   = 0;
    return value;
}

Value *value_allocate_boolean(bool b) {
    Value *value   = value_allocate();
    value->kind    = VALUE_KIND_BOOLEAN;
    value->boolean = b;
    return value;
}

Value *value_allocate_u8(u8 u) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_U8;
    value->u8_   = u;
    return value;
}

Value *value_allocate_u16(u16 u) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_U16;
    value->u16_  = u;
    return value;
}

Value *value_allocate_u32(u32 u) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_U32;
    value->u32_  = u;
    return value;
}

Value *value_allocate_u64(u64 u) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_U64;
    value->u64_  = u;
    return value;
}

Value *value_allocate_i8(i8 i) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_I8;
    value->i8_   = i;
    return value;
}

Value *value_allocate_i16(i16 i) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_I16;
    value->i16_  = i;
    return value;
}

Value *value_allocate_i32(i32 i) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_I32;
    value->i32_  = i;
    return value;
}

Value *value_allocate_i64(i64 i) {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_I64;
    value->i64_  = i;
    return value;
}

Value *value_allocate_tuple() {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_TUPLE;
    tuple_create(&value->tuple);
    return value;
}

Value *value_allocate_function() {
    Value *value = value_allocate();
    value->kind  = VALUE_KIND_FUNCTION;
    function_create(&value->function);
    return value;
}

bool value_equal(Value const *A, Value const *B) {
    exp_assert(A != NULL);
    exp_assert(B != NULL);
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
