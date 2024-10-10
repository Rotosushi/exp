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
#include <time.h>

#include "adt/string.h"
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

static bool in_range(u64 value) { return (value > 0) && (value <= 100); }

static u64 gen() { return ((u64)rand() % 100); }

static Pair generate() {
  while (true) {
    Pair result = pair(gen(), gen());
    if (in_range(result.a * result.b) && in_range(result.a - result.b)) {
      return result;
    }
  }
}

static StringView select_operator(u8 index) {
  switch (index) {
  case 0: return SV("+");
  case 1: return SV("-");
  case 2: return SV("*");
  case 3: return SV("/");
  case 4: return SV("%");

  default: EXP_UNREACHABLE;
  }
}

static i32 compute_operation(u8 index, Pair num) {
  switch (index) {
  case 0: return (i32)(num.a + num.b);
  case 1: return (i32)(num.a - num.b);
  case 2: return (i32)(num.a * num.b);
  case 3: return (i32)(num.a / num.b);
  case 4: return (i32)(num.a % num.b);

  default: EXP_UNREACHABLE;
  }
}

static void emit_operation_on_global(u8 index,
                                     u8 operator,
                                     Pair num,
                                     String *restrict buffer) {
  switch (index) {
  case 0: {
    string_append(buffer, SV("const x = "));
    string_append_u64(buffer, num.a);
    string_append(buffer, SV("; const y = "));
    string_append_u64(buffer, num.b);
    string_append(buffer, SV("; fn main() { return x "));
    string_append(buffer, select_operator(operator));
    string_append(buffer, SV(" y; }"));
    break;
  }

  case 1: {
    string_append(buffer, SV("const x = "));
    string_append_u64(buffer, num.a);
    string_append(buffer, SV(";\nfn main() { return x "));
    string_append(buffer, select_operator(operator));
    string_append(buffer, SV(" "));
    string_append_u64(buffer, num.b);
    string_append(buffer, SV("; }"));
    break;
  }

  case 2: {
    string_append(buffer, SV("const y = "));
    string_append_u64(buffer, num.b);
    string_append(buffer, SV(";\nfn main() { return "));
    string_append_u64(buffer, num.a);
    string_append(buffer, SV(" "));
    string_append(buffer, select_operator(operator));
    string_append(buffer, SV(" y; }"));
    break;
  }

  default: EXP_UNREACHABLE;
  }
}

int test_operations_on_globals(StringView source_path) {
  int result = EXIT_SUCCESS;
  for (u8 configuration = 0; configuration < 3; ++configuration) {
    for (u8 operation = 0; operation < 5; ++operation) {
      String buffer = string_create();
      Pair num      = generate();

      emit_operation_on_global(configuration, operation, num, &buffer);

      result |= test_exp(source_path,
                         string_to_cstring(&buffer),
                         compute_operation(operation, num));

      string_destroy(&buffer);
    }
  }
  return result;
}

int global_constant([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int result = EXIT_SUCCESS;

  StringView source_path = SV(EXP_TEST_DIR "/global_constant.exp");

  srand((u32)time(NULL));

  result |= test_exp(source_path, "const x = 1; fn main() { return x; }", 1);

  result |= test_operations_on_globals(source_path);

  return result;
}
