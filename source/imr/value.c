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
#include "env/context.h"
#include "utility/panic.h"

Value value_create() {
  Value value;
  value.kind = VALUEKIND_UNINITIALIZED;
  value.void_  = 0;
  return value;
}

Value value_create_nil() {
  Value value;
  value.kind = VALUEKIND_NIL;
  value.void_  = 0;
  return value;
}

Value value_create_boolean(bool b) {
  Value value;
  value.kind    = VALUEKIND_BOOLEAN;
  value.boolean = b;
  return value;
}

Value value_create_integer(i64 i) {
  Value value;
  value.kind    = VALUEKIND_INTEGER;
  value.integer = i;
  return value;
}

void value_assign(Value *dest, Value *source) {
  if (dest == source) {
    return;
  }

  /*
    since values are "POD" it is valid to use
    struct assignment on them. This function
    is here really to ease the refactoring.
  */
  *dest = *source;
}

bool value_equality(Value *v1, Value *v2) {
  if (v1 == v2) {
    return 1;
  }

  switch (v1->kind) {
  case VALUEKIND_UNINITIALIZED:
    return v2->kind == VALUEKIND_UNINITIALIZED;

  case VALUEKIND_NIL:
    return v2->kind == VALUEKIND_NIL;

  case VALUEKIND_BOOLEAN:
    if (v2->kind != VALUEKIND_BOOLEAN) {
      return 0;
    }

    return v1->boolean == v2->boolean;

  case VALUEKIND_INTEGER:
    if (v2->kind != VALUEKIND_INTEGER) {
      return 0;
    }

    return v1->integer == v2->integer;

  default:
    PANIC("bad VALUEKIND");
  }
}
