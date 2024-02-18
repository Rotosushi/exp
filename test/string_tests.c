// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#include <stdio.h>
#include <string.h>

#include "utility/string.h"

bool test_string_assign(const char *data, size_t data_length) {
  string str;
  string_init(&str);

  string_assign(&str, data, data_length);
  return memcmp(str.buffer, data, data_length) != 0;
}

int string_tests([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  bool failure = 0;

  test_string_assign("hello", sizeof("hello"));

  if (failure) {
    return 1;
  } else {
    return 0;
  }
}
