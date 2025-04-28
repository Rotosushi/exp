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
} ValueKind;

struct Value;

/**
 * @brief represents Values in the compiler
 *
 */
typedef struct Value {
    ValueKind kind;
    union {
        bool  nil;
        bool  boolean;
        u8    u8_;
        u16   u16_;
        u32   u32_;
        u64   u64_;
        i8    i8_;
        i16   i16_;
        i32   i32_;
        i64   i64_;
        Tuple tuple;
    };
} Value;

/**
 * @brief create an uninitialized value
 *
 * @return Value
 */
Value value_create();

void value_destroy(Value *restrict value);

/**
 * @brief create a nil value
 *
 * @return Value
 */
Value value_create_nil();

/**
 * @brief create a boolean value
 *
 * @param b
 * @return Value
 */
Value value_create_boolean(bool b);

/**
 * @brief create an Unsigned 8 bit value
 *
 * @param u
 * @return Value
 */
Value value_create_u8(u8 u);

/**
 * @brief create an Unsigned 16 bit value
 *
 * @param u
 * @return Value
 */
Value value_create_u16(u16 u);

/**
 * @brief create an Unsigned 32 bit value
 *
 * @param u
 * @return Value
 */
Value value_create_u32(u32 u);

/**
 * @brief create an Unsigned 64 bit value
 *
 * @param u
 * @return Value
 */
Value value_create_u64(u64 u);

/**
 * @brief create a Signed 8 bit value
 *
 * @param i
 * @return Value
 */
Value value_create_i8(i8 i);

/**
 * @brief create a Signed 16 bit value
 *
 * @param i
 * @return Value
 */
Value value_create_i16(i16 i);

/**
 * @brief create a Signed 32 bit value
 *
 * @param i
 * @return Value
 */
Value value_create_i32(i32 i);

/**
 * @brief create an Signed 64 bit value
 *
 * @param i
 * @return Value
 */
Value value_create_i64(i64 i);

/**
 * @brief create a Tuple value
 *
 * @param tuple
 * @return Value
 */
Value value_create_tuple(Tuple tuple);

/**
 * @brief equality compares values
 *
 * @param v1
 * @param v2
 * @return true
 * @return false
 */
bool value_equality(Value *v1, Value *v2);

bool value_is_index(Value const *v);
u64  value_as_index(Value const *v);

struct Context;
void print_value(String *restrict string,
                 Value const *restrict v,
                 struct Context *restrict context);

#endif // !EXP_IMR_VALUE_H
