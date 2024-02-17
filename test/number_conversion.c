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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utility/numbers_to_string.h"

static bool test_base10_uintmax_to_str(uintmax_t value) {
  size_t buffer_length = uintmax_safe_strlen(value, 10);
  char buffer[buffer_length];
  uintmax_to_str(value, buffer, 10);

  uintmax_t number;
  int result = sscanf(buffer, "%ju", &number);
  if (result == EOF) {
    perror("sscanf failed");
    abort();
  }

  return value != number;
}

static bool test_base10_intmax_to_str(intmax_t value) {
  size_t buffer_length = intmax_safe_strlen(value, 10);
  char buffer[buffer_length];
  intmax_to_str(value, buffer, 10);

  intmax_t number;
  int result = sscanf(buffer, "%jd", &number);
  if (result == EOF) {
    perror("sscanf failed");
    abort();
  }

  return value != number;
}

int number_conversion([[maybe_unused]] int argc,
                      [[maybe_unused]] char *argv[]) {
  bool failed = 0;
  srand((unsigned)time(NULL));

  failed |= test_base10_uintmax_to_str(UINTMAX_MAX);
  failed |= test_base10_uintmax_to_str((uintmax_t)rand());
  failed |= test_base10_uintmax_to_str(0);

  failed |= test_base10_intmax_to_str(INTMAX_MIN);
  failed |= test_base10_intmax_to_str(INTMAX_MAX);
  failed |= test_base10_intmax_to_str((intmax_t)rand());
  failed |= test_base10_intmax_to_str(0);

  if (failed) {
    return 1;
  } else {
    return 0;
  }
}