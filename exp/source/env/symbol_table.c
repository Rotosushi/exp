// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "env/symbol_table.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/hash.h"

#define SYMBOL_TABLE_MAX_LOAD 0.75

SymbolTable symbol_table_create() {
  SymbolTable symbol_table;
  symbol_table.capacity = symbol_table.count = 0;
  symbol_table.elements                      = NULL;
  return symbol_table;
}

void symbol_table_destroy(SymbolTable *restrict symbol_table) {
  assert(symbol_table != NULL);

  for (u64 i = 0; i < symbol_table->capacity; ++i) {
    SymbolTableElement *element = symbol_table->elements + i;
    function_body_destroy(&element->function_body);
  }

  symbol_table->count    = 0;
  symbol_table->capacity = 0;
  deallocate(symbol_table->elements);
  symbol_table->elements = NULL;
}

static SymbolTableElement *symbol_table_find(
    SymbolTableElement *restrict elements, u64 capacity, StringView name) {
  u64 index = hash_cstring(name.ptr, name.length) % capacity;
  while (1) {
    SymbolTableElement *element = &(elements[index]);
    if ((element->name.ptr == NULL) || string_view_eq(name, element->name)) {
      return element;
    }

    index = (index + 1) % capacity;
  }
}

static void symbol_table_grow(SymbolTable *restrict symbol_table) {
  Growth g =
      array_growth_u64(symbol_table->capacity, sizeof(SymbolTableElement));
  SymbolTableElement *elements =
      callocate(g.new_capacity, sizeof(SymbolTableElement));

  if (symbol_table->elements != NULL) {
    for (u64 i = 0; i < symbol_table->capacity; ++i) {
      SymbolTableElement *element = &(symbol_table->elements[i]);
      if (string_view_empty(element->name)) { continue; }

      SymbolTableElement *dest =
          symbol_table_find(elements, g.new_capacity, element->name);
      dest->name          = element->name;
      dest->function_body = element->function_body;
    }

    // we can avoid freeing each element because we
    // move the data to the new allocation.
    deallocate(symbol_table->elements);
  }

  symbol_table->capacity = g.new_capacity;
  symbol_table->elements = elements;
}

static bool symbol_table_full(SymbolTable *restrict symbol_table) {
  u64 load_limit =
      (u64)floor((double)symbol_table->capacity * SYMBOL_TABLE_MAX_LOAD);
  return (symbol_table->count + 1) >= load_limit;
}

SymbolTableElement *symbol_table_at(SymbolTable *restrict symbol_table,
                                    StringView name) {
  assert(symbol_table != NULL);

  if (symbol_table_full(symbol_table)) { symbol_table_grow(symbol_table); }

  SymbolTableElement *element =
      symbol_table_find(symbol_table->elements, symbol_table->capacity, name);
  if (element->name.ptr == NULL) {
    element->name = name;
    symbol_table->count += 1;
  }

  return element;
}

SymbolTableIterator
symbol_table_iterator_create(SymbolTable *restrict symbol_table) {
  assert(symbol_table != NULL);

  SymbolTableIterator iter = {symbol_table->elements,
                              symbol_table->elements + symbol_table->capacity};
  if ((iter.element != NULL) && (iter.element->name.ptr == NULL)) {
    symbol_table_iterator_next(&iter);
  }
  return iter;
}

void symbol_table_iterator_next(SymbolTableIterator *restrict iter) {
  assert(iter != NULL);

  if ((iter->element != iter->end) && (iter->element != NULL)) {
    do {
      iter->element += 1;
    } while ((iter->element != iter->end) && (iter->element->name.ptr == NULL));
  }
}

bool symbol_table_iterator_done(SymbolTableIterator *restrict iter) {
  assert(iter != NULL);

  return iter->element == iter->end;
}
