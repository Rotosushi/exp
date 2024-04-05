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
#include "utility/nearest_power.h"
#include "utility/panic.h"
#include "utility/string_hash.h"

#define SYMBOL_TABLE_MAX_LOAD 0.75

SymbolTable symbol_table_create() {
  SymbolTable symbol_table;
  symbol_table.capacity = symbol_table.count = 0;
  symbol_table.elements                      = NULL;
  return symbol_table;
}

void symbol_table_destroy(SymbolTable *restrict symbol_table) {
  assert(symbol_table != NULL);

  if (symbol_table->elements == NULL) {
    symbol_table->count = symbol_table->capacity = 0;
    return;
  }

  symbol_table->count = symbol_table->capacity = 0;
  free(symbol_table->elements);
  symbol_table->elements = NULL;
}

static SymbolTableElement *
symbol_table_find(SymbolTableElement *restrict elements, size_t capacity,
                  StringView name) {
  size_t index                  = string_hash(name.ptr, name.length) % capacity;
  SymbolTableElement *tombstone = NULL;
  while (1) {
    SymbolTableElement *element = &(elements[index]);
    // if the name is empty the element can be either
    // an empty element or a tombstone.
    if (element->name.ptr == NULL) {
      // UNINITIALIZED value -> tombstone
      if (element->value.kind == VALUEKIND_UNINITIALIZED) {
        return tombstone != NULL ? tombstone : element;
      } else { // otherwise -> tombstone element
        if (tombstone == NULL) {
          tombstone = element;
        }
      }
    } else if (string_view_equality(name, element->name)) {
      return element;
    } else {
      index = (index + 1) % capacity;
    }
  }
}

static void symbol_table_grow(SymbolTable *restrict symbol_table,
                              size_t capacity) {
  SymbolTableElement *elements = calloc(capacity, sizeof(SymbolTableElement));

  if (symbol_table->elements != NULL) {
    symbol_table->count = 0;
    for (size_t i = 0; i < symbol_table->capacity; ++i) {
      SymbolTableElement *element = &(symbol_table->elements[i]);
      if (element->name.ptr == NULL) {
        continue;
      }

      SymbolTableElement *dest =
          symbol_table_find(elements, capacity, element->name);
      dest->name  = element->name;
      dest->type  = element->type;
      dest->value = element->value;
      symbol_table->count += 1;
    }

    free(symbol_table->elements);
  }

  symbol_table->capacity = capacity;
  symbol_table->elements = elements;
}

static bool symbol_table_full(SymbolTable *restrict symbol_table) {
  size_t new_count;
  if (__builtin_add_overflow(symbol_table->count, 1, &new_count)) {
    PANIC("cannot allocate more than SIZE_MAX");
  }

  size_t load_limit =
      (size_t)floor((double)symbol_table->capacity * SYMBOL_TABLE_MAX_LOAD);
  return new_count >= load_limit;
}

bool symbol_table_insert(SymbolTable *restrict symbol_table, StringView name,
                         Type *type, Value value) {
  assert(symbol_table != NULL);
  if (symbol_table_full(symbol_table)) {
    size_t capacity = nearest_power_of_two(symbol_table->capacity + 1);
    symbol_table_grow(symbol_table, capacity);
  }

  SymbolTableElement *element =
      symbol_table_find(symbol_table->elements, symbol_table->capacity, name);

  // if the element already exists,
  // we return false. we don't want
  // to reassign a global so as to
  // not define conflicting global
  // symbols in the object file.
  if (element->name.ptr != NULL) {
    return 0;
  }

  // if we are replacing an empty element, we increment
  // the count (+1 element). otherwise we are replacing a tombstone
  // so we don't increment the count (-1 tombstone +1 element == +0).
  if (element->value.kind == VALUEKIND_UNINITIALIZED) {
    symbol_table->count += 1;
  }

  element->name  = name;
  element->type  = type;
  element->value = value;
  return 1;
}

SymbolTableElement *symbol_table_lookup(SymbolTable *restrict symbol_table,
                                        StringView name) {
  SymbolTableElement *element =
      symbol_table_find(symbol_table->elements, symbol_table->capacity, name);

  if (element->name.ptr == NULL) {
    return NULL;
  } else {
    return element;
  }
}

bool symbol_table_delete(SymbolTable *restrict symbol_table, StringView name) {
  if (symbol_table->count == 0) {
    return 0;
  }

  SymbolTableElement *element =
      symbol_table_find(symbol_table->elements, symbol_table->capacity, name);
  if (element == NULL) {
    return 0;
  }

  element->name  = string_view_create();
  element->type  = NULL;
  element->value = value_create();
  return 1;
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
    } while ((iter->element->name.ptr == NULL) && (iter->element != iter->end));
  }
}

bool symbol_table_iterator_done(SymbolTableIterator *restrict iter) {
  assert(iter != NULL);

  return iter->element == iter->end;
}