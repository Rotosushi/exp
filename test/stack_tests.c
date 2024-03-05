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
#include <stdlib.h>
#include <time.h>

#include "imr/stack.h"

int stack_tests([[maybe_unused]] int argc,
                [[maybe_unused]] char const *argv[]) {
  srand((unsigned)time(NULL));
  Stack stack = stack_create();
  bool failed = 0;

  failed |= !stack_empty(&stack);

  Value i0 = value_create_integer(rand());
  stack_push(&stack, i0);
  Value s0 = stack_peek(&stack);
  failed |= !value_equality(&i0, &s0);

  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));
  stack_push(&stack, value_create_integer(rand()));

  Value i1 = value_create_integer(rand());
  stack_push(&stack, i1);

  Value s1 = stack_pop(&stack);
  failed |= !value_equality(&i1, &s1);

  stack_pop(&stack);
  stack_pop(&stack);
  stack_pop(&stack);
  stack_pop(&stack);
  stack_pop(&stack);
  stack_pop(&stack);
  stack_pop(&stack);
  Value s2 = stack_pop(&stack);
  failed |= !value_equality(&i0, &s2);

  stack_destroy(&stack);
  if (failed)
    return EXIT_FAILURE;
  else
    return EXIT_SUCCESS;
}