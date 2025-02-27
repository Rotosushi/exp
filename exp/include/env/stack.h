// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_STACK_H
#define EXP_ENV_STACK_H

#include "imr/value.h"

/**
 * @brief Models the stack of the abstract machine.
 */
typedef struct Stack {
    u32 length;
    u32 capacity;
    Value *buffer;
} Stack;

void stack_initialize(Stack *stack);
void stack_terminate(Stack *stack);

Value *stack_top(Stack *stack);
Value *stack_peek(Stack *stack, u32 index);
u32 stack_push(Stack *stack, Value value);
Value stack_pop(Stack *stack);
void stack_pop_n(Stack *stack, u32 n);

#endif // !EXP_ENV_STACK_H
