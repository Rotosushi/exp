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

#include "env/type_interner.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

static PointerTypes pointer_types_create() {
  PointerTypes types = {.size = 0, .capacity = 0, .types = nullptr};
  return types;
}

static void pointer_types_destroy(PointerTypes *restrict pointer_types) {
  for (u64 i = 0; i < pointer_types->size; ++i) {
    type_destroy(pointer_types->types + i);
  }

  pointer_types->size     = 0;
  pointer_types->capacity = 0;
  deallocate(pointer_types->types);
  pointer_types->types = nullptr;
}

static bool pointer_types_full(PointerTypes *restrict pointer_types) {
  return (pointer_types->size + 1) >= pointer_types->capacity;
}

static void pointer_types_grow(PointerTypes *restrict pointer_types) {
  Growth g =
      array_growth_u64(pointer_types->capacity, sizeof(*pointer_types->types));
  pointer_types->types    = reallocate(pointer_types->types, g.alloc_size);
  pointer_types->capacity = g.new_capacity;
}

static Type *pointer_types_append(PointerTypes *restrict pointer_types,
                                  Type *restrict pointee_type) {
  Type type = type_create_pointer(pointee_type);

  for (u64 i = 0; i < pointer_types->size; ++i) {
    if (type_equality(&type, pointer_types->types + i))
      return pointer_types->types + i;
  }

  if (pointer_types_full(pointer_types)) { pointer_types_grow(pointer_types); }

  Type *result = pointer_types->types + pointer_types->size;
  *result      = type;
  return result;
}

static FunctionTypes function_types_create() {
  FunctionTypes function_types;
  function_types.capacity = 0;
  function_types.size     = 0;
  function_types.types    = NULL;
  return function_types;
}

static void function_types_destroy(FunctionTypes *restrict function_types) {
  assert(function_types != NULL);

  for (u64 i = 0; i < function_types->size; ++i) {
    type_destroy(&function_types->types[i]);
  }

  function_types->capacity = 0;
  function_types->size     = 0;
  deallocate(function_types->types);
  function_types->types = NULL;
}

static bool function_types_full(FunctionTypes *restrict function_types) {
  return (function_types->size + 1) >= function_types->capacity;
}

static void function_types_grow(FunctionTypes *restrict function_types) {
  Growth g                 = array_growth_u64(function_types->capacity,
                              sizeof(*function_types->types));
  function_types->types    = reallocate(function_types->types, g.alloc_size);
  function_types->capacity = g.new_capacity;
}

static Type *function_types_append(FunctionTypes *restrict function_types,
                                   Type *return_type,
                                   TupleType argument_types) {
  assert(function_types != NULL);

  Type function_type = type_create_function(return_type, argument_types);

  for (u64 i = 0; i < function_types->size; ++i) {
    Type *t = &function_types->types[i];
    if (type_equality(&function_type, t)) {
      tuple_type_destroy(&argument_types);
      return t;
    }
  }

  if (function_types_full(function_types)) {
    function_types_grow(function_types);
  }

  Type *new_type = function_types->types + function_types->size;
  *new_type      = function_type;
  function_types->size += 1;
  return new_type;
}

static TupleTypes tuple_types_create() {
  TupleTypes types = {.size = 0, .capacity = 0, .types = NULL};
  return types;
}

static void tuple_types_destroy(TupleTypes *restrict tuple_types) {
  for (u64 i = 0; i < tuple_types->size; ++i) {
    Type *T = tuple_types->types + i;
    assert(T->kind == TYPEKIND_TUPLE);
    tuple_type_destroy(&T->tuple_type);
  }

  deallocate(tuple_types->types);
  tuple_types->types    = NULL;
  tuple_types->size     = 0;
  tuple_types->capacity = 0;
}

static bool tuple_types_full(TupleTypes *restrict tuple_types) {
  return (tuple_types->size + 1) >= tuple_types->capacity;
}

static void tuple_types_grow(TupleTypes *restrict tuple_types) {
  Growth g =
      array_growth_u64(tuple_types->capacity, sizeof(*tuple_types->types));
  tuple_types->types    = reallocate(tuple_types->types, g.alloc_size);
  tuple_types->capacity = g.new_capacity;
}

static Type *tuple_types_append(TupleTypes *restrict tuple_types,
                                TupleType tuple) {
  Type T = type_create_tuple(tuple);

  for (u64 i = 0; i < tuple_types->size; ++i) {
    Type *U = tuple_types->types + i;
    if (type_equality(&T, U)) {
      tuple_type_destroy(&tuple);
      return U;
    }
  }

  if (tuple_types_full(tuple_types)) { tuple_types_grow(tuple_types); }

  Type *new_type = tuple_types->types + tuple_types->size;
  *new_type      = T;
  tuple_types->size += 1;
  return new_type;
}

TypeInterner type_interner_create() {
  TypeInterner type_interner = {.nil_type       = type_create_nil(),
                                .boolean_type   = type_create_boolean(),
                                .i64_type       = type_create_integer(),
                                .pointer_types  = pointer_types_create(),
                                .tuple_types    = tuple_types_create(),
                                .function_types = function_types_create()};
  return type_interner;
}

void type_interner_destroy(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  pointer_types_destroy(&type_interner->pointer_types);
  tuple_types_destroy(&type_interner->tuple_types);
  function_types_destroy(&type_interner->function_types);
  return;
}

Type *type_interner_nil_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->nil_type);
}

Type *type_interner_boolean_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->boolean_type);
}

Type *type_interner_i64_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->i64_type);
}

Type *type_interner_pointer_type(TypeInterner *restrict type_interner,
                                 Type *restrict pointee) {
  assert(type_interner != NULL);
  return pointer_types_append(&type_interner->pointer_types, pointee);
}

Type *type_interner_tuple_type(TypeInterner *restrict type_interner,
                               TupleType tuple) {
  assert(type_interner != NULL);
  return tuple_types_append(&type_interner->tuple_types, tuple);
}

Type *type_interner_function_type(TypeInterner *restrict type_interner,
                                  Type *return_type,
                                  TupleType argument_types) {
  assert(type_interner != NULL);
  return function_types_append(
      &type_interner->function_types, return_type, argument_types);
}
