// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_TYPE_INTERNER_H
#define EXP_ENV_TYPE_INTERNER_H
#include <stddef.h>

#include "imr/type.h"

/**
 * @brief The TypeInterner holds unique instances of
 * every type used.
 *
 * @note Types retrieved from the same TypeInterner
 * can be equality compared using their pointer values.
 *
 */
typedef struct TypeInterner {
  Type nil_type;
  Type boolean_type;
  Type integer_type;
  Type string_literal_type;
} TypeInterner;

/**
 * @brief create a TypeInterner
 *
 * @return TypeInterner
 */
TypeInterner type_interner_create();

/**
 * @brief destroy a TypeInterner
 *
 * @param type_interner
 */
void type_interner_destroy(TypeInterner *restrict type_interner);

/**
 * @brief get the NilType
 *
 * @param type_interner
 * @return Type*
 */
Type *type_interner_nil_type(TypeInterner *restrict type_interner);

/**
 * @brief get the BooleanType
 *
 * @param type_interner
 * @return Type*
 */
Type *type_interner_boolean_type(TypeInterner *restrict type_interner);

/**
 * @brief get the IntegerType
 *
 * @param type_interner
 * @return Type*
 */
Type *type_interner_integer_type(TypeInterner *restrict type_interner);

/**
 * @brief get the StringLiteralType
 *
 * @param type_interner
 * @return Type*
 */
Type *type_interner_string_literal_type(TypeInterner *restrict type_interner);

size_t type_interner_type_to_index(TypeInterner *restrict type_interner,
                                   Type *type);

Type *type_interner_index_to_type(TypeInterner *restrict type_interner,
                                  size_t index);

#endif // !EXP_ENV_TYPE_INTERNER_H