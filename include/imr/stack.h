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
#ifndef EXP_IMR_STACK_H
#define EXP_IMR_STACK_H
#include <stddef.h>

#include "imr/value.h"

typedef struct Stack {
  size_t capacity;
  Value **buffer;
  Value **top;
} Stack;

/**
 * @brief create a new stack
 *
 * @return Stack
 */
Stack stack_create();

/**
 * @brief destroy a stack
 *
 * @param stack
 */
void stack_destroy(Stack *restrict stack);

/**
 * @brief check if the stack is empty
 *
 * @param stack
 * @return true
 * @return false
 */
bool stack_empty(Stack const *restrict stack);

/**
 * @brief push a value onto the stack
 *
 * @param stack
 * @param value
 */
void stack_push(Stack *restrict stack, Value *value);

/**
 * @brief pop the top value off the stack
 *
 * @param stack
 * @return Value
 */
Value *stack_pop(Stack *restrict stack);

/**
 * @brief peek at the top value of the stack
 *
 * @param stack
 * @return Value*
 */
Value *stack_peek(Stack *restrict stack);

#endif // !EXP_IMR_STACK_H