// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_SYMBOL_TABLE_H
#define EXP_ENV_SYMBOL_TABLE_H

#include "imr/function.h"

typedef struct Symbol {
    StringView name;
    Type const *type;
    Function function_body;
} Symbol;

typedef struct SymbolTable {
    u64 count;
    u64 capacity;
    Symbol **elements;
} SymbolTable;

void symbol_table_create(SymbolTable *symbol_table);
void symbol_table_destroy(SymbolTable *symbol_table);

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
Symbol *symbol_table_at(SymbolTable *symbol_table, StringView name);

#endif // !EXP_ENV_SYMBOL_TABLE_H
