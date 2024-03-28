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
#include <assert.h>

#include "utility/path.h"

Path path_create() {
  Path path;
  path.string = string_create();
  return path;
}

void path_destroy(Path *restrict path) {
  assert(path != NULL);
  string_destroy(&path->string);
}

StringView path_to_view(const Path *restrict path) {
  assert(path != NULL);
  return string_to_view(&path->string);
}

bool path_empty(Path const *restrict path) {
  assert(path != NULL);
  return string_empty(&path->string);
}

void path_assign(Path *restrict path, char const *restrict data) {
  assert(path != NULL);
  string_assign(&path->string, data);
}

Path path_clone(Path *restrict path) {
  assert(path != NULL);
  Path result = path_create();
  string_assign(&result.string, path->string.buffer);
  return result;
}

int path_compare(Path const *restrict p1, Path const *restrict p2) {
  assert(p1 != NULL);
  assert(p2 != NULL);
  return string_compare(&p1->string, &p2->string);
}

void path_append(Path *restrict p1, const Path *restrict p2) {
  assert(p1 != NULL);
  assert(p2 != NULL);
  string_append_char(&p1->string, '/');
  string_append_string(&p1->string, &p2->string);
}

void path_concat(Path *restrict p1, const Path *restrict p2) {
  assert(p1 != NULL);
  assert(p2 != NULL);
  string_append_string(&p1->string, &p2->string);
}

void path_replace_extension(Path *restrict p1, const Path *restrict p2) {
  assert(p1 != NULL);
  // the path is something like
  // /some/kind/of/file.txt
  // or
  // /some/kind/of/.file.txt
  // or
  // /some/kind/of/.file

  // search for the last '/' in the string
  size_t length = p1->string.length;
  size_t cursor = length;
  char *buffer = p1->string.buffer;
  while ((cursor != 0) && (buffer[cursor] != '/')) {
    --cursor;
  }

  // set the cursor to the first char in the filename
  if (buffer[cursor] == '/') {
    ++cursor;
  }

  // if the first character in the path is '.' ignore it
  if (buffer[cursor] == '.') {
    ++cursor;
  }

  // find the beginning of the extension
  // or the end of the filename
  while ((cursor < length) && (buffer[cursor] != '.')) {
    ++cursor;
  }

  if (p2 == NULL) {
    // remove the extension
    String const *str = &p1->string;
    char *pos = str->buffer + cursor;
    char *rest = pos + str->length;
    size_t rest_length = (size_t)((str->buffer + str->length) - rest);
    string_erase(&(p1->string), cursor, rest_length);
  } else {
    if (p2->string.buffer[0] != '.') {
      if (p1->string.buffer[cursor] != '.') {
        string_append_char(&(p1->string), '.');
      } else {
        ++cursor;
      }
    }
    // insert the given extension.
    string_insert(&(p1->string), cursor, p2->string.buffer);
  }
}