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
#include "imr/function_body.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
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

  Growth g = array_growth(target->capacity, sizeof(FormalArgument));
  FormalArgument *result = reallocate(target->list, g.alloc_size);
  target->list           = result;

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

  for (u64 i = 0; i < a1->size; ++i) {
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
  return (fal->size + 1) >= fal->capacity;
}

static void formal_argument_list_grow(FormalArgumentList *restrict fal) {
  Growth g      = array_growth(fal->capacity, sizeof(FormalArgument));
  fal->list     = reallocate(fal->list, g.alloc_size);
  fal->capacity = g.new_capacity;
}

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 FormalArgument arg) {
  assert(fal != NULL);

  if (formal_argument_list_full(fal)) {
    formal_argument_list_grow(fal);
  }

  fal->list[fal->size] = arg;
  fal->size += 1;
}

FunctionBody function_body_create() {
  FunctionBody function;
  function.arguments   = formal_argument_list_create();
  function.return_type = NULL;
  function.bc          = bytecode_create();
  return function;
}

void function_body_destroy(FunctionBody *restrict function) {
  assert(function != NULL);
  formal_argument_list_destroy(&function->arguments);
  bytecode_destroy(&function->bc);
}

Type *function_body_type_of(FunctionBody const *restrict f,
                            Context *restrict context) {
  assert(f != NULL);
  assert(f->return_type != NULL);

  ArgumentTypes argument_types = argument_types_create();
  for (u64 i = 0; i < f->arguments.size; ++i) {
    FormalArgument *formal_argument = &f->arguments.list[i];
    Type *argument_type             = formal_argument->type;
    argument_types_append(&argument_types, argument_type);
  }

  return context_function_type(context, f->return_type, argument_types);
}
