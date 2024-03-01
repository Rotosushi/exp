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

void path_assign(Path *restrict path, char const *data, size_t length) {
  string_assign(&path->string, data, length);
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

/*
static StringView path_compute_filename(Path const *restrict path) {
  char const *cursor = path->string.buffer + path->string.length;
}

void path_remove_filename(Path *restrict p1) {}

void path_replace_filename(Path *restrict p1, const Path *restrict p2) {}

void path_replace_extension(Path *restrict p1, const Path *restrict p2) {}

Path path_root_name(Path const *restrict path) {}

Path path_root_directory(Path const *restrict path) {}

Path path_root_path(Path const *restrict path) {}

Path path_relative_path(Path const *restrict path) {}

Path path_parent_path(Path const *restrict path) {}

Path path_filename(Path const *restrict path) {}

Path path_stem(Path const *restrict path) {}

Path path_extension(Path const *restrict path) {}

bool path_has_root_path(Path const *restrict path) {}
bool path_has_root_name(Path const *restrict path) {}
bool path_has_root_directory(Path const *restrict path) {}
bool path_has_relative_path(Path const *restrict path) {}
bool path_has_parent_path(Path const *restrict path) {}
bool path_has_filename(Path const *restrict path) {}
bool path_has_stem(Path const *restrict path) {}
bool path_has_extension(Path const *restrict path) {}

bool path_is_absolute(Path const *restrict path) {}
bool path_is_relative(Path const *restrict path) {}
*/