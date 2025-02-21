// Copyright (C) 2025 Cade Weinberg
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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file imr/value.h
 */

#ifndef EXP_IMR_VALUE_H
#define EXP_IMR_VALUE_H
#include <stdbool.h>

#include "imr/scalar.h"

typedef enum ValueKind {
    VALUE_UNINITIALIZED,
    VALUE_SCALAR,
    VALUE_TUPLE,
} ValueKind;

struct Value;

typedef struct Tuple {
    u32 length;
    u32 capacity;
    struct Value *elements;
} Tuple;

/**
 * @brief represents Values in the compiler
 *
 */
typedef struct Value {
    ValueKind kind;
    union {
        Scalar scalar;
        Tuple tuple;
    };
} Value;

void tuple_initialize(Tuple *tuple);
void tuple_terminate(Tuple *tuple);
// void tuple_assign(Tuple *A, Tuple *B);
bool tuple_equal(Tuple *A, Tuple *B);
void tuple_append(Tuple *tuple, Value element);

void value_initialize(Value *value);
void value_terminate(Value *value);
void value_initialize_scalar(Value *value, Scalar scalar);
void value_initialize_tuple(Value *value, Tuple tuple);
// void value_assign(Value *target, Value *source);
bool value_equal(Value *A, Value *B);

void print_value(String *buffer, Value const *value);

#endif // !EXP_IMR_VALUE_H
