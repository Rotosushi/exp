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

#include "core/interpret.h"
#include "imr/opcode.h"
#include "imr/value.h"
#include "utility/panic.h"

int interpret(Context *restrict context) {
#define READBYTE() (*ip++)
  uint8_t *ip = context->global_bytecode.buffer;

  while (1) {
    switch ((Opcode)READBYTE()) {
    case OP_STOP: {
      return EXIT_SUCCESS;
    }

    case OP_POP: {
      context_stack_pop(context);
      break;
    }

    case OP_PUSH_CONSTANT: {
      uint8_t index = READBYTE();
      Value *constant = context_constants_at(context, index);
      context_stack_push(context, *constant);
      break;
    }

    case OP_DEFINE_GLOBAL_CONSTANT: {
      Value constant = context_stack_pop(context);
      Type *type = type_of(&constant, context);
      Value name = context_stack_pop(context);
      assert(VALUE_IS(name, VALUEKIND_STRING_LITERAL));
      context_insert_global_symbol(context, name.string_literal, type,
                                   constant);
      break;
    }

    default: {
      panic("Unknown Opcode");
    }
    }
  }

#undef READBYTE
}