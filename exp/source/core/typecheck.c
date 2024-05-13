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
#include <stddef.h>
#include <stdlib.h>

#include "core/typecheck.h"
#include "env/error.h"
#include "intrinsics/type_of.h"
#include "utility/alloc.h"
#include "utility/panic.h"

typedef struct TResult {
  bool has_error;
  union {
    Error error;
    Type *type;
  };
} TResult;

static void tresult_destroy(TResult *restrict tr) {
  if (tr->has_error) {
    error_destroy(&tr->error);
  }
}

static TResult error(ErrorCode code, String msg) {
  TResult result = {.has_error = 1, .error = error_from_string(code, msg)};
  return result;
}

static TResult success(Type *type) {
  TResult result = {.has_error = 0, .type = type};
  return result;
}

typedef struct LocalTypes {
  u16 count;
  Type **buffer;
} LocalTypes;

static LocalTypes lt_create(u16 count) {
  LocalTypes lt = {.count = count, .buffer = callocate(count, sizeof(Type *))};
  return lt;
}

static void lt_destroy(LocalTypes *restrict lt) {
  lt->count = 0;
  free(lt->buffer);
  lt->buffer = NULL;
}

static Type *lt_at(LocalTypes *restrict lt, u16 local) {
  return lt->buffer[local];
}

static void lt_set(LocalTypes *restrict lt, u16 local, Type *type) {
  lt->buffer[local] = type;
}

#define try(decl, call)                                                        \
  Type *decl = NULL;                                                           \
  {                                                                            \
    TResult result = call;                                                     \
    if (result.has_error) {                                                    \
      return result;                                                           \
    }                                                                          \
    decl = result.type;                                                        \
  }                                                                            \
  assert(decl != NULL)

static TResult typecheck_operand(Context *restrict c, LocalTypes *restrict lt,
                                 OperandFormat fmt, u16 operand) {
  switch (fmt) {
  case OPRFMT_SSA: {
    Type *t = lt_at(lt, operand);
    if (t == NULL) {
      StringView sv = string_view_from_string("", 0);
      return error(ERROR_TYPECHECK_UNDEFINED_SYMBOL, string_from_view(sv));
    }

    return success(t);
  }
  case OPRFMT_CONSTANT: {
    Value *value = context_constants_at(c, operand);
    return success(type_of(value, c));
  }

  case OPRFMT_IMMEDIATE: {
    return success(context_i64_type(c));
  }

  default:
    unreachable();
  }
}

static TResult typecheck_move(Context *restrict c, LocalTypes *restrict lt,
                              Instruction I) {
  return typecheck_operand(c, lt, I.B_format, I.B);
}

static TResult typecheck_ret(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  return typecheck_operand(c, lt, I.B_format, I.B);
}

static TResult typecheck_neg(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_add(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  try(Cty, typecheck_operand(c, lt, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_sub(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  try(Cty, typecheck_operand(c, lt, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_mul(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  try(Cty, typecheck_operand(c, lt, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_div(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  try(Cty, typecheck_operand(c, lt, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_mod(Context *restrict c, LocalTypes *restrict lt,
                             Instruction I) {
  try(Bty, typecheck_operand(c, lt, I.B_format, I.B));

  try(Cty, typecheck_operand(c, lt, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_string("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  return success(Bty);
}

static TResult typecheck_function(Context *restrict c,
                                  FunctionBody *restrict body) {
  Type *return_type = NULL;
  Bytecode *bc      = &body->bc;
  LocalTypes lt     = lt_create(body->ssa_count);

  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (I.opcode) {
    case OPC_MOVE: {
      try(Bty, typecheck_move(c, &lt, I));

      lt_set(&lt, I.A, Bty);
      break;
    }

    case OPC_NEG: {
      try(Bty, typecheck_neg(c, &lt, I));

      lt_set(&lt, I.A, Bty);
      break;
    }

    case OPC_ADD: {
      try(Aty, typecheck_add(c, &lt, I));

      lt_set(&lt, I.A, Aty);
      break;
    }

    case OPC_SUB: {
      try(Aty, typecheck_sub(c, &lt, I));

      lt_set(&lt, I.A, Aty);
      break;
    }

    case OPC_MUL: {
      try(Aty, typecheck_mul(c, &lt, I));

      lt_set(&lt, I.A, Aty);
      break;
    }

    case OPC_DIV: {
      try(Aty, typecheck_div(c, &lt, I));

      lt_set(&lt, I.A, Aty);
      break;
    }

    case OPC_MOD: {
      try(Aty, typecheck_mod(c, &lt, I));

      lt_set(&lt, I.A, Aty);
      break;
    }

    case OPC_RET: {
      try(Bty, typecheck_ret(c, &lt, I));

      if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
        StringView sv = string_view_from_string("", 0);
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
      }

      return_type = Bty;
      break;
    }

    default:
      unreachable();
    }
  }

  lt_destroy(&lt);
  return success(return_type);
}

static TResult typecheck_ste(Context *restrict c,
                             SymbolTableElement *restrict element) {
  StringView name = element->name;
  switch (element->kind) {
  case STE_UNDEFINED:
    return error(ERROR_TYPECHECK_UNDEFINED_SYMBOL, string_from_view(name));

  case STE_FUNCTION: {
    FunctionBody *body = &element->function_body;
    try(Rty, typecheck_function(c, body));

    if ((body->return_type != NULL) &&
        (!type_equality(Rty, body->return_type))) {
      StringView sv = string_view_from_string("", 0);
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
    }

    body->return_type = Rty;
    return success(type_of_function(body, c));
  }

  default:
    unreachable();
  }
}

#undef try

i32 typecheck(Context *restrict context) {
  i32 result               = EXIT_SUCCESS;
  SymbolTableIterator iter = context_global_symbol_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {
    TResult tr = typecheck_ste(context, iter.element);
    if (tr.has_error) {
      error_print(&tr.error, context_source_path(context), 0);
      tresult_destroy(&tr);
      result |= EXIT_FAILURE;
    }

    symbol_table_iterator_next(&iter);
  }

  return result;
}