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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "env/context.h"
#include "imr/function.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

FormalArgumentList formal_argument_list_create() {
  FormalArgumentList fal;
  fal.capacity = 0;
  fal.size     = 0;
  fal.list     = NULL;
  return fal;
}

void formal_argument_list_destroy(FormalArgumentList *restrict fal) {
  assert(fal != NULL);
  fal->capacity = 0;
  fal->size     = 0;
  free(fal->list);
  fal->list = NULL;
}

void formal_argument_list_clone(FormalArgumentList *target,
                                FormalArgumentList *source) {
  assert(target != NULL);
  assert(source != NULL);
  if (target == source) {
    return;
  }

  formal_argument_list_destroy(target);
  target->capacity = source->capacity;
  target->size     = source->size;

  size_t alloc_size;
  if (__builtin_mul_overflow(target->capacity, sizeof(FormalArgument),
                             &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  FormalArgument *result = realloc(target->list, alloc_size);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }
  target->list = result;

  memcpy(target->list, source->list, target->size);
}

bool formal_argument_list_equality(FormalArgumentList *a1,
                                   FormalArgumentList *a2) {
  assert(a1 != NULL);
  assert(a2 != NULL);
  if (a1 == a2) {
    return 1;
  }

  if (a1->size != a2->size) {
    return 0;
  }

  for (size_t i = 0; i < a1->size; ++i) {
    FormalArgument *arg1 = &a1->list[i];
    FormalArgument *arg2 = &a2->list[i];

    if (!string_view_equality(arg1->name, arg2->name)) {
      return 0;
    }

    if (arg1->type != arg2->type) {
      return 0;
    }
  }

  return 1;
}

static bool formal_argument_list_full(FormalArgumentList *restrict fal) {
  size_t new_size;
  if (__builtin_add_overflow(fal->size, 1, &new_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  if (new_size == SIZE_MAX) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  return new_size >= fal->capacity;
}

static void formal_argument_list_grow(FormalArgumentList *restrict fal) {
  size_t new_capacity = nearest_power_of_two(fal->capacity + 1);

  size_t alloc_size;
  if (__builtin_mul_overflow(new_capacity, sizeof(FormalArgument),
                             &alloc_size)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  FormalArgument *result = realloc(fal->list, alloc_size);
  if (result == NULL) {
    PANIC_ERRNO("realloc failed");
  }

  fal->capacity = new_capacity;
  fal->list     = result;
}

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 StringView name, Type *type) {
  assert(fal != NULL);

  if (formal_argument_list_full(fal)) {
    formal_argument_list_grow(fal);
  }

  fal->list[fal->size] = (FormalArgument){name, type};
  fal->size += 1;
}

Function function_create() {
  Function function;
  function.name        = string_view_create();
  function.arguments   = formal_argument_list_create();
  function.return_type = NULL;
  function.body        = bytecode_create();
  return function;
}

void function_destroy(Function *restrict function) {
  assert(function != NULL);
  formal_argument_list_destroy(&function->arguments);
  bytecode_destroy(&function->body);
}

void function_clone(Function *target, Function *source) {
  assert(target != NULL);
  assert(source != NULL);
  if (target == source) {
    return;
  }

  target->name = source->name;
  formal_argument_list_clone(&target->arguments, &source->arguments);
  bytecode_clone(&target->body, &source->body);
}

bool function_equality(Function *f1, Function *f2) {
  assert(f1 != NULL);
  assert(f2 != NULL);
  if (f1 == f2) {
    return 1;
  }

  if (!formal_argument_list_equality(&f1->arguments, &f2->arguments)) {
    return 0;
  }

  return bytecode_equality(&f1->body, &f2->body);
}

Type *function_type_of(Function const *restrict f, Context *restrict context) {
  assert(f != NULL);
  assert(f->return_type != NULL);

  ArgumentTypes argument_types = argument_types_create();
  for (size_t i = 0; i < f->arguments.size; ++i) {
    FormalArgument *formal_argument = &f->arguments.list[i];
    Type *argument_type             = formal_argument->type;
    argument_types_append(&argument_types, argument_type);
  }

  return context_function_type(context, f->return_type, argument_types);
}

void function_add_argument(Function *restrict function, StringView name,
                           Type *type) {
  assert(function != NULL);

  formal_argument_list_append(&function->arguments, name, type);
}