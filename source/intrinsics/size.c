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

#include "intrinsics/size.h"
#include "utility/panic.h"

size_t size_of(Type *restrict type) {
  assert(type != NULL);

  switch (type->kind) {
  case TYPEKIND_NIL:
    return 1;
  case TYPEKIND_BOOLEAN:
    return 1;
  case TYPEKIND_INTEGER:
    return 8;
  // #NOTE a string literals size is the length of
  // the actual string literal. so we cannot return
  // a meaningful value here.
  case TYPEKIND_STRING_LITERAL:
    return 0;

  default:
    panic("bad TYPEKIND", sizeof("bad TYPEKIND"));
  }
}