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
#include <math.h>

#include "backend/x64/symbols.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/hash.h"
#include "utility/panic.h"

#define SYMBOL_TABLE_MAX_LOAD_FACTOR 0.75

void x64_symbol_destroy(x64_Symbol *restrict symbol) {
  if (symbol->kind == X64SYM_UNDEFINED) { return; }

  x64_function_body_destroy(&symbol->body);
}

x64_SymbolTable x64_symbol_table_create() {
  x64_SymbolTable symbols = {
      .size     = 0,
      .capacity = 0,
      .buffer   = nullptr,
  };
  return symbols;
}

void x64_symbol_table_destroy(x64_SymbolTable *restrict symbols) {
  assert(symbols != NULL);
  for (u64 i = 0; i < symbols->size; ++i) {
    x64_symbol_destroy(symbols->buffer + i);
  }
  symbols->size     = 0;
  symbols->capacity = 0;
  deallocate(symbols->buffer);
  symbols->buffer = NULL;
}

static x64_Symbol *x64_symbol_table_find(x64_Symbol *restrict elements,
                                         u64 capacity,
                                         StringView name) {
  u64 index = hash_cstring(name.ptr, name.length) % capacity;
  while (1) {
    x64_Symbol *element = elements + index;
    if ((element->name.ptr == nullptr) ||
        (string_view_eq(name, element->name))) {
      return element;
    }

    index = (index + 1) % capacity;
  }
}

static void x64_symbol_table_grow(x64_SymbolTable *restrict table) {
  Growth g           = array_growth_u64(table->capacity, sizeof(x64_Symbol));
  x64_Symbol *buffer = callocate(g.new_capacity, sizeof(x64_Symbol));

  if (table->buffer != nullptr) {
    for (u64 i = 0; i < table->capacity; ++i) {
      x64_Symbol *element = table->buffer + i;
      if (string_view_empty(element->name)) { continue; }

      x64_Symbol *dst =
          x64_symbol_table_find(buffer, g.new_capacity, element->name);
      *dst = *element;
    }

    deallocate(table->buffer);
  }

  table->capacity = g.new_capacity;
  table->buffer   = buffer;
}

static bool x64_symbol_table_full(x64_SymbolTable *restrict table) {
  u64 load_limit =
      (u64)floor((double)table->capacity * SYMBOL_TABLE_MAX_LOAD_FACTOR);
  return (table->size + 1) >= load_limit;
}

x64_Symbol *x64_symbol_table_at(x64_SymbolTable *restrict symbols,
                                StringView name) {
  assert(symbols != NULL);
  if (x64_symbol_table_full(symbols)) { x64_symbol_table_grow(symbols); }

  x64_Symbol *symbol =
      x64_symbol_table_find(symbols->buffer, symbols->capacity, name);
  if (symbol->name.ptr == nullptr) {
    symbol->name = name;
    symbols->size += 1;
  }

  return symbol;
}

x64_SymbolIterator x64_symbol_iterator(x64_SymbolTable *restrict table) {
  x64_SymbolIterator iter = {.symbol = table->buffer,
                             .end    = table->buffer + table->capacity};
  while ((iter.symbol != iter.end) && (iter.symbol->kind == X64SYM_UNDEFINED)) {
    iter.symbol = iter.symbol + 1;
  }

  return iter;
}

void x64_symbol_iterator_next(x64_SymbolIterator *restrict iter) {
  if (iter->symbol == iter->end) { return; }

  iter->symbol += 1;

  while ((iter->symbol != iter->end) &&
         (iter->symbol->kind == X64SYM_UNDEFINED)) {
    iter->symbol += 1;
  }
}

bool x64_symbol_iterator_done(x64_SymbolIterator *restrict iter) {
  return iter->symbol == iter->end;
}
