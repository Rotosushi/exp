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
#include "env/error.h"
#include "imr/opcode.h"
#include "imr/value.h"
#include "utility/panic.h"

static MaybeError interpret_impl(Context *restrict context) {
#define READBYTE() (*ip++)
#define CURIDX() (size_t)(ip - context->global_bytecode.buffer)
  uint8_t *ip = context->global_bytecode.buffer;

  while (1) {
    switch ((Opcode)READBYTE()) {
    case OP_STOP: {
      return success();
    }

    case OP_POP: {
      context_stack_pop(context);
      break;
    }

    case OP_PUSH_CONSTANT_U8: {
      size_t index = context_read_immediate(context, CURIDX(), sizeof(uint8_t));
      ip += sizeof(uint8_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, *constant);
      break;
    }

    case OP_PUSH_CONSTANT_U16: {
      size_t index =
          context_read_immediate(context, CURIDX(), sizeof(uint16_t));
      ip += sizeof(uint16_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, *constant);
      break;
    }

    case OP_PUSH_CONSTANT_U32: {
      size_t index =
          context_read_immediate(context, CURIDX(), sizeof(uint32_t));
      ip += sizeof(uint32_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, *constant);
      break;
    }

    case OP_PUSH_CONSTANT_U64: {
      size_t index =
          context_read_immediate(context, CURIDX(), sizeof(uint64_t));
      ip += sizeof(uint64_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, *constant);
      break;
    }

    case OP_DEFINE_GLOBAL_CONSTANT: {
      Value constant = context_stack_pop(context);
      Type *type     = type_of(&constant, context);
      Value name     = context_stack_pop(context);
      assert(VALUE_IS(name, VALUEKIND_STRING_LITERAL));
      context_insert_global_symbol(context, name.string_literal, type,
                                   constant);
      break;
    }

    case OP_UNOP_MINUS: {
      Value *top = context_stack_peek(context);
      Type *type = type_of(top, context);
      if (type != context_integer_type(context)) {
        return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(type));
      }
      break;
    }

    default: {
      PANIC("Unknown Opcode");
    }
    }
  }

#undef READBYTE
#undef CURIDX
}

int interpret(Context *restrict context) {
  MaybeError maybe = interpret_impl(context);
  if (maybe.has_error) {
    error_print(&maybe.error, __FILE__, __LINE__);

    maybe_error_destroy(&maybe);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}