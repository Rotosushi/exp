/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "env/stack.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

Stack stack_create() {
  Stack stack;
  stack.capacity = 0;
  stack.buffer   = NULL;
  stack.top      = NULL;
  return stack;
}

void stack_destroy(Stack *restrict stack) {
  assert(stack != NULL);
  stack->capacity = 0;
  free(stack->buffer);
  stack->buffer = NULL;
  stack->top    = NULL;
}

bool stack_empty(Stack const *restrict stack) {
  assert(stack != NULL);
  return stack->top == stack->buffer;
}

static bool stack_full(Stack *restrict stack) {
  return stack->top == (stack->buffer + stack->capacity);
}

static void stack_grow(Stack *restrict stack) {
  Growth g = array_growth(stack->capacity, sizeof(Value *));
  // since stack->top >= stack->buffer, this subtraction
  // is always positive, making the cast lossless.
  u64 top_offset = (u64)(stack->top - stack->buffer);

  stack->buffer   = reallocate(stack->buffer, g.alloc_size);
  stack->top      = stack->buffer + top_offset;
  stack->capacity = g.new_capacity;
}

void stack_push(Stack *restrict stack, Value value) {
  assert(stack != NULL);
  if (stack_full(stack)) {
    stack_grow(stack);
  }

  *(stack->top++) = value;
}

Value stack_pop(Stack *restrict stack) {
  assert(stack != NULL);
  Value *result = stack_peek(stack);
  stack->top -= 1;
  return *result;
}

Value *stack_peek(Stack *restrict stack) {
  assert(stack != NULL);
  return stack->top - 1;
}
