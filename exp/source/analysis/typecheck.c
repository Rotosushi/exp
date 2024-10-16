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

#include "analysis/typecheck.h"
#include "env/error.h"
#include "intrinsics/type_of.h"
#include "utility/unreachable.h"

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

static TResult failure(ErrorCode code, String msg, SourceLocation location) {
  TResult result = {.has_error = 1, .error = error(code, msg, location)};
  return result;
}

static TResult semantic_error(String msg, SourceLocation location) {
  return failure(ERROR_SEMANTICS, msg, location);
}

static TResult undefined_symbol(StringView symbol, SourceLocation location) {
  String msg = string_create();
  string_append(&msg, SV("symbol ["));
  string_append(&msg, symbol);
  string_append(&msg, SV("] is not defined at the current scope"));
  return semantic_error(msg, location);
}

static TResult expected_return(SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("expected a return statement"));
  return semantic_error(buf, location);
}

static TResult
expected_type(Type *expected, Type *actual, SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("Expected ["));
  print_type(expected, &buf);
  string_append(&buf, SV("] Actual ["));
  print_type(actual, &buf);
  string_append(&buf, SV("]"));
  return semantic_error(buf, location);
}

static TResult type_not_callable(Type *type, SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("Type is not callable ["));
  print_type(type, &buf);
  string_append(&buf, SV("]"));
  return semantic_error(buf, location);
}

static TResult type_not_indexable(Type *type, SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("Type is indexable ["));
  print_type(type, &buf);
  string_append(&buf, SV("]"));
  return semantic_error(buf, location);
}

static TResult index_not_comptime(SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("index is not known at comptime"));
  return semantic_error(buf, location);
}

static TResult
index_out_of_bounds(i64 index, u64 bound, SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("The given index "));
  string_append_i64(&buf, index);
  string_append(&buf, SV(" is not in the valid range of 0.."));
  string_append_u64(&buf, bound);
  return semantic_error(buf, location);
}

static TResult
args_size_mismatch(u64 formal, u64 actual, SourceLocation location) {
  String buf = string_create();
  string_append(&buf, SV("Expected "));
  string_append_u64(&buf, formal);
  string_append(&buf, SV(" arguments, have "));
  string_append_u64(&buf, actual);
  return semantic_error(buf, location);
}

static TResult
arg_type_mismatch(Type *formal, Type *actual, SourceLocation location) {
  return expected_type(formal, actual, location);
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

static SourceLocation curloc(Context *restrict c, u64 Idx) {
  return context_lookup_source_location(c, Idx);
}

static TResult typecheck_operand(Context *restrict c,
                                 u64 Idx,
                                 OperandFormat format,
                                 OperandValue operand) {
  switch (format) {
  case OPRFMT_SSA: {
    LocalVariable *local = context_lookup_ssa(c, operand.ssa);
    Type *type           = local->type;
    if (type == NULL) { return undefined_symbol(SV(""), curloc(c, Idx)); }

    return success(type);
  }

  case OPRFMT_VALUE: {
    Value *value = context_values_at(c, operand.index);
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

static TResult typecheck_move(Context *restrict c, u64 Idx, Instruction I) {
  // The load instruction is currently only used to initialize
  // symbols and ssa locals. it could be renamed to define or something similar.
  // The type of the symbol or ssa local is inferred to be the type
  // of the initializer.
  switch (I.A_format) {
  case OPRFMT_SSA: {
    LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
    try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));
    local->type = Bty;
    return success(Bty);
  }

  case OPRFMT_LABEL: {
    StringView label           = context_global_labels_at(c, I.A.index);
    SymbolTableElement *global = context_global_symbol_table_at(c, label);
    try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));
    global->type = Bty;
    return success(Bty);
  }

  default: EXP_UNREACHABLE;
  }
}

static TResult typecheck_ret(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_operand(c, Idx, I.B_format, I.B);
}

static TResult typecheck_call(Context *restrict c, u64 Idx, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));

  if (Bty->kind != TYPEKIND_FUNCTION) {
    return type_not_callable(Bty, curloc(c, Idx));
  }

  FunctionType *function_type     = &Bty->function_type;
  TupleType *formal_types         = &function_type->argument_types;
  ActualArgumentList *actual_args = context_call_at(c, I.C.index);

  if (formal_types->size != actual_args->size) {
    return args_size_mismatch(
        formal_types->size, actual_args->size, curloc(c, Idx));
  }

  for (u8 i = 0; i < actual_args->size; ++i) {
    Type *formal_type = formal_types->types[i];
    Operand operand   = actual_args->list[i];
    try(actual_type, typecheck_operand(c, Idx, operand.format, operand.value));

    if (!type_equality(actual_type, formal_type)) {
      return arg_type_mismatch(formal_type, actual_type, curloc(c, Idx));
    }
  }

  local->type = function_type->return_type;
  return success(function_type->return_type);
}

static bool tuple_index_out_of_bounds(i64 index, TupleType *tuple) {
  return ((index < 0) || ((u64)index >= tuple->size));
}

static TResult typecheck_dot(Context *restrict c, u64 Idx, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));

  if (Bty->kind != TYPEKIND_TUPLE) {
    return type_not_indexable(Bty, curloc(c, Idx));
  }

  TupleType *tuple = &Bty->tuple_type;

  if (I.C_format != OPRFMT_IMMEDIATE) {
    return index_not_comptime(curloc(c, Idx));
  }

  i64 index = I.C.immediate;

  if (tuple_index_out_of_bounds(index, tuple)) {
    return index_out_of_bounds(index, tuple->size, curloc(c, Idx));
  }

  local->type = tuple->types[index];
  return success(tuple->types[index]);
}

static TResult typecheck_lea(Context *restrict c, u64 Idx, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));

  Type *pointer = context_pointer_type(c, Bty);
  local->type   = pointer;
  return success(pointer);
}

static TResult typecheck_neg(Context *restrict c, u64 Idx, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    return expected_type(i64ty, Bty, curloc(c, Idx));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult
typecheck_i64_binop(Context *restrict c, u64 Idx, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, Idx, I.B_format, I.B));

  try(Cty, typecheck_operand(c, Idx, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    return expected_type(i64ty, Bty, curloc(c, Idx));
  }

  if (!type_equality(Bty, Cty)) {
    return expected_type(Bty, Cty, curloc(c, Idx));
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_add(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_i64_binop(c, Idx, I);
}

static TResult typecheck_sub(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_i64_binop(c, Idx, I);
}

static TResult typecheck_mul(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_i64_binop(c, Idx, I);
}

static TResult typecheck_div(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_i64_binop(c, Idx, I);
}

static TResult typecheck_mod(Context *restrict c, u64 Idx, Instruction I) {
  return typecheck_i64_binop(c, Idx, I);
}

static TResult typecheck_function(Context *restrict c) {
  Type *return_type       = NULL;
  SymbolTableElement *ste = context_current_ste(c);
  assert(ste->kind == STE_FUNCTION);
  FunctionBody *body   = &ste->function_body;
  Bytecode *bc         = &body->bc;
  bool explicit_return = false;

  Instruction *ip = bc->buffer;
  u64 idx         = 0;
  for (; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (I.opcode) {
    case OPC_RET: {
      explicit_return = true;
      try(Bty, typecheck_ret(c, idx, I));

      if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
        return expected_type(return_type, Bty, curloc(c, idx));
      }

      return_type = Bty;
      break;
    }

    case OPC_CALL: {
      try(Aty, typecheck_call(c, idx, I));
      break;
    }

    case OPC_DOT: {
      try(Aty, typecheck_dot(c, idx, I));
      break;
    }

    case OPC_LEA: {
      try(Aty, typecheck_lea(c, idx, I));
      break;
    }

    case OPC_MOVE: {
      try(Bty, typecheck_move(c, idx, I));
      break;
    }

    case OPC_NEG: {
      try(Bty, typecheck_neg(c, idx, I));
      break;
    }

    case OPC_ADD: {
      try(Aty, typecheck_add(c, idx, I));
      break;
    }

    case OPC_SUB: {
      try(Aty, typecheck_sub(c, idx, I));
      break;
    }

    case OPC_MUL: {
      try(Aty, typecheck_mul(c, idx, I));
      break;
    }

    case OPC_DIV: {
      try(Aty, typecheck_div(c, idx, I));
      break;
    }

    case OPC_MOD: {
      try(Aty, typecheck_mod(c, idx, I));
      break;
    }

    default: EXP_UNREACHABLE;
    }
  }

  // #TODO: it should be possible to insert a return statement
  //  here. we just have to maintain the invariant that there is
  //  an associated SourceLocation in the context for this new
  //  return statement.
  if (!explicit_return) { return expected_return(curloc(c, idx)); }

  return success(return_type);
}

static TResult typecheck_constant(Context *restrict c) {
  SymbolTableElement *ste = context_current_ste(c);
  FunctionBody *body      = &ste->function_body;
  Bytecode *bc            = &body->bc;
  Type *result            = nullptr;

  for (u64 i = 0; i < bc->length; ++i) {
    Instruction I = bc->buffer[i];
    switch (I.opcode) {
    // OPC_RET is generated by the return statement
    // which cannot be present in the initialization
    // expression of a global constant. (per the grammar)
    case OPC_RET: {
      EXP_UNREACHABLE;
    }

    case OPC_MOVE: {
      try(Aty, typecheck_move(c, i, I));
      result = Aty;
      break;
    }

    case OPC_CALL: {
      try(Aty, typecheck_call(c, i, I));
      break;
    }

    case OPC_ADD: {
      try(Aty, typecheck_add(c, i, I));
      break;
    }

    case OPC_SUB: {
      try(Aty, typecheck_sub(c, i, I));
      break;
    }

    case OPC_MUL: {
      try(Aty, typecheck_mul(c, i, I));
      break;
    }

    case OPC_DIV: {
      try(Aty, typecheck_div(c, i, I));
      break;
    }

    case OPC_MOD: {
      try(Aty, typecheck_mod(c, i, I));
      break;
    }

    case OPC_DOT: {
      try(Aty, typecheck_dot(c, i, I));
      break;
    }

    default: EXP_UNREACHABLE;
    }
  }
  return success(result);
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
    SymbolTableElement *ste = context_enter_global(c, element->name);
    assert(ste->kind == STE_FUNCTION);
    FunctionBody *body = &ste->function_body;

    try(Rty, typecheck_function(c));
    context_leave_global(c);

    if ((body->return_type != NULL) &&
        (!type_equality(Rty, body->return_type))) {
      return expected_type(body->return_type, Rty, curloc(c, 0));
    }

    body->return_type   = Rty;
    Type *function_type = type_of_function(body, c);
    element->type       = function_type;
    return success(function_type);
  }

  case STE_CONSTANT: {
    SymbolTableElement *ste = context_enter_global(c, element->name);
    assert(ste->kind == STE_CONSTANT);
    try(Ty, typecheck_constant(c));
    context_leave_global(c);
    element->type = Ty;
    return success(Ty);
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
      write_error(&tr.error, stderr);
      tresult_destroy(&tr);
      result |= EXIT_FAILURE;
    }

    symbol_table_iterator_next(&iter);
  }

  return result;
}
