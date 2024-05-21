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

#include "backend/x64/function_body.h"
#include "utility/alloc.h"

x64_FormalArgumentList x64_formal_argument_list_create(u8 size) {
  x64_FormalArgumentList args = {
      .size = size, .buffer = allocate(size * sizeof(x64_FormalArgument))};
  return args;
}

static void x64formal_arguments_destroy(x64_FormalArgumentList *restrict args) {
  args->size = 0;
  free(args->buffer);
  args->buffer = NULL;
}

x64_FormalArgument *
x64_formal_argument_list_at(x64_FormalArgumentList *restrict args, u8 idx) {
  assert(args != NULL);
  assert(idx < args->size);
  return args->buffer + idx;
}

void x64_formal_argument_list_assign(x64_FormalArgumentList *restrict args,
                                     u8 idx,
                                     x64_Location allocation) {
  assert(args != NULL);
  x64_FormalArgument *arg = x64_formal_argument_list_at(args, idx);
  arg->allocation         = allocation;
}

x64_FunctionBody x64_function_body_create(u8 arg_count) {
  x64_FunctionBody x64body = {.arguments =
                                  x64_formal_argument_list_create(arg_count),
                              .return_allocation = x64_location_reg(X64GPR_RAX),
                              .bc                = x64_bytecode_create()};
  return x64body;
}

void x64_function_body_destroy(x64_FunctionBody *restrict body) {
  assert(body != NULL);
  x64formal_arguments_destroy(&body->arguments);
  x64_bytecode_destroy(&body->bc);
}