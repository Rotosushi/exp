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

TypeInterner type_interner_create() {
  TypeInterner type_interner;
  type_interner.integer_type = type_create_integer();
  type_interner.string_literal_type = type_create_string_literal();
  return type_interner;
}

void type_interner_destroy(
    [[maybe_unused]] TypeInterner *restrict type_interner) {
  return;
}

Type *type_interner_integer_type(TypeInterner *restrict type_interner) {
  return &(type_interner->integer_type);
}

Type *type_interner_string_literal_type(TypeInterner *restrict type_interner) {
  return &(type_interner->string_literal_type);
}
