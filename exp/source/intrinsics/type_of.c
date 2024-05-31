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

#include "intrinsics/type_of.h"
#include "utility/panic.h"

Type *type_of(Value *restrict value, Context *restrict context) {
  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED: PANIC("uninitialized Value");
  case VALUEKIND_NIL:           return context_nil_type(context);
  case VALUEKIND_BOOLEAN:       return context_boolean_type(context);
  case VALUEKIND_I64:           return context_i64_type(context);

  default: PANIC("bad VALUEKIND");
  }
}

Type *type_of_function(FunctionBody *restrict body, Context *restrict context) {
  assert(body != NULL);
  assert(body->return_type != NULL);

  ArgumentTypes argument_types = argument_types_create();
  for (u64 i = 0; i < body->arguments.size; ++i) {
    FormalArgument *formal_argument = &body->arguments.list[i];
    Type *argument_type             = formal_argument->type;
    argument_types_append(&argument_types, argument_type);
  }

  return context_function_type(context, body->return_type, argument_types);
}
