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
#ifndef EXP_ENV_SYMBOL_TABLE_H
#define EXP_ENV_SYMBOL_TABLE_H

#include "imr/type.h"
#include "imr/value.h"
#include "utility/string_view.h"

typedef struct SymbolTableElement {
  StringView name;
  Type *type;
  Value value;
} SymbolTableElement;

typedef struct SymbolTable {
  size_t count;
  size_t capacity;
  SymbolTableElement *elements;
} SymbolTable;

SymbolTable symbol_table_create();
void symbol_table_destroy(SymbolTable *restrict symbol_table);

/**
 * @brief create and insert a new entry into the symbol table.
 *
 * @note if the entry already exists, the function returns false.
 *
 * @param symbol_table
 * @param name
 * @param type
 * @param value
 * @return true if the entry is new
 * @return false if the entry already exists.
 */
bool symbol_table_insert(SymbolTable *restrict symbol_table, StringView name,
                         Type *type, Value value);

/**
 * @brief lookup an existing entry in the symbol_table.
 *
 * @note returns NULL when no entry was found
 *
 * @param symbol_table
 * @param name
 * @return SymbolTableElement*
 */
SymbolTableElement *symbol_table_lookup(SymbolTable *restrict symbol_table,
                                        StringView name);

/**
 * @brief delete an existing entry
 *
 * @param symbol_table
 * @param name
 * @return true if the entry was removed
 * @return false if nothing was removed
 */
bool symbol_table_delete(SymbolTable *restrict symbol_table, StringView name);

typedef struct SymbolTableIterator {
  SymbolTableElement *element;
  SymbolTableElement *end;
} SymbolTableIterator;

SymbolTableIterator
symbol_table_iterator_create(SymbolTable *restrict symbol_table);

void symbol_table_iterator_next(SymbolTableIterator *restrict iter);

bool symbol_table_iterator_done(SymbolTableIterator *restrict iter);

#endif // !EXP_ENV_SYMBOL_TABLE_H