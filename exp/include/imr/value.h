// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_VALUE_H
#define EXP_IMR_VALUE_H
#include <stdbool.h>

#include "imr/operand.h"

typedef enum ValueKind {
    VALUE_KIND_UNINITIALIZED,

    //  VALUE_KIND_NIL,
    //  VALUE_KIND_BOOLEAN,
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
        //        bool nil;
        //        bool boolean;
        i32 i32_;
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
void value_initialize_i32(Value *value, i32 i32_);
void value_initialize_tuple(Value *value, Tuple tuple);
// void value_assign(Value *target, Value *source);
bool value_equality(Value *A, Value *B);

struct Context;
void print_value(String *buffer, Value const *value, struct Context *context);

#endif // !EXP_IMR_VALUE_H
