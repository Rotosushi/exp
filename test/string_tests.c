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

#include "filesystem/io.h"
#include "utility/panic.h"
#include "utility/string.h"

static bool s1_same_as_s2(const char *restrict s1, size_t s1_len,
                          const char *restrict s2, size_t s2_len) {
  if (s1_len != s2_len) {
    return 1;
  }

  if (memcmp(s1, s2, s1_len) != 0) {
    return 1;
  }

  return 0;
}

// returns true on failure
bool test_string_assign(const char *data, size_t data_length) {
  String str = string_create();

  string_assign(&str, data);

  bool failed;
  if (s1_same_as_s2(str.buffer, str.length, data, data_length)) {
    fputs(str.buffer, stderr);
    failed = 1;
  } else {
    failed = 0;
  }

  string_destroy(&str);
  return failed;
}

bool test_string_to_view(const char *data, size_t data_length) {
  String str = string_create();

  string_assign(&str, data);

  bool failed;
  if (s1_same_as_s2(str.buffer, str.length, data, data_length)) {
    fputs(str.buffer, stderr);
    failed = 1;
  } else {
    failed = 0;
  }

  StringView sv = string_to_view(&str);

  if (s1_same_as_s2(sv.ptr, sv.length, data, data_length)) {
    fputs(str.buffer, stderr);
    failed |= 1;
  } else {
    failed |= 0;
  }

  string_destroy(&str);
  return failed;
}

static bool s1_same_as_sum_of_s2_s3(const char *restrict s1, size_t s1_len,
                                    const char *restrict s2, size_t s2_len,
                                    const char *restrict s3, size_t s3_len) {
  size_t total_length;
  if (__builtin_add_overflow(s2_len, s3_len, &total_length)) {
    panic("size_t overflow");
  }

  if (s1_len != total_length) {
    return 1;
  }

  if (memcmp(s1, s2, s2_len) != 0) {
    return 1;
  }

  if (memcmp(s1 + s2_len, s3, s3_len) != 0) {
    return 1;
  }

  return 0;
}

bool test_string_append(const char *d1, size_t d1_len, const char *d2,
                        size_t d2_len) {
  String str = string_create();

  string_append(&str, d1);
  string_append(&str, d2);

  bool failure;
  if (s1_same_as_sum_of_s2_s3(str.buffer, str.length, d1, d1_len, d2, d2_len)) {
    fputs(str.buffer, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

bool test_string_erase(char const *d1, size_t offset, size_t length,
                       char const *d2, size_t d2_length) {
  String str = string_create();

  string_assign(&str, d1);

  // StringView view = string_to_view_at(&str, offset, length);

  string_erase(&str, offset, length);

  bool failure;
  if (s1_same_as_s2(str.buffer, str.length, d2, d2_length)) {
    fputs(str.buffer, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

bool test_string_insert(char const *d1, size_t offset, char const *d2,
                        char const *d3, size_t d3_length) {
  String str = string_create();

  string_assign(&str, d1);

  string_insert(&str, offset, d2);

  bool failure;
  if (s1_same_as_s2(str.buffer, str.length, d3, d3_length)) {
    fputs(str.buffer, stderr);
    failure = 1;
  } else {
    failure = 0;
  }

  string_destroy(&str);
  return failure;
}

int string_tests([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  bool failure = 0;
  failure |= test_string_assign("", strlen(""));
  failure |= test_string_assign("hello", strlen("hello"));

  failure |= test_string_to_view("hello, world!", strlen("hello, world!"));

  failure |= test_string_append("hello", strlen("hello"), ", world!",
                                strlen(", world!"));

  failure |= test_string_append("hello", strlen("hello"), "/", strlen("/"));

  failure |= test_string_erase("hello world", 0, 5, " world", strlen(" world"));

  failure |= test_string_erase("hello world", 5, 6, "hello", strlen("hello"));

  failure |= test_string_erase("hello world", 2, 7, "held", strlen("held"));

  failure |= test_string_erase("hello world", 0, 11, "", strlen(""));

  failure |= test_string_insert("hello", 0, "world", "world", strlen("world"));

  failure |= test_string_insert("hello", 5, " world", "hello world",
                                strlen("hello world"));

  failure |= test_string_insert("hello", 4, " world", "hell world",
                                strlen("hell world"));

  if (failure) {
    return 1;
  } else {
    return 0;
  }
}
