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

Context context_create(CLIOptions *restrict options) {
  assert(options != NULL);
  Context context = {.options             = context_options_create(options),
                     .string_interner     = string_interner_create(),
                     .type_interner       = type_interner_create(),
                     .global_symbol_table = symbol_table_create(),
                     .global_labels       = global_labels_create(),
                     .values              = values_create()};
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbol_table));
  global_labels_destroy(&(context->global_labels));
  values_destroy(&(context->values));
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

Type *context_tuple_type(Context *restrict context, TupleType tuple) {
  assert(context != NULL);
  return type_interner_tuple_type(&context->type_interner, tuple);
}

Type *context_function_type(Context *restrict context,
                            Type *return_type,
                            TupleType argument_types) {
  assert(context != NULL);
  return type_interner_function_type(
      &context->type_interner, return_type, argument_types);
}

u64 context_global_labels_insert(Context *restrict context, StringView symbol) {
  assert(context != NULL);
  return global_labels_insert(&context->global_labels, symbol);
}

StringView context_global_labels_at(Context *restrict context, u64 idx) {
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

ActualArgumentList *context_call_at(Context *restrict c, u64 idx) {
  return function_body_call_at(context_current_function(c), idx);
}

void context_def_local_const(Context *restrict c,
                             StringView name,
                             Operand value) {
  Operand A = context_emit_load(c, value);
  function_body_new_local(context_current_function(c), name, A.ssa);
}

LocalVariable *context_lookup_local(Context *restrict c, StringView name) {
  return local_variables_lookup(&(context_current_function(c)->locals), name);
}

LocalVariable *context_lookup_ssa(Context *restrict c, u64 ssa) {
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
  return values_add(&(context->values), value);
}

Value *context_constants_at(Context *restrict context, u64 index) {
  assert(context != NULL);
  return values_at(&(context->values), index);
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
  bytecode_append(bc, imr_ret(B));
}

Operand context_emit_call(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  bytecode_append(bc, imr_call(A, B, C));
  return A;
}

Operand context_emit_load(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  bytecode_append(bc, imr_load(A, B));
  return A;
}

FoldResult context_emit_neg(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A;
  switch (B.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_neg(A, B));
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    A = operand_immediate(-B.immediate);
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
    bytecode_append(bc, imr_add(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = B.immediate;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_add(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = C.immediate;
    break;
  }

  default: unreachable();
  }

  if (__builtin_add_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  A = operand_immediate(z);

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
    bytecode_append(bc, imr_sub(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = B.immediate;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_sub(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = C.immediate;
    break;
  }

  default: unreachable();
  }

  if (__builtin_sub_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  A = operand_immediate(z);

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
    bytecode_append(bc, imr_mul(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = B.immediate;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_mul(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = C.immediate;
    break;
  }

  default: unreachable();
  }

  if (__builtin_mul_overflow(x, y, &z)) {
    return error(ERROR_INTEGER_TO_LARGE, string_view_from_cstring(""));
  }

  A = operand_immediate(z);

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
    bytecode_append(bc, imr_div(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = B.immediate;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_div(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = C.immediate;
    break;
  }

  default: unreachable();
  }

  z = x / y;

  A = operand_immediate(z);

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
    bytecode_append(bc, imr_mod(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    x = B.immediate;
    break;
  }

  default: unreachable();
  }

  switch (C.format) {
  case OPRFMT_SSA: {
    A = context_new_ssa(c);
    bytecode_append(bc, imr_mod(A, B, C));
    return success(A);
  }

  case OPRFMT_CONSTANT: {
    assert(0);
    break;
  }

  case OPRFMT_IMMEDIATE: {
    y = C.immediate;
    break;
  }

  default: unreachable();
  }

  z = x % y;

  A = operand_immediate(z);

  return success(A);
}
