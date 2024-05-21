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

#include "backend/x64_function_body.h"
#include "utility/alloc.h"

X64FormalArguments x64formal_arguments_create(u8 size) {
  X64FormalArguments args = {
      .size = size, .buffer = allocate(size * sizeof(X64FormalArgument))};
  return args;
}

static void x64formal_arguments_destroy(X64FormalArguments *restrict args) {
  args->size = 0;
  free(args->buffer);
  args->buffer = NULL;
}

X64FormalArgument *x64formal_arguments_at(X64FormalArguments *restrict args,
                                          u8 idx) {
  assert(args != NULL);
  assert(idx < args->size);
  return args->buffer + idx;
}

void x64formal_arguments_assign(X64FormalArguments *restrict args,
                                u8 idx,
                                X64Allocation allocation) {
  assert(args != NULL);
  X64FormalArgument *arg = x64formal_arguments_at(args, idx);
  arg->allocation        = allocation;
}

X64FunctionBody x64function_body_create(u8 arg_count) {
  X64FunctionBody x64body = {.arguments = x64formal_arguments_create(arg_count),
                             .return_allocation = x64allocation_reg(X64GPR_RAX),
                             .bc                = x64bytecode_create()};
  return x64body;
}

void x64function_body_destroy(X64FunctionBody *restrict body) {
  assert(body != NULL);
  x64formal_arguments_destroy(&body->arguments);
  x64bytecode_destroy(&body->bc);
}