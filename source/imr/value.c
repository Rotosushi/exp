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
#include "imr/value.h"
#include "utility/panic.h"

Value value_create_integer(long i) {
  Value v;
  v.kind = VALUEKIND_INTEGER;
  v.integer = i;
  return v;
}

Value value_create_string_literal(StringView sv) {
  Value v;
  v.kind = VALUEKIND_STRING_LITERAL;
  v.string_literal = sv;
  return v;
}

bool value_equality(Value *v1, Value *v2) {
  switch (v1->kind) {
  case VALUEKIND_INTEGER:
    if (v2->kind != VALUEKIND_INTEGER) {
      return 0;
    }

    return v1->integer == v2->integer;
    break;

  case VALUEKIND_STRING_LITERAL:
    if (v2->kind != VALUEKIND_STRING_LITERAL) {
      return 0;
    }

    return string_view_equality(v1->string_literal, v2->string_literal);
    break;

  default:
    panic("bad VALUEKIND", sizeof("bad VALUEKIND"));
  }
}