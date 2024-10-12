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
#include <assert.h>
#include <stdlib.h>

#include "imr/type.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

TupleType tuple_type_create() {
  TupleType tuple_type;
  tuple_type.capacity = 0;
  tuple_type.size     = 0;
  tuple_type.types    = NULL;
  return tuple_type;
}

void tuple_type_destroy(TupleType *restrict tuple_type) {
  assert(tuple_type != NULL);
  tuple_type->capacity = 0;
  tuple_type->size     = 0;
  deallocate(tuple_type->types);
  tuple_type->types = NULL;
}

bool tuple_type_equality(TupleType const *A, TupleType const *B) {
  assert(A != NULL);
  assert(B != NULL);
  if (A == B) { return 1; }

  if (A->size != B->size) { return 0; }

  for (u64 i = 0; i < A->size; ++i) {
    Type *t = A->types[i];
    Type *u = B->types[i];

    if (!type_equality(t, u)) { return 0; }
  }

  return 1;
}

static bool tuple_type_full(TupleType *restrict tuple_type) {
  return (tuple_type->size + 1) >= tuple_type->capacity;
}

static void tuple_type_grow(TupleType *restrict tuple_type) {
  Growth g             = array_growth_u64(tuple_type->capacity, sizeof(Type *));
  tuple_type->types    = reallocate(tuple_type->types, g.alloc_size);
  tuple_type->capacity = g.new_capacity;
}

void tuple_type_append(TupleType *restrict tuple_type, Type *type) {
  assert(tuple_type != NULL);

  if (tuple_type_full(tuple_type)) { tuple_type_grow(tuple_type); }

  tuple_type->types[tuple_type->size] = type;
  tuple_type->size += 1;
}

bool function_type_equality(FunctionType const *A, FunctionType const *B) {
  assert(A != NULL);
  assert(B != NULL);
  if (A == B) { return 1; }

  if (!type_equality(A->return_type, B->return_type)) { return 0; }

  return tuple_type_equality(&A->argument_types, &B->argument_types);
}

Type type_create_nil() {
  Type type = {.kind = TYPEKIND_NIL, .nil_type.empty = 0};
  return type;
}

Type type_create_boolean() {
  Type type = {.kind = TYPEKIND_BOOLEAN, .boolean_type.empty = 0};
  return type;
}

Type type_create_integer() {
  Type type = {.kind = TYPEKIND_I64, .integer_type.empty = 0};
  return type;
}

Type type_create_pointer(Type *restrict pointee) {
  Type type = {.kind = TYPEKIND_POINTER, .pointer_type.pointee_type = pointee};
  return type;
}

Type type_create_tuple(TupleType tuple_type) {
  Type type = {.kind = TYPEKIND_TUPLE, .tuple_type = tuple_type};
  return type;
}

Type type_create_function(Type *result, TupleType args) {
  Type type = {
      .kind = TYPEKIND_FUNCTION, .function_type = (FunctionType){result, args}
  };
  return type;
}

void type_destroy(Type *type) {
  switch (type->kind) {
  case TYPEKIND_TUPLE: {
    tuple_type_destroy(&type->tuple_type);
    break;
  }

  case TYPEKIND_FUNCTION: {
    tuple_type_destroy(&type->function_type.argument_types);
    break;
  }

  // #NOTE: no other types dynamically allocate
  default: break;
  }
}

bool type_equality(Type const *A, Type const *B) {
  if (A->kind != B->kind) { return 0; }

  switch (A->kind) {
  case TYPEKIND_POINTER:
    return type_equality(A->pointer_type.pointee_type,
                         B->pointer_type.pointee_type);
  case TYPEKIND_TUPLE:
    return tuple_type_equality(&A->tuple_type, &B->tuple_type);
  case TYPEKIND_FUNCTION:
    return function_type_equality(&A->function_type, &B->function_type);

  // #NOTE: scalar types are equal when their kinds are equal
  default: return true;
  }
}

bool type_is_scalar(Type const *T) {
  switch (T->kind) {
  case TYPEKIND_NIL:
  case TYPEKIND_BOOLEAN:
  case TYPEKIND_I64:
  case TYPEKIND_POINTER: return true;

  // a tuple type of size two or more cannot be scalar
  // unless we optimize it to be so. which is a TODO.
  // and a tuple type of length 0 or 1 is never created.
  case TYPEKIND_TUPLE:
  default:             return false;
  }
}

static void print_pointer_type(PointerType const *restrict pointer,
                               String *restrict buf) {
  string_append(buf, SV("*"));
  print_type(pointer->pointee_type, buf);
}

static void print_tuple_type(TupleType const *restrict tuple_type,
                             String *restrict buf) {
  string_append(buf, SV("("));
  for (u64 i = 0; i < tuple_type->size; ++i) {
    print_type(tuple_type->types[i], buf);

    if (i < (tuple_type->size - 1)) { string_append(buf, SV(", ")); }
  }
  string_append(buf, SV(")"));
}

static void print_function_type(FunctionType const *restrict function_type,
                                String *restrict buf) {
  string_append(buf, SV("fn "));
  TupleType const *tuple_type = &function_type->argument_types;
  print_tuple_type(tuple_type, buf);
  string_append(buf, SV(" -> "));
  print_type(function_type->return_type, buf);
}

void print_type(Type const *restrict T, String *restrict buf) {
  switch (T->kind) {
  case TYPEKIND_NIL:      string_append(buf, SV("nil")); break;
  case TYPEKIND_BOOLEAN:  string_append(buf, SV("bool")); break;
  case TYPEKIND_I64:      string_append(buf, SV("i64")); break;
  case TYPEKIND_POINTER:  print_pointer_type(&T->pointer_type, buf); break;
  case TYPEKIND_TUPLE:    print_tuple_type(&T->tuple_type, buf); break;
  case TYPEKIND_FUNCTION: print_function_type(&T->function_type, buf); break;

  default: EXP_UNREACHABLE;
  }
}
