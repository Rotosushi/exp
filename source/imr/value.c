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

Value value_create_nil() {
  Value value;
  value.kind = VALUEKIND_NIL;
  value.nil = 0;
  return value;
}

Value value_create_boolean(bool b) {
  Value value;
  value.kind = VALUEKIND_BOOLEAN;
  value.boolean = b;
  return value;
}

Value value_create_integer(long i) {
  Value value;
  value.kind = VALUEKIND_INTEGER;
  value.integer = i;
  return value;
}

Value value_create_string_literal(StringView sv) {
  Value value;
  value.kind = VALUEKIND_STRING_LITERAL;
  value.string_literal = sv;
  return value;
}

void value_assign(Value *dest, Value *source) {
  if (dest == source) {
    return;
  }

  switch (source->kind) {
  case VALUEKIND_NIL:
    dest->kind = VALUEKIND_NIL;
    dest->nil = 0;
    break;

  case VALUEKIND_BOOLEAN:
    dest->kind = VALUEKIND_BOOLEAN;
    dest->boolean = source->boolean;
    break;

  case VALUEKIND_INTEGER:
    dest->kind = VALUEKIND_INTEGER;
    dest->integer = source->integer;
    break;

  case VALUEKIND_STRING_LITERAL:
    dest->kind = VALUEKIND_STRING_LITERAL;
    dest->string_literal = source->string_literal;
    break;

  default:
    panic("bad VALUEKIND");
    break;
  }
}

bool value_equality(Value *v1, Value *v2) {
  if (v1 == v2) {
    return 1;
  }

  switch (v1->kind) {
  case VALUEKIND_NIL:
    return v2->kind == VALUEKIND_NIL;
    break;

  case VALUEKIND_BOOLEAN:
    if (v2->kind != VALUEKIND_BOOLEAN) {
      return 0;
    }

    return v1->boolean == v2->boolean;
    break;

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
    panic("bad VALUEKIND");
  }
}