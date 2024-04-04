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
#include "imr/type.h"
#include "utility/panic.h"

Type type_create_nil() {
  Type type;
  type.kind           = TYPEKIND_NIL;
  type.nil_type.empty = 0;
  return type;
}

Type type_create_boolean() {
  Type type;
  type.kind               = TYPEKIND_BOOLEAN;
  type.boolean_type.empty = 0;
  return type;
}

Type type_create_integer() {
  Type type;
  type.kind               = TYPEKIND_INTEGER;
  type.integer_type.empty = 0;
  return type;
}

Type type_create_string_literal() {
  Type type;
  type.kind                      = TYPEKIND_STRING_LITERAL;
  type.string_literal_type.empty = 0;
  return type;
}

bool type_equality(Type const *t1, Type const *t2) {
  return t1->kind == t2->kind;
}

String type_to_string(Type const *t) {
  switch (t->kind) {
  case TYPEKIND_NIL:
    return string_from_cstring("Nil");

  case TYPEKIND_BOOLEAN:
    return string_from_cstring("Bool");

  case TYPEKIND_INTEGER:
    return string_from_cstring("Int");

  case TYPEKIND_STRING_LITERAL:
    return string_from_cstring("StringLiteral");

  default:
    PANIC("bad TYPEKIND");
  }
}