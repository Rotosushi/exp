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
#include <string.h>

#include "adt/string.h"
#include "utility/panic.h"
#include "utility/string_view.h"

StringView string_view_create() {
  StringView sv;
  sv.length = 0;
  sv.ptr    = NULL;
  return sv;
}

StringView string_view(u64 length, char const *ptr) {
  StringView sv = {length, ptr};
  return sv;
}

StringView string_view_from_cstring(char const *cstring) {
  StringView sv = {strlen(cstring), cstring};
  return sv;
}

bool string_view_eq(StringView sv1, StringView sv2) {
  if (sv1.ptr == sv2.ptr) { return 1; }
  if (sv1.length != sv2.length) { return 0; }
  return (memcmp(sv1.ptr, sv2.ptr, sv1.length) == 0);
}

bool string_view_empty(StringView sv) { return sv.length == 0; }

void print_string_view(String *restrict out, StringView sv) {
  for (u64 i = 0; i < sv.length; ++i) {
    string_append(out, sv);
  }
}
