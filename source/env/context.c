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
#include "env/context.h"

Context context_create() {
  Context context;
  context.source_path = path_create();
  context.string_interner = string_interner_create();
  context.type_interner = type_interner_create();
  context.global_symbols = symbol_table_create();
  return context;
}

void context_destroy(Context *restrict context) {
  path_destroy(&(context->source_path));
  string_interner_destroy(&(context->string_interner));
  type_interner_destroy(&(context->type_interner));
  symbol_table_destroy(&(context->global_symbols));
}

void context_set_source_path(Context *restrict context,
                             char const *restrict data, size_t length) {
  path_assign(&(context->source_path), data, length);
}

StringView context_source_path(Context *restrict context) {
  return path_to_view(&(context->source_path));
}

StringView context_intern(Context *restrict context, char const *data,
                          size_t length) {
  return string_interner_insert(&(context->string_interner), data, length);
}

Type *context_integer_type(Context *restrict context) {
  return type_interner_integer_type(&(context->type_interner));
}

bool context_insert_global(Context *restrict context, StringView name,
                           Type *type, Value value) {
  return symbol_table_insert(&(context->global_symbols), name, type, value);
}

SymbolTableElement *context_lookup_global(Context *restrict context,
                                          StringView name) {
  return symbol_table_lookup(&(context->global_symbols), name);
}