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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "intrinsics/type_of.h"
#include "utility/panic.h"

Type *type_of(Value *restrict value, Context *restrict context) {
  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED:
    return context_nil_type(context);

  case VALUEKIND_NIL:
    return context_nil_type(context);

  case VALUEKIND_BOOLEAN:
    return context_boolean_type(context);

  case VALUEKIND_INTEGER:
    return context_integer_type(context);

  case VALUEKIND_STRING_LITERAL:
    return context_string_literal_type(context);

  case VALUEKIND_TYPE:
    return value->type;

  case VALUEKIND_FUNCTION:
    return function_type_of(&value->function, context);

  default:
    PANIC("bad VALUEKIND");
  }
}