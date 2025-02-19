/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "env/stack.h"
#include "imr/value.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void stack_initialize(Stack *stack) {
    EXP_ASSERT(stack != nullptr);
    stack->length   = 0;
    stack->capacity = 0;
    stack->buffer   = nullptr;
}

void stack_terminate(Stack *stack) {
    EXP_ASSERT(stack != nullptr);

    for (u32 index = 0; index < stack->length; ++index) {
        value_terminate(&stack->buffer[index]);
    }

    deallocate(stack->buffer);
    stack_initialize(stack);
}

Value *stack_top(Stack *stack) {
    EXP_ASSERT(stack != nullptr);
    if (stack->length == 0) { return nullptr; }
    return &stack->buffer[stack->length - 1];
}

Value *stack_peek(Stack *stack, u32 index) {
    EXP_ASSERT(stack != nullptr);
    EXP_ASSERT(index < stack->length);
    if (stack->length == 0) { return nullptr; }
    return &stack->buffer[index];
}

static bool stack_full(Stack *stack) {
    EXP_ASSERT(stack != nullptr);
    return (stack->length + 1) >= stack->capacity;
}

static void stack_grow(Stack *stack) {
    EXP_ASSERT(stack != nullptr);
    Growth32 g      = array_growth_u32(stack->capacity, sizeof(*stack->buffer));
    stack->buffer   = reallocate(stack->buffer, g.alloc_size);
    stack->capacity = g.new_capacity;
}

u32 stack_push(Stack *stack, Value value) {
    EXP_ASSERT(stack != nullptr);
    if (stack_full(stack)) { stack_grow(stack); }

    u32 index            = stack->length++;
    stack->buffer[index] = value;
    return index;
}

Value stack_pop(Stack *stack) {
    EXP_ASSERT(stack != nullptr);
    EXP_ASSERT(stack->length > 0);
    return stack->buffer[--stack->length];
}

void stack_pop_n(Stack *stack, u32 n) {
    EXP_ASSERT(stack != nullptr);
    EXP_ASSERT(stack->length >= n);
    for (u32 i = 0; i < n; ++i) {
        value_terminate(&stack->buffer[stack->length - (i - 1)]);
    }
    stack->length -= n;
}
