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
#include <string.h>

#include "env/context.h"
#include "imr/value.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

Tuple tuple_create() {
  Tuple tuple = {.capacity = 0, .size = 0, .elements = NULL};
  return tuple;
}

void tuple_destroy(Tuple *restrict tuple) {
  deallocate(tuple->elements);
  tuple->elements = NULL;
  tuple->capacity = 0;
  tuple->size     = 0;
}

void tuple_assign(Tuple *restrict A, Tuple *restrict B) {
  tuple_destroy(A);
  A->capacity = B->capacity;
  A->size     = B->size;
  A->elements = callocate(A->capacity, sizeof(*A->elements));

  memcpy(A->elements, B->elements, A->size);
}

bool tuple_equal(Tuple *A, Tuple *B) {
  if (A->size != B->size) { return 0; }

  for (u64 i = 0; i < A->size; ++i) {
    if (!operand_equality(A->elements[i], B->elements[i])) { return 0; }
  }

  return 1;
}

static bool tuple_full(Tuple *restrict tuple) {
  return (tuple->size + 1) >= tuple->capacity;
}

static void tuple_grow(Tuple *restrict tuple) {
  Growth g        = array_growth_u64(tuple->capacity, sizeof(*tuple->elements));
  tuple->elements = reallocate(tuple->elements, g.alloc_size);
  tuple->capacity = g.new_capacity;
}

void tuple_append(Tuple *restrict tuple, Operand element) {
  if (tuple_full(tuple)) { tuple_grow(tuple); }

  tuple->elements[tuple->size] = element;
  tuple->size += 1;
}

Value value_create() {
  Value value = {.kind = VALUEKIND_UNINITIALIZED, .nil = 0};
  return value;
}

void value_destroy(Value *restrict value) {
  switch (value->kind) {
  case VALUEKIND_TUPLE: {
    tuple_destroy(&value->tuple);
    break;
  }

  // values without dynamic storage
  default: return;
  }
}

Value value_create_nil() {
  Value value = {.kind = VALUEKIND_NIL, .nil = 0};
  return value;
}

Value value_create_boolean(bool b) {
  Value value = {.kind = VALUEKIND_BOOLEAN, .boolean = b};
  return value;
}

Value value_create_i64(i64 i) {
  Value value = {.kind = VALUEKIND_I64, .integer_64 = i};
  return value;
}

Value value_create_tuple(Tuple tuple) {
  Value value = {.kind = VALUEKIND_TUPLE, .tuple = tuple};
  return value;
}

void value_assign(Value *dest, Value *source) {
  if (dest == source) { return; }

  switch (source->kind) {
  case VALUEKIND_TUPLE: {
    value_destroy(dest);
    *dest = (Value){.kind = VALUEKIND_TUPLE, .tuple = tuple_create()};
    tuple_assign(&dest->tuple, &source->tuple);
    break;
  }

  // values without dynamic storage can be struct assigned.
  default: {
    *dest = *source;
    break;
  }
  }
}

bool value_equality(Value *A, Value *B) {
  if (A == B) { return 1; }

  switch (A->kind) {
  case VALUEKIND_UNINITIALIZED: return B->kind == VALUEKIND_UNINITIALIZED;
  case VALUEKIND_NIL:           return B->kind == VALUEKIND_NIL;

  case VALUEKIND_BOOLEAN: {
    if (B->kind != VALUEKIND_BOOLEAN) { return 0; }

    return A->boolean == B->boolean;
  }

  case VALUEKIND_I64: {
    if (B->kind != VALUEKIND_I64) { return 0; }

    return A->integer_64 == B->integer_64;
  }

  case VALUEKIND_TUPLE: {
    return tuple_equal(&A->tuple, &B->tuple);
  }

  default: PANIC("bad VALUEKIND");
  }
}

static void print_tuple(Tuple const *restrict tuple, FILE *restrict file) {
  file_write("(", file);
  for (u64 i = 0; i < tuple->size; ++i) {
    print_operand(tuple->elements[i], file);

    if (i < (tuple->size - 1)) { file_write(", ", file); }
  }
  file_write(")", file);
}

void print_value(Value const *restrict v, FILE *restrict file) {
  switch (v->kind) {
  case VALUEKIND_UNINITIALIZED:
  case VALUEKIND_NIL:           file_write("()", file); break;

  case VALUEKIND_BOOLEAN: {
    (v->boolean) ? file_write("true", file) : file_write("false", file);
    break;
  }

  case VALUEKIND_I64: {
    print_i64(v->integer_64, file);
    break;
  }

  case VALUEKIND_TUPLE: {
    print_tuple(&v->tuple, file);
    break;
  }

  default: file_write("undefined", file); break;
  }
}