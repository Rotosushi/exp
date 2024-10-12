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

#include "adt/string.h"

typedef enum TypeKind {
  TYPEKIND_NIL,
  TYPEKIND_BOOLEAN,
  TYPEKIND_I64,

  TYPEKIND_POINTER,
  TYPEKIND_TUPLE,
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

typedef struct PointerType {
  struct Type *pointee_type;
} PointerType;

typedef struct TupleType {
  u64 size;
  u64 capacity;
  struct Type **types;
} TupleType;

TupleType tuple_type_create();
void tuple_type_destroy(TupleType *restrict tuple_type);
bool tuple_type_equality(TupleType const *A, TupleType const *B);
void tuple_type_append(TupleType *restrict tuple_type, struct Type *type);

typedef struct FunctionType {
  struct Type *return_type;
  TupleType argument_types;
} FunctionType;

bool function_type_equality(FunctionType const *A, FunctionType const *B);

/**
 * @brief represents Types in the compiler
 *
 * #TODO #FEATURE: type attributes, something like u16::max, could work in the
 * same way that struct members work, there is also no reason we cannot bind a
 * function ptr there as well, giving type "member" functions. except not tied
 * to a specific instance of that type. I think we can leverage such a mechanism
 * for type introspection if we implicitly fill in the member details when we
 * create the type.
 */
typedef struct Type {
  TypeKind kind;
  union {
    NilType nil_type;
    BooleanType boolean_type;
    IntegerType integer_type;
    PointerType pointer_type;
    TupleType tuple_type;
    FunctionType function_type;
  };
} Type;

Type type_create_nil();

Type type_create_boolean();

Type type_create_integer();

Type type_create_pointer(Type *pointee);

Type type_create_tuple(TupleType tuple_type);

Type type_create_function(Type *result, TupleType args);

void type_destroy(Type *type);
/**
 * @brief equality compares types
 *
 * @param t1
 * @param t2
 * @return bool
 */
bool type_equality(Type const *t1, Type const *t2);

bool type_is_scalar(Type const *t);

void print_type(Type const *restrict t, String *restrict buf);

#endif // !EXP_IMR_TYPE_H
