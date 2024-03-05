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

#include "env/context.h"

Context context_create(CLIOptions *restrict cli_options) {
  Context context;
  if (cli_options == NULL) {
    context.options = options_create();
  } else {
    context.options = options_from_cli_options(cli_options);
  }
  context.string_interner = string_interner_create();
  context.type_interner = type_interner_create();
  context.global_symbols = symbol_table_create();
  context.constants = constants_create();
  context.stack = stack_create();
  return context;
}

void context_destroy(Context *restrict context) {
  assert(context != NULL);
  options_destroy(&(context->options));
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

StringView context_intern(Context *restrict context, char const *data,
                          size_t length) {
  assert(context != NULL);
  return string_interner_insert(&(context->string_interner), data, length);
}

Type *context_integer_type(Context *restrict context) {
  assert(context != NULL);
  return type_interner_integer_type(&(context->type_interner));
}

bool context_insert_global(Context *restrict context, StringView name,
                           Type *type, Value value) {
  assert(context != NULL);
  return symbol_table_insert(&(context->global_symbols), name, type, value);
}

SymbolTableElement *context_lookup_global(Context *restrict context,
                                          StringView name) {
  assert(context != NULL);
  return symbol_table_lookup(&(context->global_symbols), name);
}