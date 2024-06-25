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
  Context context = {.options             = context_options_create(options),
                     .string_interner     = string_interner_create(),
                     .type_interner       = type_interner_create(),
                     .global_symbol_table = symbol_table_create(),
                     .global_labels       = global_labels_create(),
                     .constants           = constants_create()};
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbol_table));
  global_labels_destroy(&(context->global_labels));
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
  return string_interner_insert(&(context->string_interner), sv);
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

Type *context_function_type(Context *restrict context,
                            Type *return_type,
                            ArgumentTypes argument_types) {
  assert(context != NULL);
  return type_interner_function_type(
      &context->type_interner, return_type, argument_types);
}

u16 context_global_labels_insert(Context *restrict context, StringView symbol) {
  assert(context != NULL);
  return global_labels_insert(&context->global_labels, symbol);
}

StringView context_global_labels_at(Context *restrict context, u16 idx) {
  assert(context != NULL);
  return global_labels_at(&context->global_labels, idx);
}

SymbolTableElement *context_global_symbol_table_at(Context *restrict context,
                                                   StringView name) {
  assert(context != NULL);
  return symbol_table_at(&context->global_symbol_table, name);
}

SymbolTableIterator
context_global_symbol_table_iterator(Context *restrict context) {
  assert(context != NULL);
  return symbol_table_iterator_create(&context->global_symbol_table);
}

FunctionBody *context_enter_function(Context *restrict c, StringView name) {
  assert(c != NULL);
  SymbolTableElement *element = symbol_table_at(&c->global_symbol_table, name);
  if (element->kind == STE_UNDEFINED) { element->kind = STE_FUNCTION; }

  c->current_function = &element->function_body;
  return c->current_function;
}

FunctionBody *context_current_function(Context *restrict c) {
  assert(c != NULL);
  assert(c->current_function != NULL);
  return c->current_function;
}

Bytecode *context_active_bytecode(Context *restrict c) {
  return &(context_current_function(c)->bc);
}

static Operand context_new_ssa(Context *restrict c) {
  return function_body_new_ssa(context_current_function(c));
}

CallPair context_new_call(Context *restrict c) {
  return function_body_new_call(context_current_function(c));
}

ActualArgumentList *context_call_at(Context *restrict c, u16 idx) {
  return function_body_call_at(context_current_function(c), idx);
}

void context_def_local_const(Context *restrict c,
                             StringView name,
                             Operand value) {
  Operand A = context_emit_load(c, value);
  function_body_new_local(context_current_function(c), name, A.common);
}

LocalVariable *context_lookup_local(Context *restrict c, StringView name) {
  return local_variables_lookup(&(context_current_function(c)->locals), name);
}

LocalVariable *context_lookup_ssa(Context *restrict c, u16 ssa) {
  return local_variables_lookup_ssa(&(context_current_function(c)->locals),
                                    ssa);
}

FormalArgument *context_lookup_argument(Context *restrict c, StringView name) {
  return formal_argument_list_lookup(&(context_current_function(c)->arguments),
                                     name);
}

FormalArgument *context_argument_at(Context *restrict c, u8 index) {
  return formal_argument_list_at(&(context_current_function(c)->arguments),
                                 index);
}

void context_leave_function(Context *restrict c) {
  assert(c != NULL);
  c->current_function = NULL;
}

Operand context_constants_append(Context *restrict context, Value value) {
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
  FoldResult result = {.has_error = 1, .error = error_construct(code, sv)};
  return result;
}

void fold_result_destroy(FoldResult *restrict fr) {
  if (fr->has_error) { error_destroy(&fr->error); }
}

void context_emit_return(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  bytecode_emit_return(bc, B);
}

Operand context_emit_call(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  bytecode_emit_call(bc, A, B, C);
  return A;
}

Operand context_emit_load(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  bytecode_emit_load(bc, A, B);
  return A;
}

static bool in_range(i64 n) { return (n < u16_MAX) && (n > 0); }

FoldResult context_emit_neg(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_neg(bc, A, B);
    break;
  }

  case OPRFMT_CONSTANT: {
    Value *v = context_constants_at(c, B.common);
    if (v->kind == VALUEKIND_I64) {
      i64 n = -(v->integer);
      A     = context_constants_append(c, value_create_i64(n));
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    i64 n = -((i64)(B.common));
    if (in_range(n)) {
      A = operand_immediate((u16)n);
    } else {
      A = context_constants_append(c, value_create_i64(n));
    }
    break;
  }

  default: unreachable();
  }
  return success(A);
}

FoldResult context_emit_add(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  i64 x = 0;
  i64 y = 0;
  i64 z = 0;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_add(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = (i64)B.common;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_add(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Cv = context_constants_at(c, C.common);
    if (Cv->kind == VALUEKIND_I64) {
      y = Cv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = (i64)C.common;
    break;
  }

  default: unreachable();
  }

  if (__builtin_add_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  if (in_range(z)) {
    A = operand_immediate((u16)z);
  } else {
    A = context_constants_append(c, value_create_i64(z));
  }
  return success(A);
}

FoldResult context_emit_sub(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  i64 x = 0;
  i64 y = 0;
  i64 z = 0;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_sub(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = (i64)B.common;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_sub(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Cv = context_constants_at(c, C.common);
    if (Cv->kind == VALUEKIND_I64) {
      y = Cv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = (i64)C.common;
    break;
  }

  default: unreachable();
  }

  if (__builtin_sub_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  if (in_range(z)) {
    A = operand_immediate((u16)z);
  } else {
    A = context_constants_append(c, value_create_i64(z));
  }
  return success(A);
}

FoldResult context_emit_mul(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  i64 x = 0;
  i64 y = 0;
  i64 z = 0;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_mul(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = (i64)B.common;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_mul(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Cv = context_constants_at(c, C.common);
    if (Cv->kind == VALUEKIND_I64) {
      y = Cv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = (i64)C.common;
    break;
  }

  default: unreachable();
  }

  if (__builtin_mul_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  if (in_range(z)) {
    A = operand_immediate((u16)z);
  } else {
    A = context_constants_append(c, value_create_i64(z));
  }
  return success(A);
}

FoldResult context_emit_div(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  i64 x = 0;
  i64 y = 0;
  i64 z = 0;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_div(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = (i64)B.common;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_div(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Cv = context_constants_at(c, C.common);
    if (Cv->kind == VALUEKIND_I64) {
      y = Cv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = (i64)C.common;
    break;
  }

  default: unreachable();
  }

  z = x / y;

  if (in_range(z)) {
    A = operand_immediate((u16)z);
  } else {
    A = context_constants_append(c, value_create_i64(z));
  }
  return success(A);
}

FoldResult context_emit_mod(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  i64 x = 0;
  i64 y = 0;
  i64 z = 0;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_mod(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Bv = context_constants_at(c, B.common);
    if (Bv->kind == VALUEKIND_I64) {
      x = Bv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = (i64)B.common;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_emit_mod(bc, A, B, C);
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    Value *Cv = context_constants_at(c, C.common);
    if (Cv->kind == VALUEKIND_I64) {
      y = Cv->integer;
    } else {
      return error(ERROR_TYPECHECK_TYPE_MISMATCH, string_view_from_cstring(""));
    }
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = (i64)C.common;
    break;
  }

  default: unreachable();
  }

  z = x % y;

  if (in_range(z)) {
    A = operand_immediate((u16)z);
  } else {
    A = context_constants_append(c, value_create_i64(z));
  }
  return success(A);
}
