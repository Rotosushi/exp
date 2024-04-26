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
#include <stdlib.h>

#include "backend/compute_lifetimes.h"

/*
  in order to test that the lifetimes are being computed
  correctly, we have to create a function body where
  we know the lifetimes in advance, then check that
  the lifetimes that were computed match the expected
  lifetimes.

  and ideally we want to not hardcode the expected lifetimes.
  but that seems very difficult without tying the test to the
  implementation of how function bodies are generated.
*/

FunctionBody test_body() {
  FunctionBody test = function_body_create();
  bytecode_emit_move(&test.bc, local(0), immediate(1));      // 0
  bytecode_emit_move(&test.bc, local(1), immediate(1));      // 1
  bytecode_emit_add(&test.bc, local(2), local(0), local(1)); // 2
  bytecode_emit_return(&test.bc, local(2));                  // 3
  return test;
}

static Lifetime lifetime(u16 l, u16 fu, u16 lu) {
  Lifetime r = {.local = l, .first_use = fu, .last_use = lu};
  return r;
}

Lifetimes expected_lifetimes() {
  // hardcoded from the test_body above
  // {%0, 0, 2}, {%1, 1, 2}, {%2, 2, 3}
  Lifetimes test = lifetimes_create();
  lifetimes_insert_sorted(&test, lifetime(0, 0, 2));
  lifetimes_insert_sorted(&test, lifetime(1, 1, 2));
  lifetimes_insert_sorted(&test, lifetime(2, 2, 3));
  return test;
}

static bool lifetimes_match(Lifetimes *restrict expected,
                            Lifetimes *restrict actual) {
  if (expected->size != actual->size) {
    return 0;
  }

  for (u16 i = 0; i < expected->size; ++i) {
    Lifetime e = expected->buffer[i];
    Lifetime a = actual->buffer[i];
    if (e.local != a.local) {
      return 0;
    }

    if (e.first_use != a.first_use) {
      return 0;
    }

    if (e.last_use != a.last_use) {
      return 0;
    }
  }

  return 1;
}

int compute_lifetimes_tests([[maybe_unused]] int argc,
                            [[maybe_unused]] char **argv) {
  FunctionBody body  = test_body();
  Lifetimes expected = expected_lifetimes();

  Lifetimes actual = compute_lifetimes(&body);

  bool failure = !lifetimes_match(&expected, &actual);

  function_body_destroy(&body);
  lifetimes_destroy(&expected);
  lifetimes_destroy(&actual);

  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}