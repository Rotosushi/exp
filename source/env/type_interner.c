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
#include "utility/nearest_power.h"
#include "utility/panic.h"

FunctionTypes function_types_create() {
  FunctionTypes f;
  f.capacity = 0;
  f.size     = 0;
  f.types    = NULL;
  return f;
}

void function_types_destroy(FunctionTypes *restrict f) {
  assert(f != NULL);

  for (u64 i = 0; i < f->size; ++i) {
    type_destroy(&f->types[i]);
  }

  f->capacity = 0;
  f->size     = 0;
  free(f->types);
  f->types = NULL;
}

static bool function_types_full(FunctionTypes *restrict f) {
  u64 new_size;
  if (__builtin_add_overflow(f->size, 1, &new_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  return new_size >= f->capacity;
}

static void function_types_grow(FunctionTypes *restrict f) {
  u64 new_capacity = nearest_power_of_two(f->capacity + 1);

  u64 alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Type), &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  Type *result = realloc(f->types, alloc_size);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  f->types    = result;
  f->capacity = new_capacity;
}

Type *function_types_append(FunctionTypes *restrict f, Type *return_type,
                            ArgumentTypes argument_types) {
  assert(f != NULL);

  Type function_type = type_create_function(return_type, argument_types);

  for (u64 i = 0; i < f->size; ++i) {
    Type *t = &f->types[i];
    if (type_equality(&function_type, t)) {
      argument_types_destroy(&argument_types);
      return t;
    }
  }

  if (function_types_full(f)) {
    function_types_grow(f);
  }

  Type *new_type = &f->types[f->size];
  *new_type      = function_type;
  f->size += 1;
  return new_type;
}

TypeInterner type_interner_create() {
  TypeInterner type_interner;
  type_interner.nil_type       = type_create_void();
  type_interner.boolean_type   = type_create_boolean();
  type_interner.integer_type   = type_create_integer();
  type_interner.function_types = function_types_create();
  return type_interner;
}

void type_interner_destroy(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return;
}

Type *type_interner_void_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->nil_type);
}

Type *type_interner_boolean_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->boolean_type);
}

Type *type_interner_integer_type(TypeInterner *restrict type_interner) {
  assert(type_interner != NULL);
  return &(type_interner->integer_type);
}

Type *type_interner_function_type(TypeInterner *restrict type_interner,
                                  Type *return_type,
                                  ArgumentTypes argument_types) {
  assert(type_interner != NULL);
  return function_types_append(&type_interner->function_types, return_type,
                               argument_types);
}
