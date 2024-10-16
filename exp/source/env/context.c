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
#include "utility/unreachable.h"

Context context_create(CLIOptions *restrict options) {
  assert(options != NULL);
  Context context = {
      .options             = context_options_create(options),
      .source_locations    = source_locations_create(),
      .string_interner     = string_interner_create(),
      .type_interner       = type_interner_create(),
      .global_symbol_table = symbol_table_create(),
      .global_labels       = global_labels_create(),
      .values              = values_create(),
      .current_ste         = nullptr,
  };
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  source_locations_destroy(&context->source_locations);
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

SourceLocation context_lookup_source_location(Context *restrict context,
                                              u64 Idx) {
  return source_locations_lookup(&context->source_locations, Idx);
}

static void context_insert_source_location(Context *restrict context,
                                           SourceLocation location,
                                           u64 Idx) {
  source_locations_insert(&context->source_locations, location, Idx);
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

Type *context_pointer_type(Context *restrict context, Type *restrict pointee) {
  assert(context != NULL);
  return type_interner_pointer_type(&context->type_interner, pointee);
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

SymbolTableElement *context_enter_global(Context *restrict c, StringView name) {
  SymbolTableElement *element = context_global_symbol_table_at(c, name);
  c->current_ste              = element;
  return element;
}

SymbolTableElement *context_current_ste(Context *restrict c) {
  assert(c != nullptr);
  assert(c->current_ste != nullptr);
  return c->current_ste;
}

SymbolTableElement *context_global_init(Context *restrict c) {
  assert(c != nullptr);
  SymbolTableElement *init = context_global_symbol_table_at(c, SV("_init"));
  if (init->kind != STE_UNDEFINED) { return init; }

  init->kind = STE_FUNCTION;
  return init;
}

bool context_at_global_scope(Context *restrict c) {
  if (c->current_ste == nullptr) return true;
  switch (c->current_ste->kind) {
  case STE_UNDEFINED: EXP_UNREACHABLE;
  case STE_FUNCTION:  return false;
  case STE_CONSTANT:  return true;
  default:            EXP_UNREACHABLE;
  }
}

void context_leave_global(Context *restrict c) {
  assert(c != nullptr);
  c->current_ste = nullptr;
}

// we can check if the current symbol is a constant
// and replace the individual function body for each constant
// with the single _init function. and I can get that to
// work with the backend. I am struggling to figure out how
// to get the backend to emit the initializer expressions
// into the single _init body from multiple functions.
// this is because we precompute lifetime information
// for a given function and use that information to drive
// the register allocator.
// We can precompute the lifetime information for a given
// initializer function, and generate a stub function.
// and then the emitter can collect all the stubs into the
// single function body.
FunctionBody *context_current_function(Context *restrict c) {
  SymbolTableElement *ste = context_current_ste(c);
  return &ste->function_body;
}

Bytecode *context_active_bytecode(Context *restrict c) {
  return &context_current_function(c)->bc;
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

void context_def_constant_global(Context *restrict c,
                                 SourceLocation location,
                                 StringView name,
                                 Operand value) {
  u64 idx   = context_global_labels_insert(c, name);
  Operand A = operand_label(idx);
  context_emit_move(c, location, A, value);
}

void context_def_constant_local(Context *restrict c,
                                SourceLocation location,
                                StringView name,
                                Operand operand) {
  if (operand.format != OPRFMT_SSA) {
    Operand A = context_new_ssa(c);
    context_emit_move(c, location, A, operand);
    function_body_new_local(context_current_function(c), name, A.value.ssa);
    return;
  }

  function_body_new_local(context_current_function(c), name, operand.value.ssa);
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

Operand context_values_append(Context *restrict context, Value value) {
  assert(context != NULL);
  return values_add(&(context->values), value);
}

Value *context_values_at(Context *restrict context, u64 index) {
  assert(context != NULL);
  return values_at(&(context->values), index);
}

void context_emit_return(Context *restrict c,
                         SourceLocation location,
                         Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_ret(B));
}

Operand context_emit_call(Context *restrict c,
                          SourceLocation location,
                          Operand B,
                          Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_call(A, B, C));
  return A;
}

Operand context_emit_dot(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_dot(A, B, C));
  return A;
}

Operand context_emit_move(Context *restrict c,
                          SourceLocation location,
                          Operand A,
                          Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_move(A, B));
  return A;
}

Operand
context_emit_neg(Context *restrict c, SourceLocation location, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_neg(A, B));
  return A;
}

Operand context_emit_add(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_add(A, B, C));
  return A;
}

Operand context_emit_sub(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_sub(A, B, C));
  return A;
}

Operand context_emit_mul(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_mul(A, B, C));
  return A;
}

Operand context_emit_div(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_div(A, B, C));
  return A;
}

Operand context_emit_mod(Context *restrict c,
                         SourceLocation location,
                         Operand B,
                         Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_ssa(c);
  context_insert_source_location(c, location, bytecode_current_index(bc));
  bytecode_append(bc, instruction_mod(A, B, C));
  return A;
}
