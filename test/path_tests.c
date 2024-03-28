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
#include <stdio.h>
#include <string.h>

#include "utility/path.h"

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

// static bool s1_same_as_sum_of_s2_s3(const char *restrict s1, size_t s1_len,
//                                     const char *restrict s2, size_t s2_len,
//                                     const char *restrict s3, size_t s3_len) {
//   size_t total_length;
//   if (__builtin_add_overflow(s2_len, s3_len, &total_length)) {
//     panic("size_t overflow");
//   }

//   if (s1_len != total_length) {
//     return 1;
//   }

//   if (memcmp(s1, s2, s2_len) != 0) {
//     return 1;
//   }

//   if (memcmp(s1 + s2_len, s3, s3_len) != 0) {
//     return 1;
//   }

//   return 0;
// }

bool test_path_assign(char const *data, size_t length) {
  Path path = path_create();
  path_assign(&path, data);

  StringView sv = path_to_view(&path);

  bool failure = s1_same_as_s2(data, length, sv.ptr, sv.length);

  path_destroy(&path);

  return failure;
}

bool test_path_append(char const *d1, char const *d2, char const *d3,
                      size_t d3_len) {
  Path p1 = path_create();
  Path p2 = path_create();

  path_assign(&p1, d1);
  path_assign(&p2, d2);

  path_append(&p1, &p2);

  StringView s1 = path_to_view(&p1);

  bool failure = s1_same_as_s2(s1.ptr, s1.length, d3, d3_len);

  path_destroy(&p1);
  path_destroy(&p2);

  return failure;
}

bool test_path_concat(char const *d1, char const *d2, char const *d3,
                      size_t d3_len) {
  Path p1 = path_create();
  Path p2 = path_create();

  path_assign(&p1, d1);
  path_assign(&p2, d2);

  path_concat(&p1, &p2);

  StringView s1 = path_to_view(&p1);

  bool failure = s1_same_as_s2(s1.ptr, s1.length, d3, d3_len);

  path_destroy(&p1);
  path_destroy(&p2);

  return failure;
}

bool test_path_replace_extension(char const *d1, char const *d2, char const *d3,
                                 size_t d3_len) {
  Path p1 = path_create();
  Path p2 = path_create();

  path_assign(&p1, d1);
  path_assign(&p2, d2);

  path_replace_extension(&p1, &p2);

  bool failure = s1_same_as_s2(p1.string.buffer, p1.string.length, d3, d3_len);

  path_destroy(&p1);
  path_destroy(&p2);
  return failure;
}

int path_tests([[maybe_unused]] int argc, [[maybe_unused]] char const *argv[]) {

  bool failure = 0;

  failure |= test_path_assign("hello/world.txt", strlen("hello/world.txt"));

  failure |= test_path_append("hello", "world.txt", "hello/world.txt",
                              strlen("hello/world.txt"));

  failure |= test_path_concat("hello/", "world.txt", "hello/world.txt",
                              strlen("hello/world.txt"));

  failure |= test_path_replace_extension("hello.txt", ".data", "hello.data",
                                         strlen("hello.data"));

  failure |= test_path_replace_extension("hello", ".data", "hello.data",
                                         strlen("hello.data"));

  failure |= test_path_replace_extension("hello.", ".data", "hello.data",
                                         strlen("hello.data"));

  failure |= test_path_replace_extension("hello.", "data", "hello.data",
                                         strlen("hello.data"));

  if (failure) {
    return 1;
  } else {
    return 0;
  }
}