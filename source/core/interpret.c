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

static MaybeError unop_minus(Context *restrict context) {
  Value *top = context_stack_peek(context);
  Type *type = type_of(top, context);
  if (type != context_integer_type(context)) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(type));
  }
  top->integer = -(top->integer);
  return success();
}

static MaybeError binop_plus(Context *restrict context) {
  Value *b           = context_stack_pop(context);
  Type *b_type       = type_of(b, context);
  Type *integer_type = context_integer_type(context);
  if (b_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(b_type));
  }

  Value *a     = context_stack_peek(context);
  Type *a_type = type_of(a, context);
  if (a_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(a_type));
  }

  a->integer += b->integer;
  return success();
}

static MaybeError binop_minus(Context *restrict context) {
  Value *b           = context_stack_pop(context);
  Type *b_type       = type_of(b, context);
  Type *integer_type = context_integer_type(context);
  if (b_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(b_type));
  }

  Value *a     = context_stack_peek(context);
  Type *a_type = type_of(a, context);
  if (a_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(a_type));
  }

  a->integer -= b->integer;
  return success();
}

static MaybeError binop_star(Context *restrict context) {
  Value *b           = context_stack_pop(context);
  Type *b_type       = type_of(b, context);
  Type *integer_type = context_integer_type(context);
  if (b_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(b_type));
  }

  Value *a     = context_stack_peek(context);
  Type *a_type = type_of(a, context);
  if (a_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(a_type));
  }

  a->integer *= b->integer;
  return success();
}

static MaybeError binop_slash(Context *restrict context) {
  Value *b           = context_stack_pop(context);
  Type *b_type       = type_of(b, context);
  Type *integer_type = context_integer_type(context);
  if (b_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(b_type));
  }

  Value *a     = context_stack_peek(context);
  Type *a_type = type_of(a, context);
  if (a_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(a_type));
  }

  a->integer /= b->integer;
  return success();
}

static MaybeError binop_percent(Context *restrict context) {
  Value *b           = context_stack_pop(context);
  Type *b_type       = type_of(b, context);
  Type *integer_type = context_integer_type(context);
  if (b_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(b_type));
  }

  Value *a     = context_stack_peek(context);
  Type *a_type = type_of(a, context);
  if (a_type != integer_type) {
    return error(ERROR_INTERPRET_EXPECTED_TYPE_INT, type_to_string(a_type));
  }

  a->integer %= b->integer;
  return success();
}

static MaybeError interpret_impl(Context *restrict context) {
#define READBYTE() (*ip++)
#define CURIDX() (u64)(ip - context->global_bytecode.buffer)
  u8 *ip = context->global_bytecode.buffer;

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
      u64 index = context_read_immediate(context, CURIDX(), sizeof(u8));
      ip += sizeof(u8);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, constant);
      break;
    }

    case OP_PUSH_CONSTANT_U16: {
      u64 index = context_read_immediate(context, CURIDX(), sizeof(uint16_t));
      ip += sizeof(uint16_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, constant);
      break;
    }

    case OP_PUSH_CONSTANT_U32: {
      u64 index = context_read_immediate(context, CURIDX(), sizeof(uint32_t));
      ip += sizeof(uint32_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, constant);
      break;
    }

    case OP_PUSH_CONSTANT_U64: {
      u64 index = context_read_immediate(context, CURIDX(), sizeof(uint64_t));
      ip += sizeof(uint64_t);

      Value *constant = context_constants_at(context, index);
      context_stack_push(context, constant);
      break;
    }

    case OP_DEFINE_GLOBAL_CONSTANT: {
      Value *constant = context_stack_pop(context);
      Type *type      = type_of(constant, context);
      Value *name     = context_stack_pop(context);
      assert(name->kind == VALUEKIND_STRING_LITERAL);
      context_insert_global_symbol(context, name->string_literal, type,
                                   constant);
      break;
    }

    // #TODO:
    case OP_RETURN: {
      break;
    }

    case OP_UNOP_MINUS: {
      MaybeError maybe = unop_minus(context);
      if (maybe.has_error) {
        return maybe;
      }
      break;
    }

    case OP_BINOP_PLUS: {
      MaybeError maybe = binop_plus(context);
      if (maybe.has_error) {
        return maybe;
      }
      break;
    }

    case OP_BINOP_MINUS: {
      MaybeError maybe = binop_minus(context);
      if (maybe.has_error) {
        return maybe;
      }
      break;
    }

    case OP_BINOP_STAR: {
      MaybeError maybe = binop_star(context);
      if (maybe.has_error) {
        return maybe;
      }
      break;
    }

    case OP_BINOP_SLASH: {
      MaybeError maybe = binop_slash(context);
      if (maybe.has_error) {
        return maybe;
      }
      break;
    }

    case OP_BINOP_PERCENT: {
      MaybeError maybe = binop_percent(context);
      if (maybe.has_error) {
        return maybe;
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

i32 interpret(Context *restrict context) {
  MaybeError maybe = interpret_impl(context);
  if (maybe.has_error) {
    error_print(&maybe.error, __FILE__, __LINE__);

    maybe_error_destroy(&maybe);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}