/**
 * Copyright (C) 2025 Cade Weinberg
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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "env/stack.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void stack_create(Stack *restrict stack) {
    exp_assert(stack != NULL);
    stack->size     = 0;
    stack->capacity = 0;
    stack->buffer   = NULL;
}

void stack_destroy(Stack *restrict stack) {
    exp_assert(stack != NULL);
    deallocate(stack->buffer);
    stack_create(stack);
}

bool stack_empty(Stack const *restrict stack) {
    exp_assert(stack != NULL);
    return stack->size == 0;
}

u32 stack_size(Stack const *restrict stack) {
    exp_assert(stack != NULL);
    return stack->size;
}

static bool stack_full(Stack const *restrict stack) {
    return (stack->size + 1) >= stack->capacity;
}

static void stack_grow(Stack *restrict stack) {
    Growth_u32 g    = array_growth_u32(stack->capacity, sizeof(*stack->buffer));
    stack->buffer   = reallocate(stack->buffer, g.alloc_size);
    stack->capacity = g.new_capacity;
}

void stack_push(Stack *restrict stack, Value const *value) {
    exp_assert(stack != NULL);
    exp_assert(value != NULL);

    if (stack_full(stack)) { stack_grow(stack); }

    stack->buffer[stack->size++] = value;
}

Value const *stack_pop(Stack *restrict stack) {
    exp_assert(stack != NULL);
    exp_assert(!stack_empty(stack));
    return stack->buffer[--stack->size];
}

void stack_pop_n(Stack *restrict stack, u32 n) {
    exp_assert(stack != NULL);
    exp_assert(stack->size >= n);
    stack->size -= n;
}

Value const *stack_peek(Stack const *restrict stack, u32 index) {
    exp_assert(stack != NULL);
    exp_assert(stack->size >= index);
    return stack->buffer[index];
}
