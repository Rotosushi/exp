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

static bool formal_argument_list_full(FormalArgumentList *restrict fal) {
  return (fal->size + 1) >= fal->capacity;
}

static void formal_argument_list_grow(FormalArgumentList *restrict fal) {
  Growth g      = array_growth_u8(fal->capacity, sizeof(FormalArgument));
  fal->list     = reallocate(fal->list, g.alloc_size);
  fal->capacity = (u8)g.new_capacity;
}

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 FormalArgument arg) {
  assert(fal != NULL);

  if (formal_argument_list_full(fal)) { formal_argument_list_grow(fal); }

  fal->list[fal->size] = arg;
  fal->size += 1;
}

LocalVariables local_variables_create() {
  LocalVariables lv = {.size = 0, .capacity = 0, .buffer = NULL};
  return lv;
}

static void local_variables_destroy(LocalVariables *restrict lv) {
  lv->size     = 0;
  lv->capacity = 0;
  free(lv->buffer);
  lv->buffer = NULL;
}

static bool local_variables_full(LocalVariables *restrict lv) {
  return (lv->size + 1) >= lv->capacity;
}

static void local_variables_grow(LocalVariables *restrict lv) {
  Growth g     = array_growth_u16(lv->capacity, sizeof(LocalVariable));
  lv->buffer   = reallocate(lv->buffer, g.alloc_size);
  lv->capacity = (u16)g.new_capacity;
}

void local_variables_append(LocalVariables *restrict lv, LocalVariable var) {
  if (local_variables_full(lv)) { local_variables_grow(lv); }

  lv->buffer[lv->size] = var;
  lv->size += 1;
}

LocalVariable *local_variables_lookup(LocalVariables *restrict lv,
                                      StringView name) {
  for (u16 i = 0; i < lv->size; ++i) {
    LocalVariable *var = lv->buffer + i;
    if (string_view_eq(var->name, name)) { return var; }
  }
  return NULL;
}

FunctionBody function_body_create() {
  FunctionBody function;
  function.arguments   = formal_argument_list_create();
  function.locals      = local_variables_create();
  function.return_type = NULL;
  function.bc          = bytecode_create();
  function.ssa_count   = 0;
  return function;
}

void function_body_destroy(FunctionBody *restrict function) {
  assert(function != NULL);
  formal_argument_list_destroy(&function->arguments);
  local_variables_destroy(&function->locals);
  bytecode_destroy(&function->bc);
  function->return_type = NULL;
  function->ssa_count   = 0;
}

static void print_formal_argument(FormalArgument *arg, FILE *restrict file) {
  file_write(arg->name.ptr, file);
  file_write(": ", file);
  print_type(arg->type, file);
}

void print_function_body(FunctionBody const *restrict f, FILE *restrict file) {
  file_write("(", file);
  FormalArgumentList const *args = &f->arguments;
  for (u8 i = 0; i < args->size; ++i) {
    print_formal_argument(args->list + i, file);

    if (i < (u8)(args->size - 1)) { file_write(", ", file); }
  }
  file_write(")\n", file);
  print_bytecode(&f->bc, file);
}