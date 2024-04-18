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
  TYPEKIND_VOID,
  TYPEKIND_BOOLEAN,
  TYPEKIND_INTEGER,
  TYPEKIND_FUNCTION,
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

struct Type;
// #TODO: strictly speaking, ArgumentTypes is equivalent
// to a TupleType. Maybe it would be useful to unify them?
typedef struct ArgumentTypes {
  u64 size;
  u64 capacity;
  struct Type **types;
} ArgumentTypes;

ArgumentTypes argument_types_create();
void argument_types_destroy(ArgumentTypes *restrict a);
bool argument_types_equality(ArgumentTypes const *a1, ArgumentTypes const *a2);
void argument_types_append(ArgumentTypes *restrict a, struct Type *type);

typedef struct FunctionType {
  struct Type *return_type;
  ArgumentTypes argument_types;
} FunctionType;

bool function_type_equality(FunctionType const *f1, FunctionType const *f2);

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
    FunctionType function_type;
  };
} Type;

/**
 * @brief create a new NilType
 *
 * @return Type
 */
Type type_create_void();

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
 * @brief create a new FunctionType
 *
 * @param result
 * @param args
 * @return Type
 */
Type type_create_function(Type *result, ArgumentTypes args);

void type_destroy(Type *type);
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