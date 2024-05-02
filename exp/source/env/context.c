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

#include "env/context.h"
#include "utility/io.h"
#include "utility/nearest_power.h"
#include "utility/panic.h"

Context context_create(CLIOptions *restrict options) {
  assert(options != NULL);
  Context context = {.options         = context_options_create(options),
                     .string_interner = string_interner_create(),
                     .type_interner   = type_interner_create(),
                     .global_symbols  = symbol_table_create(),
                     .constants       = constants_create()};
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbols));
  constants_destroy(&(context->constants));
}

bool context_do_assemble(Context *restrict context) {
  assert(context != NULL);
  return context_options_do_assemble(&context->options);
}

bool context_do_link(Context *restrict context) {
  assert(context != NULL);
  return context_options_do_link(&context->options);
}

bool context_do_cleanup(Context *restrict context) {
  assert(context != NULL);
  return context_options_do_cleanup(&context->options);
}

StringView context_source_path(Context *restrict context) {
  assert(context != NULL);
  return string_to_view(&(context->options.source));
}

StringView context_assembly_path(Context *restrict context) {
  assert(context != NULL);
  return string_to_view(&context->options.assembly);
}

StringView context_object_path(Context *restrict context) {
  assert(context != NULL);
  return string_to_view(&context->options.object);
}

StringView context_output_path(Context *restrict context) {
  assert(context != NULL);
  return string_to_view(&(context->options.output));
}

StringView context_intern(Context *restrict context, StringView sv) {
  assert(context != NULL);
  return string_interner_insert(&(context->string_interner), sv.ptr, sv.length);
}

Type *context_nil_type(Context *restrict context) {
  assert(context != NULL);
  return type_interner_nil_type(&(context->type_interner));
}

Type *context_boolean_type(Context *restrict context) {
  assert(context != NULL);
  return type_interner_boolean_type(&(context->type_interner));
}

Type *context_i64_type(Context *restrict context) {
  assert(context != NULL);
  return type_interner_i64_type(&(context->type_interner));
}

Type *context_function_type(Context *restrict context, Type *return_type,
                            ArgumentTypes argument_types) {
  assert(context != NULL);
  return type_interner_function_type(&context->type_interner, return_type,
                                     argument_types);
}

SymbolTableElement *context_global_symbols_at(Context *restrict context,
                                              StringView name) {
  assert(context != NULL);
  return symbol_table_at(&context->global_symbols, name);
}

SymbolTableIterator context_global_symbol_iterator(Context *restrict context) {
  assert(context != NULL);
  return symbol_table_iterator_create(&context->global_symbols);
}

FunctionBody *context_enter_function(Context *restrict c, StringView name) {
  assert(c != NULL);
  SymbolTableElement *element = symbol_table_at(&c->global_symbols, name);
  element->kind               = STE_FUNCTION;
  c->current_function         = &element->function_body;
  return c->current_function;
}

void context_leave_function(Context *restrict c) {
  assert(c != NULL);
  c->current_function = NULL;
}

static Bytecode *context_active_bytecode(Context *restrict c) {
  assert(c->current_function != NULL);
  return &c->current_function->bc;
}

static Operand context_new_local(Context *restrict c) {
  assert(c->current_function != NULL);
  return opr_ssa(c->current_function->local_count++);
}

Operand context_constants_add(Context *restrict context, Value value) {
  assert(context != NULL);
  return constants_add(&(context->constants), value);
}

Value *context_constants_at(Context *restrict context, u16 index) {
  assert(context != NULL);
  return constants_at(&(context->constants), index);
}

static FoldResult success(Operand O) {
  FoldResult result = {.has_error = 0, .operand = O};
  return result;
}

static FoldResult error(ErrorCode code, StringView sv) {
  FoldResult result = {.has_error = 1, .error = error_from_view(code, sv)};
  return result;
}

void fresult_destroy(FoldResult *restrict fr) {
  if (fr->has_error) {
    error_destroy(&fr->error);
  }
}

void context_emit_return(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  bytecode_emit_return(bc, B);
}

Operand context_emit_move(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_move(bc, A, B);
  return A;
}

FoldResult context_emit_neg(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_neg(bc, A, B);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *v = context_constants_at(c, B.common);
    if (v->kind == VALUEKIND_I64) {
      i64 n = -(v->integer);
      A     = context_constants_add(c, value_create_i64(n));
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 n = -((i64)(B.common));
    if ((n > i16_MAX) || (n < i16_MIN)) {
      A = context_constants_add(c, value_create_i64(n));
    } else {
      A = opr_immediate((u16)n);
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}

FoldResult context_emit_add(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_add(bc, A, B, C);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    i64 x     = 0;
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_add(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_add_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_add_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 x = (i64)B.common;

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_add(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_add_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_add_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      if ((z > i16_MIN) && (z < i16_MAX)) {
        A = opr_immediate((u16)z);
      } else {
        A = context_constants_add(c, value_create_i64(z));
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}

FoldResult context_emit_sub(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_sub(bc, A, B, C);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    i64 x     = 0;
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_sub(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_sub_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_sub_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 x = (i64)B.common;

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_sub(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_sub_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_sub_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      if ((z > i16_MIN) && (z < i16_MAX)) {
        A = opr_immediate((u16)z);
      } else {
        A = context_constants_add(c, value_create_i64(z));
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}

FoldResult context_emit_mul(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_mul(bc, A, B, C);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    i64 x     = 0;
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_mul(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_mul_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_mul_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 x = (i64)B.common;

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_mul(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = 0;
      if (__builtin_mul_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = 0;
      if (__builtin_mul_overflow(x, y, &z)) {
        return error(ERROR_PARSER_INTEGER_TO_LARGE,
                     string_view_from_cstring(""));
      }

      if ((z > i16_MIN) && (z < i16_MAX)) {
        A = opr_immediate((u16)z);
      } else {
        A = context_constants_add(c, value_create_i64(z));
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}

FoldResult context_emit_div(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_div(bc, A, B, C);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    i64 x     = 0;
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_div(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = x / y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = x / y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 x = (i64)B.common;

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_div(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = x / y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = x / y;

      if ((z > i16_MIN) && (z < i16_MAX)) {
        A = opr_immediate((u16)z);
      } else {
        A = context_constants_add(c, value_create_i64(z));
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}

FoldResult context_emit_mod(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_local(c);
    bytecode_emit_mod(bc, A, B, C);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    i64 x     = 0;
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_mod(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = x % y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = x % y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 x = (i64)B.common;

    switch (C.format) {
    case OPRFMT_SSA: {
      A = context_new_local(c);
      bytecode_emit_mod(bc, A, B, C);
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cv = context_constants_at(c, C.common);
      i64 y     = 0;
      if (Cv->kind == VALUEKIND_I64) {
        y = Cv->integer;
      } else {
        return error(ERROR_TYPECHECK_TYPE_MISMATCH,
                     string_view_from_cstring(""));
      }

      i64 z = x % y;

      A = context_constants_add(c, value_create_i64(z));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      i64 y = (i64)C.common;

      i64 z = x % y;

      if ((z > i16_MIN) && (z < i16_MAX)) {
        A = opr_immediate((u16)z);
      } else {
        A = context_constants_add(c, value_create_i64(z));
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }
  return success(A);
}
