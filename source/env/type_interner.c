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

#include "env/type_interner.h"
#include "utility/panic.h"

TypeInterner type_interner_create() {
  TypeInterner type_interner;
  type_interner.nil_type = type_create_nil();
  type_interner.boolean_type = type_create_boolean();
  type_interner.integer_type = type_create_integer();
  type_interner.string_literal_type = type_create_string_literal();
  return type_interner;
}

void type_interner_destroy(
    [[maybe_unused]] TypeInterner *restrict type_interner) {
  return;
}

Type *type_interner_nil_type(TypeInterner *restrict type_interner) {
  return &(type_interner->nil_type);
}

Type *type_interner_boolean_type(TypeInterner *restrict type_interner) {
  return &(type_interner->boolean_type);
}

Type *type_interner_integer_type(TypeInterner *restrict type_interner) {
  return &(type_interner->integer_type);
}

Type *type_interner_string_literal_type(TypeInterner *restrict type_interner) {
  return &(type_interner->string_literal_type);
}

size_t type_interner_type_to_index(TypeInterner *restrict type_interner,
                                   Type *type) {
  if (type == &type_interner->nil_type) {
    return 0;
  } else if (type == &type_interner->boolean_type) {
    return 1;
  } else if (type == &type_interner->integer_type) {
    return 2;
  } else if (type == &type_interner->string_literal_type) {
    return 3;
  } else {
    PANIC("unknown type");
  }
}

Type *type_interner_index_to_type(TypeInterner *restrict type_interner,
                                  size_t index) {
  switch (index) {
  case 0:
    return &type_interner->nil_type;
  case 1:
    return &type_interner->boolean_type;
  case 2:
    return &type_interner->integer_type;
  case 3:
    return &type_interner->string_literal_type;
  default:
    PANIC("unkown index");
  }
}
