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

#include "adt/string.h"
#include "frontend/token.h"
#include "test_exp.h"
#include "utility/config.h"
#include "utility/unreachable.h"

typedef struct Pair {
  u64 a;
  u64 b;
} Pair;

static Pair pair(u64 a, u64 b) {
  Pair result = {.a = a, .b = b};
  return result;
}

static bool in_range(u64 value) { return (value >= 0) && (value <= 100); }

static u64 gen() { return ((u64)rand() % 100); }

static Pair generate() {
  while (true) {
    Pair result = pair(gen(), gen());
    if (in_range(result.a * result.b)) { return result; }
  }
}

static StringView token_to_view(Token token) {
  switch (token) {
  case TOK_PLUS:    return SV("+");
  case TOK_MINUS:   return SV("-");
  case TOK_STAR:    return SV("*");
  case TOK_SLASH:   return SV("/");
  case TOK_PERCENT: return SV("%");

  default: EXP_UNREACHABLE;
  }
}

static void emit_test(Token op, String *restrict buffer) {
  Pair num = generate();
  string_append(buffer, SV("const x = "));
  string_append_u64(buffer, num.a);
  string_append(buffer, SV("; const y = "));
  string_append_u64(buffer, num.b);
  string_append(buffer, SV("; fn main() { return x "));
  string_append(buffer, token_to_view(op));
  string_append(buffer, SV(" y; }"));
}

int global_constant([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  StringView source_path = SV(EXP_TEST_DIR "/global_constant.exp");

  result |= test_exp(source_path, "const x = 1; fn main() { return x; }", 1);

  result |=
      test_exp(source_path, "const x = 1; fn main() { return x + x; }", 2);

  result |=
      test_exp(source_path, "const x = 2; fn main() { return x + 1; }", 3);

  return result;
}
