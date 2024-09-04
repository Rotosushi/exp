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

#include "backend/x64/context.h"
#include "backend/x64/function_body.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/panic.h"

x64_FormalArgumentList x64_formal_argument_list_create(u8 size) {
  x64_FormalArgumentList args = {
      .size = size, .buffer = allocate(size * sizeof(x64_FormalArgument))};
  return args;
}

static void
x64_formal_arguments_destroy(x64_FormalArgumentList *restrict args) {
  args->size = 0;
  deallocate(args->buffer);
  args->buffer = NULL;
}

x64_FormalArgument *
x64_formal_argument_list_at(x64_FormalArgumentList *restrict args, u8 idx) {
  assert(args != NULL);
  assert(idx < args->size);
  return args->buffer + idx;
}

x64_FunctionBody x64_function_body_create(FunctionBody *restrict body,
                                          x64_Context *restrict context) {
  x64_FunctionBody x64_body = {
      .arguments = x64_formal_argument_list_create(body->arguments.size),
      .result    = NULL,
      .bc        = x64_bytecode_create(),
      .allocator = x64_allocator_create(body, context->context)};

  u8 scalar_argument_count = 0;

  if (type_is_scalar(body->return_type)) {
    x64_body.result = x64_context_allocate_result(
        context, x64_location_gpr(X64GPR_RAX), body->return_type);
  } else {
    x64_body.result = x64_context_allocate_result(
        context,
        x64_location_address(X64GPR_RDI,
                             x64_optional_gpr_empty(),
                             x64_optional_u8_empty(),
                             x64_optional_i64_empty()),
        body->return_type);
    scalar_argument_count += 1;
  }

  i64 offset = 16;
  for (u8 i = 0; i < body->arguments.size; ++i) {
    FormalArgument *arg  = body->arguments.list + i;
    LocalVariable *local = x64_context_lookup_ssa(context, arg->ssa);

    if ((scalar_argument_count < 6) && type_is_scalar(local->type)) {
      x64_GPR gpr = x64_scalar_argument_gpr(scalar_argument_count++);
      x64_context_allocate_to_gpr(context, local, gpr, 0);
    } else {
      u64 argument_size = size_of(arg->type);
      assert(argument_size <= i64_MAX);

      x64_context_allocate_to_stack(context, local, offset);

      if (__builtin_add_overflow(offset, (i64)argument_size, &offset)) {
        PANIC("argument offset overflow");
      }
    }
  }

  return x64_body;
}

void x64_function_body_destroy(x64_FunctionBody *restrict body) {
  assert(body != NULL);
  x64_formal_arguments_destroy(&body->arguments);
  x64_bytecode_destroy(&body->bc);
  x64_allocator_destroy(&body->allocator);
}
