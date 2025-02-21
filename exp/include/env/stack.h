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
 * @file env/stack.h
 */

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
