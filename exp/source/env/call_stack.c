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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "env/call_stack.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

CallStack call_stack_create() {
  CallStack cs = {.capacity = 0, .stack = NULL, .top = NULL};
  return cs;
}

void call_stack_destroy(CallStack *restrict cs) {
  cs->capacity = 0;
  cs->top      = NULL;
  free(cs->stack);
  cs->stack = NULL;
}

bool call_stack_empty(CallStack *restrict cs) {
  assert(cs != NULL);
  return cs->top == cs->stack;
}

static bool call_stack_full(CallStack *restrict cs) {
  return (cs->top + 1) == (cs->stack + cs->capacity);
}

static void call_stack_grow(CallStack *restrict cs) {
  Growth g     = array_growth(cs->capacity, sizeof(CallFrame));
  u64 offset   = (u64)(cs->top - cs->stack);
  cs->stack    = reallocate(cs->stack, g.alloc_size);
  cs->top      = cs->stack + offset;
  cs->capacity = g.new_capacity;
}

CallFrame call_stack_push(CallStack *restrict cs, FunctionBody *fn,
                          Frame frame) {
  assert(cs != NULL);
  assert(fn != NULL);
  assert(frame != NULL);

  if (call_stack_full(cs)) {
    call_stack_grow(cs);
  }

  *(cs->top) = (CallFrame){.function = fn, .frame = frame};
  return *(cs->top++);
}

CallFrame call_stack_pop(CallStack *restrict cs) {
  assert(cs != NULL);
  assert(!call_stack_empty(cs));

  return *(--cs->top);
}

CallFrame call_stack_top(CallStack *restrict cs) {
  assert(cs != NULL);
  assert(!call_stack_empty(cs));

  return cs->top[-1];
}