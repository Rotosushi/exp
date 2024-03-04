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

#include "filesystem/path.h"

Path path_create() {
  Path path;
  path.string = string_create();
  return path;
}

void path_destroy(Path *restrict path) { string_destroy(&path->string); }

StringView path_to_view(const Path *restrict path) {
  return string_to_view(&path->string);
}

bool path_empty(Path const *restrict path) {
  return string_empty(&path->string);
}

void path_assign(Path *restrict path, char const *restrict data,
                 size_t length) {
  string_assign(&path->string, data, length);
}

Path path_clone(Path *restrict path) {
  Path result = path_create();
  string_assign(&result.string, path->string.buffer, path->string.length);
  return result;
}

int path_compare(Path const *restrict p1, Path const *restrict p2) {
  return string_compare(&p1->string, &p2->string);
}

void path_append(Path *restrict p1, const Path *restrict p2) {
  string_append(&p1->string, "/", sizeof("/"));
  string_append_string(&p1->string, &p2->string);
}

void path_concat(Path *restrict p1, const Path *restrict p2) {
  string_append_string(&p1->string, &p2->string);
}

// void path_replace_extension(Path *restrict p1, const Path *restrict p2) {
//   // the path is something like
//   // /some/kind/of/file.txt
//   // or
//   // /some/kind/of/.file.txt
//   // or
//   // /some/kind/of/.file
//   size_t length = p1->string.length;
//   size_t cursor = length;
//   char *buffer = p1->string.buffer;
//   while (1) {
//     if (buffer[cursor] == '.') {
//     }

//     --cursor;
//   }
// }