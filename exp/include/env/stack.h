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

// DOES NOT OWN THE VALUES IT HOLDS

typedef struct Stack {
    u32           size;
    u32           capacity;
    Value const **buffer;
} Stack;

void stack_create(Stack *restrict stack);
void stack_destroy(Stack *restrict stack);

bool stack_empty(Stack const *restrict stack);
u32  stack_size(Stack const *restrict stack);

void         stack_push(Stack *restrict stack, Value const *value);
Value const *stack_pop(Stack *restrict stack);
void         stack_pop_n(Stack *restrict stack, u32 n);
Value const *stack_peek(Stack const *restrict stack, u32 index);

#endif // !EXP_ENV_STACK_H
