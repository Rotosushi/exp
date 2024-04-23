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
  Context context;
  context.options         = *options;
  context.string_interner = string_interner_create();
  context.type_interner   = type_interner_create();
  context.global_symbols  = symbol_table_create();
  context.constants       = constants_create();
  context.stack           = stack_create();
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  context_options_destroy(&(context->options));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbols));
  constants_destroy(&(context->constants));
  stack_destroy(&(context->stack));
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

u64 context_constants_append(Context *restrict context, Value value) {
  assert(context != NULL);
  return constants_append(&(context->constants), value);
}

Value *context_constants_at(Context *restrict context, u64 index) {
  assert(context != NULL);
  return constants_at(&(context->constants), index);
}

bool context_stack_empty(Context *restrict context) {
  assert(context != NULL);
  return stack_empty(&(context->stack));
}

void context_stack_push(Context *restrict context, Value value) {
  assert(context != NULL);
  stack_push(&context->stack, value);
}

Value context_stack_pop(Context *restrict context) {
  assert(context != NULL);
  return stack_pop(&context->stack);
}

Value *context_stack_peek(Context *restrict context) {
  assert(context != NULL);
  return stack_peek(&context->stack);
}
