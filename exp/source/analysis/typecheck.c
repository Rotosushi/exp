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

static TResult typecheck_operand(Context *restrict c,
                                 OperandFormat format,
                                 OperandValue operand) {
  switch (format) {
  case OPRFMT_SSA: {
    LocalVariable *local = context_lookup_ssa(c, operand.ssa);
    Type *type           = local->type;
    if (type == NULL) {
      return error(ERROR_TYPECHECK_UNDEFINED_SYMBOL, string_from_view(SV("")));
    }

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

static TResult typecheck_move(Context *restrict c, Instruction I) {
  // The load instruction is currently only used to initialize
  // symbols and ssa locals. it could be renamed to define or something similar.
  // The type of the symbol or ssa local is inferred to be the type
  // of the initializer.
  switch (I.A_format) {
  case OPRFMT_SSA: {
    LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
    try(Bty, typecheck_operand(c, I.B_format, I.B));
    local->type = Bty;
    return success(Bty);
  }

  case OPRFMT_LABEL: {
    StringView label           = context_global_labels_at(c, I.A.index);
    SymbolTableElement *global = context_global_symbol_table_at(c, label);
    try(Bty, typecheck_operand(c, I.B_format, I.B));
    global->type = Bty;
    return success(Bty);
  }

  default: EXP_UNREACHABLE;
  }
}

static TResult typecheck_ret(Context *restrict c, Instruction I) {
  return typecheck_operand(c, I.B_format, I.B);
}

static TResult typecheck_call(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  if (Bty->kind != TYPEKIND_FUNCTION) {
    String buf = string_create();
    string_append(&buf, SV("Type is not callable ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  FunctionType *function_type     = &Bty->function_type;
  TupleType *formal_types         = &function_type->argument_types;
  ActualArgumentList *actual_args = context_call_at(c, I.C.index);

  if (formal_types->size != actual_args->size) {
    String buf = string_create();
    string_append(&buf, SV("Expected "));
    string_append_u64(&buf, formal_types->size);
    string_append(&buf, SV(" arguments, have "));
    string_append_u64(&buf, actual_args->size);
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  for (u8 i = 0; i < actual_args->size; ++i) {
    Type *formal_type = formal_types->types[i];
    Operand operand   = actual_args->list[i];
    try(actual_type, typecheck_operand(c, operand.format, operand.value));

    if (!type_equality(actual_type, formal_type)) {
      String buf = string_create();
      string_append(&buf, SV("Expected ["));
      print_type(formal_type, &buf);
      string_append(&buf, SV("] Actual ["));
      print_type(actual_type, &buf);
      string_append(&buf, SV("]"));
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
    }
  }

  local->type = function_type->return_type;
  return success(function_type->return_type);
}

static bool tuple_index_out_of_bounds(i64 index, TupleType *tuple) {
  return ((index < 0) || ((u64)index >= tuple->size));
}

static TResult typecheck_dot(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  if (Bty->kind != TYPEKIND_TUPLE) {
    String buf = string_create();
    string_append(&buf, SV("Type is not a tuple ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  TupleType *tuple = &Bty->tuple_type;

  if (I.C_format != OPRFMT_IMMEDIATE) {
    return error(ERROR_TYPECHECK_TUPLE_INDEX_NOT_IMMEDIATE,
                 string_from_view(SV("")));
  }

  i64 index = I.C.immediate;

  if (tuple_index_out_of_bounds(index, tuple)) {
    String buf = string_create();
    string_append(&buf, SV("The given index "));
    string_append_i64(&buf, index);
    string_append(&buf, SV(" is not in the valid range of 0.."));
    string_append_u64(&buf, tuple->size);
    return error(ERROR_TYPECHECK_TUPLE_INDEX_OUT_OF_BOUNDS, buf);
  }

  local->type = tuple->types[index];
  return success(tuple->types[index]);
}

static TResult typecheck_lea(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  Type *pointer = context_pointer_type(c, Bty);
  local->type   = pointer;
  return success(pointer);
}

static TResult typecheck_neg(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_add(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  try(Cty, typecheck_operand(c, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  if (!type_equality(Bty, Cty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Cty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_sub(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  try(Cty, typecheck_operand(c, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  if (!type_equality(Bty, Cty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Cty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_mul(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  try(Cty, typecheck_operand(c, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  if (!type_equality(Bty, Cty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Cty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_div(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  try(Cty, typecheck_operand(c, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  if (!type_equality(Bty, Cty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Cty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_mod(Context *restrict c, Instruction I) {
  LocalVariable *local = context_lookup_ssa(c, I.A.ssa);
  try(Bty, typecheck_operand(c, I.B_format, I.B));

  try(Cty, typecheck_operand(c, I.C_format, I.C));

  Type *i64ty = context_i64_type(c);
  if (!type_equality(i64ty, Bty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Bty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  if (!type_equality(Bty, Cty)) {
    String buf = string_create();
    string_append(&buf, SV("Expected [i64] Actual ["));
    print_type(Cty, &buf);
    string_append(&buf, SV("]"));
    return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
  }

  local->type = Bty;
  return success(Bty);
}

static TResult typecheck_function(Context *restrict c) {
  Type *return_type       = NULL;
  SymbolTableElement *ste = context_current_ste(c);
  assert(ste->kind == STE_FUNCTION);
  FunctionBody *body   = &ste->function_body;
  Bytecode *bc         = &body->bc;
  bool explicit_return = false;

  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (I.opcode) {
    case OPC_RET: {
      explicit_return = true;
      try(Bty, typecheck_ret(c, I));

      if ((return_type != NULL) && (!type_equality(return_type, Bty))) {
        String buf = string_create();
        string_append(&buf, SV("Previous return statement had type ["));
        print_type(return_type, &buf);
        string_append(&buf, SV("] this return statement has type ["));
        print_type(Bty, &buf);
        string_append(&buf, SV("]"));
        return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
      }

      return_type = Bty;
      break;
    }

    case OPC_CALL: {
      try(Aty, typecheck_call(c, I));
      break;
    }

    case OPC_DOT: {
      try(Aty, typecheck_dot(c, I));
      break;
    }

    case OPC_LEA: {
      try(Aty, typecheck_lea(c, I));
      break;
    }

    case OPC_MOVE: {
      try(Bty, typecheck_move(c, I));
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

    default: EXP_UNREACHABLE;
    }
  }

  if (!explicit_return) {
    Operand index = context_values_append(c, value_create_nil());
    bytecode_append(bc, instruction_ret(index));
    return_type = context_nil_type(c);
  }

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
      try(Aty, typecheck_move(c, I));
      result = Aty;
      break;
    }

    case OPC_CALL: {
      try(Aty, typecheck_call(c, I));
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

    case OPC_DOT: {
      try(Aty, typecheck_dot(c, I));
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
      String buf = string_create();
      string_append(&buf, SV("Function was annotated with type ["));
      print_type(body->return_type, &buf);
      string_append(&buf, SV("] actual returned type ["));
      print_type(Rty, &buf);
      string_append(&buf, SV("]"));
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, buf);
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
