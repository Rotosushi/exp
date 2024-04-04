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
#ifndef EXP_IMR_TYPE_H
#define EXP_IMR_TYPE_H

#include "utility/string.h"

typedef enum TypeKind {
  TYPEKIND_NIL,
  TYPEKIND_BOOLEAN,
  TYPEKIND_INTEGER,
  TYPEKIND_STRING_LITERAL,
  // TYPEKIND_FUNCTION,
} TypeKind;

typedef struct NilType {
  char empty; // zero length structs are not valid C
} NilType;

typedef struct BooleanType {
  char empty;
} BooleanType;

typedef struct IntegerType {
  char empty;
} IntegerType;

typedef struct StringLiteralType {
  char empty;
} StringLiteralType;

// struct Type;
// typedef struct FunctionType {
//   Type *return_type;
//   size_t argument_count;
//   Type *argument_types;
// };

/**
 * @brief represents Types in the compiler
 *
 */
typedef struct Type {
  TypeKind kind;
  union {
    NilType nil_type;
    BooleanType boolean_type;
    IntegerType integer_type;
    StringLiteralType string_literal_type;
    // FunctionType function_type;
  };
} Type;

/**
 * @brief create a new NilType
 *
 * @return Type
 */
Type type_create_nil();

/**
 * @brief create a new BooleanType
 *
 * @return Type
 */
Type type_create_boolean();

/**
 * @brief create a new IntegerType
 *
 * @return Type
 */
Type type_create_integer();

/**
 * @brief create a new StringLiteralType
 *
 * @return Type
 */
Type type_create_string_literal();

/**
 * @brief equality compares types
 *
 * @param t1
 * @param t2
 * @return bool
 */
bool type_equality(Type const *t1, Type const *t2);

String type_to_string(Type const *t);

#endif // !EXP_IMR_TYPE_H