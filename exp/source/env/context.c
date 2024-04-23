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

Context context_create(ContextOptions *restrict options) {
  assert(options != NULL);
  Context context = {.options         = *options,
                     .string_interner = string_interner_create(),
                     .type_interner   = type_interner_create(),
                     .global_symbols  = symbol_table_create(),
                     .call_stack      = call_stack_create(),
                     .locals          = locals_create(),
                     .constants       = constants_create()};
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbols));
  call_stack_destroy(&context->call_stack);
  locals_destroy(&(context->locals));
  constants_destroy(&(context->constants));
}

StringView context_source_path(Context *restrict context) {
  assert(context != NULL);
  return path_to_view(&(context->options.source));
}

FILE *context_open_source(Context *restrict context) {
  StringView path = context_source_path(context);
  return file_open(path.ptr, "r");
}

String context_buffer_source(Context *restrict context) {
  FILE *file = context_open_source(context);

  String result = file_readall(file);

  file_close(file);

  return result;
}

StringView context_output_path(Context *restrict context) {
  assert(context != NULL);
  return path_to_view(&(context->options.output));
}

FILE *context_open_output(Context *restrict context) {
  StringView path = context_output_path(context);
  return file_open(path.ptr, "w");
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

CallFrame context_push_function(Context *restrict c, StringView name) {
  assert(c != NULL);
  // get the new function
  SymbolTableElement *element = symbol_table_at(&c->global_symbols, name);
  // push the function onto the call stack
  CallFrame cf = call_stack_push(&c->call_stack, &element->function_body,
                                 locals_push_frame(&c->locals));
  // insert the return slot at frame[0]
  locals_new_local(&c->locals, cf.frame);
  return cf;
}

void context_pop_function(Context *restrict c) {
  assert(c != NULL);
  CallFrame cf = call_stack_pop(&c->call_stack);
  locals_pop_frame(&c->locals, cf.frame);
}

CallFrame context_active_frame(Context *restrict c) {
  return call_stack_top(&c->call_stack);
}

static Bytecode *context_active_bytecode(Context *restrict c) {
  CallFrame cf = context_active_frame(c);
  return &cf.function->bc;
}

Operand context_new_local(Context *restrict c) {
  assert(c != NULL);
  CallFrame cf   = context_active_frame(c);
  Operand result = {.format = FORMAT_LOCAL,
                    .common = locals_new_local(&c->locals, cf.frame)};
  return result;
}

Value *context_local_at(Context *restrict c, Operand operand) {
  assert(c != NULL);
  assert(operand.format == FORMAT_LOCAL);
  CallFrame cf = context_active_frame(c);
  return locals_at(&c->locals, cf.frame, operand.common);
}

Operand context_constants_add(Context *restrict context, Value value) {
  assert(context != NULL);
  return constants_add(&(context->constants), value);
}

Value *context_constants_at(Context *restrict context, u16 index) {
  assert(context != NULL);
  return constants_at(&(context->constants), index);
}

Operand context_emit_move(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_move(bc, A, B);
  return A;
}

Operand context_emit_neg(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_neg(bc, A, B);
  return A;
}

Operand context_emit_add(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_add(bc, A, B, C);
  return A;
}

Operand context_emit_sub(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_sub(bc, A, B, C);
  return A;
}

Operand context_emit_mul(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_mul(bc, A, B, C);
  return A;
}

Operand context_emit_div(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_div(bc, A, B, C);
  return A;
}

Operand context_emit_mod(Context *restrict c, Operand B, Operand C) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  Operand A    = context_new_local(c);
  bytecode_emit_mod(bc, A, B, C);
  return A;
}

void context_emit_return(Context *restrict c, Operand B) {
  assert(c != NULL);
  Bytecode *bc = context_active_bytecode(c);
  bytecode_emit_return(bc, B);
}
