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

#include "imr/stack.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Stack stack_create() {
  Stack stack;
  stack.capacity = 0;
  stack.buffer = NULL;
  stack.top = NULL;
  return stack;
}

void stack_destroy(Stack *restrict stack) {
  assert(stack != NULL);
  stack->capacity = 0;
  free(stack->buffer);
  stack->buffer = NULL;
  stack->top = NULL;
}

bool stack_empty(Stack const *restrict stack) {
  assert(stack != NULL);
  return stack->top == stack->buffer;
}

static bool stack_full(Stack *restrict stack) {
  assert(stack != NULL);
  return stack->top == (stack->buffer + stack->capacity);
}

static void stack_grow(Stack *restrict stack) {
  assert(stack != NULL);
  assert(stack->capacity != SIZE_MAX && "cannot allocate more than SIZE_MAX");
  size_t new_capacity = nearest_power_of_two(stack->capacity + 1);

  size_t new_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Value), &new_size)) {
    panic("cannot allocate more than SIZE_MAX");
  }

  // since stack->top >= stack->buffer, this subtraction
  // is always positive, making the cast lossless.
  size_t top_offset = (size_t)(stack->top - stack->buffer);

  Value *buffer = realloc(stack->buffer, new_size);
  if (buffer == NULL) {
    panic_errno("realloc failed");
  }

  stack->buffer = buffer;
  stack->top = stack->buffer + top_offset;
  stack->capacity = new_capacity;
}

void stack_push(Stack *restrict stack, Value value) {
  assert(stack != NULL);
  if (stack_full(stack)) {
    stack_grow(stack);
  }

  *(stack->top) = value;
  stack->top += 1;
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
