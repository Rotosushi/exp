// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_VALUE_H
#define EXP_IMR_VALUE_H
#include <stdbool.h>

#include "imr/function.h"
#include "imr/tuple.h"
#include "support/string.h"

typedef enum ValueKind {
    VALUE_KIND_UNINITIALIZED,

    VALUE_KIND_NIL,
    VALUE_KIND_BOOLEAN,
    VALUE_KIND_U8,
    VALUE_KIND_U16,
    VALUE_KIND_U32,
    VALUE_KIND_U64,
    VALUE_KIND_I8,
    VALUE_KIND_I16,
    VALUE_KIND_I32,
    VALUE_KIND_I64,

    VALUE_KIND_TUPLE,
    VALUE_KIND_FUNCTION,
} ValueKind;

struct Value;

/**
 * @brief represents Values in the compiler
 */
typedef struct Value {
    ValueKind kind;
    union {
        bool     nil;
        bool     boolean;
        u8       u8_;
        u16      u16_;
        u32      u32_;
        u64      u64_;
        i8       i8_;
        i16      i16_;
        i32      i32_;
        i64      i64_;
        Tuple    tuple;
        Function function;
    };
} Value;

Value *value_allocate();
void   value_deallocate(Value *restrict value);

Value *value_allocate_nil();
Value *value_allocate_boolean(bool b);
Value *value_allocate_u8(u8 u);
Value *value_allocate_u16(u16 u);
Value *value_allocate_u32(u32 u);
Value *value_allocate_u64(u64 u);
Value *value_allocate_i8(i8 i);
Value *value_allocate_i16(i16 i);
Value *value_allocate_i32(i32 i);
Value *value_allocate_i64(i64 i);
Value *value_allocate_tuple();
Value *value_allocate_function();

bool value_equal(Value const *v1, Value const *v2);

bool value_is_index(Value const *v);
u64  value_as_index(Value const *v);

struct Context;
void print_value(String *restrict string,
                 Value const *restrict v,
                 struct Context *restrict context);

#endif // !EXP_IMR_VALUE_H
