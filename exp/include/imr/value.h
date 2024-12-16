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

#include "imr/operand.h"
#include "utility/io.h"

typedef enum ValueKind {
    VALUE_KIND_UNINITIALIZED,

    VALUE_KIND_NIL,
    VALUE_KIND_BOOLEAN,
    VALUE_KIND_I32,

    VALUE_KIND_TUPLE,
} ValueKind;

struct Value;

typedef struct Tuple {
    u32 size;
    u32 capacity;
    Operand *elements;
} Tuple;

/**
 * @brief represents Values in the compiler
 *
 */
typedef struct Value {
    ValueKind kind;
    union {
        bool nil;
        bool boolean;
        i32 i32_;
        Tuple tuple;
    };
} Value;

Tuple tuple_create();
void tuple_destroy(Tuple *restrict tuple);
void tuple_assign(Tuple *restrict A, Tuple *restrict B);
bool tuple_equal(Tuple *A, Tuple *B);
void tuple_append(Tuple *restrict tuple, Operand element);

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
 * @brief create an Integer value
 *
 * @param i
 * @return Value
 */
Value value_create_i32(i32 i);

/**
 * @brief create a Tuple value
 *
 * @param tuple
 * @return Value
 */
Value value_create_tuple(Tuple tuple);

/**
 * @brief assign dest the value of source
 *
 * @param dest
 * @param source
 */
void value_assign(Value *dest, Value *source);

/**
 * @brief equality compares values
 *
 * @param v1
 * @param v2
 * @return true
 * @return false
 */
bool value_equality(Value *v1, Value *v2);

struct Context;
void print_value(Value const *restrict v,
                 FILE *restrict file,
                 struct Context *restrict context);

#endif // !EXP_IMR_VALUE_H
