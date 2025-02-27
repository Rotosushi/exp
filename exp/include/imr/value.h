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

typedef enum ValueKind {
    VALUE_KIND_UNINITIALIZED,

    //  VALUE_KIND_NIL,
    //  VALUE_KIND_BOOLEAN,
    VALUE_KIND_I64,

    VALUE_KIND_TUPLE,
} ValueKind;

struct Value;

typedef struct Tuple {
    u64 size;
    u64 capacity;
    Operand *elements;
} Tuple;

/**
 * @brief represents Values in the compiler
 *
 */
typedef struct Value {
    ValueKind kind;
    union {
        //        bool nil;
        //        bool boolean;
        i64 i64_;
        Tuple tuple;
    };
} Value;

void tuple_initialize(Tuple *tuple);
void tuple_terminate(Tuple *tuple);
// void tuple_assign(Tuple *A, Tuple *B);
bool tuple_equal(Tuple *A, Tuple *B);
void tuple_append(Tuple *tuple, Operand element);

void value_initialize(Value *value);
void value_terminate(Value *value);
// void value_initialize_nil(Value *value);
// void value_initialize_boolean(Value *value, bool bool_);
void value_initialize_i64(Value *value, i64 i64_);
void value_initialize_tuple(Value *value, Tuple tuple);
// void value_assign(Value *target, Value *source);
bool value_equality(Value *A, Value *B);

struct Context;
void print_value(String *buffer, Value const *value, struct Context *context);

#endif // !EXP_IMR_VALUE_H
