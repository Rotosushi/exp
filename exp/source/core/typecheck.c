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

typedef struct TResult {
  bool has_error;
  union {
    Error error;
    Type *type;
  };
} TResult;

static void tresult_destroy(TResult *restrict tr) {
  if (tr->has_error) { error_destroy(&tr->error); }
}

static TResult error(ErrorCode code, String msg) {
  TResult result = {.has_error = 1, .error = error_from_string(code, msg)};
  return result;
}

static TResult success(Type *type) {
  TResult result = {.has_error = 0, .type = type};
  return result;
}

#define try(decl, call)                                                        \
  Type *decl = NULL;                                                           \
  {                                                                            \
    TResult result = call;                                                     \
    if (result.has_error) { return result; }                                   \
    decl = result.type;                                                        \
  }                                                                            \
  assert(decl != NULL)

static TResult typecheck_global(Context *restrict c,
                                SymbolTableElement *restrict element);

static TResult typecheck_operand(Context *restrict c, Operand operand) {
  switch (operand.format) {
  case OPRFMT_SSA: {
    LocalVariable *local = context_lookup_ssa(c, operand.ssa);
    Type *type           = local->type;
    if (type == NULL) {
      StringView sv = string_view_from_str("", 0);
      return error(ERROR_TYPECHECK_UNDEFINED_SYMBOL, string_from_view(sv));
    }

    return success(type);
  }

  case OPRFMT_VALUE: {
    Value *value = context_constants_at(c, operand.index);
    return success(type_of_value(value, c));
  }

  case OPRFMT_IMMEDIATE: {
    return success(context_i64_type(c));
  }

  case OPRFMT_LABEL: {
    StringView name            = context_global_labels_at(c, operand.index);
    SymbolTableElement *global = context_global_symbol_table_at(c, name);
    Type *type                 = global->type;
    if (type == NULL) {
      try(Gty, typecheck_global(c, global));
      type = Gty;
    }

    return success(type);
  }

  default: unreachable();
  }
}

static TResult typecheck_load(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));
  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_ret(Context *restrict c, Instruction I) {
  return typecheck_operand(c, I.B);
}

static TResult typecheck_call(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  if (Bty->kind != TYPEKIND_FUNCTION) {
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(SV("")));
  }

  FunctionType *function_type     = &Bty->function_type;
  TupleType *formal_types         = &function_type->argument_types;
  ActualArgumentList *actual_args = context_call_at(c, I.C.index);

  if (formal_types->size != actual_args->size) {
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(SV("")));
  }

  for (u8 i = 0; i < actual_args->size; ++i) {
    Type *formal_type = formal_types->types[i];
    Operand operand   = actual_args->list[i];
    try(actual_type, typecheck_operand(c, operand));

    if (!type_equality(actual_type, formal_type)) {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(SV("")));
    }
  }

  local->type = function_type->return_type;
  return success(function_type->return_type);
}

static TResult typecheck_neg(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_add(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  try(Cty, typecheck_operand(c, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_sub(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  try(Cty, typecheck_operand(c, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_mul(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  try(Cty, typecheck_operand(c, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_div(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  try(Cty, typecheck_operand(c, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_mod(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A);
  try(Bty, typecheck_operand(c, I.B));

  try(Cty, typecheck_operand(c, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  if (!type_equality(Bty, Cty)) {
    StringView sv = string_view_from_str("", 0);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_function(Context *restrict c) {
  Type *return_type  = NULL;
  FunctionBody *body = context_current_function(c);
  Bytecode *bc       = &body->bc;

  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (I.opcode) {
    case OPC_RET: {
      try(Bty, typecheck_ret(c, I));

      if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
        StringView sv = string_view_from_str("", 0);
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_from_view(sv));
      }

      return_type = Bty;
      break;
    }

    case OPC_CALL: {
      try(Aty, typecheck_call(c, I));
      break;
    }

    case OPC_LOAD: {
      try(Bty, typecheck_load(c, I));
      break;
    }

    case OPC_NEG: {
      try(Bty, typecheck_neg(c, I));
      break;
    }

    case OPC_ADD: {
      try(Aty, typecheck_add(c, I));
      break;
    }

    case OPC_SUB: {
      try(Aty, typecheck_sub(c, I));
      break;
    }

    case OPC_MUL: {
      try(Aty, typecheck_mul(c, I));
      break;
    }

    case OPC_DIV: {
      try(Aty, typecheck_div(c, I));
      break;
    }

    case OPC_MOD: {
      try(Aty, typecheck_mod(c, I));
      break;
    }

    default: unreachable();
    }
  }

  return success(return_type);
}

static TResult typecheck_global(Context *restrict c,
                                SymbolTableElement *restrict element) {
  if (element->type != NULL) { return success(element->type); }

  switch (element->kind) {
  case STE_UNDEFINED: {
    // #TODO: this should be handled as a forward declaration
    // but only if the type exists.
    return success(context_nil_type(c));
  }

  case STE_FUNCTION: {
    // we want to avoid infinite recursion. but we also need to
    // handle the fact that functions are going to be typechecked
    // in an indeterminite order. the natural solution is to type
    // the dependencies of a function body as those are used within
    // the function body. This only breaks when we have mutual recursion,
    // otherwise, when the global is successfully typed.
    // the question is, how do we accomplish this?
    FunctionBody *body = context_enter_function(c, element->name);

    try(Rty, typecheck_function(c));
    context_leave_function(c);

    if ((body->return_type != NULL) &&
        (!type_equality(Rty, body->return_type))) {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_create());
    }

    body->return_type   = Rty;
    Type *function_type = type_of_function(body, c);
    element->type       = function_type;
    return success(function_type);
  }

  default: unreachable();
  }
}

#undef try

i32 typecheck(Context *restrict context) {
  i32 result               = EXIT_SUCCESS;
  SymbolTableIterator iter = context_global_symbol_table_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {
    TResult tr = typecheck_global(context, iter.element);
    if (tr.has_error) {
      error_print(&tr.error, context_source_path(context), 0);
      tresult_destroy(&tr);
      result |= EXIT_FAILURE;
    }

    symbol_table_iterator_next(&iter);
  }

  return result;
}
