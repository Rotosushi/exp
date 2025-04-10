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

#include "imr/function.h"
#include "imr/type.h"
#include "support/string_view.h"

typedef enum SymbolKind {
    STE_UNDEFINED,
    STE_FUNCTION,
} SymbolKind;

typedef struct Symbol {
    StringView  name;
    Type const *type;
    SymbolKind  kind;
    union {
        u8       empty;
        Function function_body;
    };
} Symbol;

typedef struct SymbolTable {
    u64      count;
    u64      capacity;
    Symbol **elements;
} SymbolTable;

SymbolTable symbol_table_create();
void        symbol_table_destroy(SymbolTable *restrict symbol_table);

/**
 * @brief Return the entry associated with the given key in the
 * symbol table.
 *
 * @param symbol_table
 * @param name
 * @param type
 * @param value
 * @return SymbolTableElement *
 */
Symbol *symbol_table_at(SymbolTable *restrict symbol_table, StringView name);

typedef struct SymbolTableIterator {
    Symbol **element;
    Symbol **end;
} SymbolTableIterator;

SymbolTableIterator
symbol_table_iterator_create(SymbolTable *restrict symbol_table);

void symbol_table_iterator_next(SymbolTableIterator *restrict iter);

bool symbol_table_iterator_done(SymbolTableIterator *restrict iter);

#endif // !EXP_ENV_SYMBOL_TABLE_H
