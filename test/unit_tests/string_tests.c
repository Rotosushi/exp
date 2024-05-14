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

#include "adt/string.h"
#include "utility/io.h"
#include "utility/panic.h"

// returns true on failure
bool test_string_assign(StringView sv) {
  String str = string_create();

  string_assign(&str, sv);

  bool failed;
  if (!string_eq(&str, sv)) {
    // fputs(str.buffer, stderr);
    failed = 1;
  } else {
    failed = 0;
  }

  string_destroy(&str);
  return failed;
}

bool test_string_to_view(StringView sv) {
  String str = string_create();

  string_assign(&str, sv);

  bool failed;
  if (!string_eq(&str, sv)) {
    // fputs(str.buffer, stderr);
    failed = 1;
  } else {
    failed = 0;
  }

  StringView sv0 = string_to_view(&str);

  if (!string_view_equality(sv0, sv)) {
    // fputs(str.buffer, stderr);
    failed |= 1;
  } else {
    failed |= 0;
  }

  string_destroy(&str);
  return failed;
}

bool test_string_append(StringView sv0, StringView sv1, StringView sv2) {
  String str = string_create();

  string_append(&str, sv0);
  string_append(&str, sv1);

  bool failure;
  if (!string_eq(&str, sv2)) {
    // fputs(str.ptr, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

bool test_string_erase(StringView sv0, u64 offset, u64 length, StringView sv1) {
  String str = string_create();

  string_assign(&str, sv0);

  string_erase(&str, offset, length);

  bool failure;
  if (!string_eq(&str, sv1)) {
    // fputs(str.buffer, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

bool test_string_insert(StringView sv0, u64 offset, StringView sv1,
                        StringView sv2) {
  String str = string_create();

  string_assign(&str, sv0);

  string_insert(&str, offset, sv1);

  bool failure;
  if (!string_eq(&str, sv2)) {
    // fputs(str.buffer, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

i32 string_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char *argv[]) {
  bool failure = 0;
  failure |= test_string_assign(SV(""));
  failure |= test_string_assign(SV("hello"));

  failure |= test_string_to_view(SV("hello, world!"));

  failure |=
      test_string_append(SV("hello"), SV(", world!"), SV("hello, world!"));

  failure |= test_string_append(SV("hello"), SV("/"), SV("hello/"));

  failure |= test_string_erase(SV("hello world"), 0, 5, SV(" world"));

  failure |= test_string_erase(SV("hello world"), 5, 6, SV("hello"));

  failure |= test_string_erase(SV("hello world"), 2, 7, SV("held"));

  failure |= test_string_erase(SV("hello world"), 0, 11, SV(""));

  failure |= test_string_insert(SV("hello"), 0, SV("world"), SV("world"));

  failure |=
      test_string_insert(SV("hello"), 5, SV(", world!"), SV("hello, world!"));

  failure |=
      test_string_insert(SV("hello"), 4, SV(", world!"), SV("hell, world!"));

  if (failure) {
    return 1;
  } else {
    return 0;
  }
}
