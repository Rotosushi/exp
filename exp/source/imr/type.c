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
#include "utility/nearest_power.h"
#include "utility/panic.h"

ArgumentTypes argument_types_create() {
  ArgumentTypes args;
  args.capacity = 0;
  args.size     = 0;
  args.types    = NULL;
  return args;
}

void argument_types_destroy(ArgumentTypes *restrict a) {
  assert(a != NULL);
  a->capacity = 0;
  a->size     = 0;
  deallocate(a->types);
  a->types = NULL;
}

bool argument_types_equality(ArgumentTypes const *a1, ArgumentTypes const *a2) {
  assert(a1 != NULL);
  assert(a2 != NULL);
  if (a1 == a2) { return 1; }

  if (a1->size != a2->size) { return 0; }

  for (u64 i = 0; i < a1->size; ++i) {
    Type *t1 = a1->types[i];
    Type *t2 = a2->types[i];

    if (!type_equality(t1, t2)) { return 0; }
  }

  return 1;
}

static bool argument_types_full(ArgumentTypes *restrict a) {
  u64 new_size;
  if (__builtin_add_overflow(a->size, 1, &new_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  return new_size >= a->capacity;
}

static void argument_types_grow(ArgumentTypes *restrict a) {
  u64 new_capacity = nearest_power_of_two(a->capacity);

  u64 alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(Type *), &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  a->types    = reallocate(a->types, alloc_size);
  a->capacity = new_capacity;
}

void argument_types_append(ArgumentTypes *restrict a, Type *type) {
  assert(a != NULL);

  if (argument_types_full(a)) { argument_types_grow(a); }

  a->types[a->size] = type;
  a->size += 1;
}

bool function_type_equality(FunctionType const *f1, FunctionType const *f2) {
  assert(f1 != NULL);
  assert(f2 != NULL);
  if (f1 == f2) { return 1; }

  if (!type_equality(f1->return_type, f2->return_type)) { return 0; }

  return argument_types_equality(&f1->argument_types, &f2->argument_types);
}

Type type_create_nil() {
  Type type;
  type.kind           = TYPEKIND_NIL;
  type.nil_type.empty = 0;
  return type;
}

Type type_create_boolean() {
  Type type;
  type.kind               = TYPEKIND_BOOLEAN;
  type.boolean_type.empty = 0;
  return type;
}

Type type_create_integer() {
  Type type;
  type.kind               = TYPEKIND_I64;
  type.integer_type.empty = 0;
  return type;
}

Type type_create_function(Type *result, ArgumentTypes args) {
  Type type;
  type.kind          = TYPEKIND_FUNCTION;
  type.function_type = (FunctionType){result, args};
  return type;
}

void type_destroy(Type *type) {
  switch (type->kind) {
  case TYPEKIND_FUNCTION:
    argument_types_destroy(&type->function_type.argument_types);
    break;

  // #NOTE: no other types dynamically allocate
  default: break;
  }
}

bool type_equality(Type const *t1, Type const *t2) {
  if (t1->kind != t2->kind) { return 0; }

  switch (t1->kind) {
  case TYPEKIND_FUNCTION:
    return function_type_equality(&t1->function_type, &t2->function_type);

  // #NOTE: scalar types are equal when their kinds are equal
  default: return 1;
  }
}

bool type_is_scalar(Type const *t) {
  switch (t->kind) {
  case TYPEKIND_NIL:
  case TYPEKIND_BOOLEAN:
  case TYPEKIND_I64:     return 1;

  default: return 0;
  }
}

static void print_function_type(FunctionType const *restrict ft,
                                FILE *restrict file) {
  file_write("fn (", file);
  ArgumentTypes const *a = &ft->argument_types;
  for (u64 i = 0; i < a->size; ++i) {
    print_type(a->types[i], file);

    if (i < (a->size - 1)) { file_write(", ", file); }
  }

  file_write(") -> ", file);
  print_type(ft->return_type, file);
}

void print_type(Type const *restrict t, FILE *restrict file) {
  switch (t->kind) {
  case TYPEKIND_NIL: file_write("nil", file); break;

  case TYPEKIND_BOOLEAN: file_write("bool", file); break;

  case TYPEKIND_I64: file_write("i64", file); break;

  case TYPEKIND_FUNCTION: print_function_type(&t->function_type, file); break;

  default: file_write("undefined", file);
  }
}