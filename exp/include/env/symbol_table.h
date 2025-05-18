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

#include "support/string_view.h"

struct Type;
struct Value;

typedef struct Symbol {
    StringView          name;
    struct Type const  *type;
    struct Value const *value;
} Symbol;

typedef struct SymbolTable {
    u64      count;
    u64      capacity;
    Symbol **elements;
} SymbolTable;

SymbolTable symbol_table_create();
void        symbol_table_destroy(SymbolTable *restrict symbol_table);

Symbol *symbol_table_at(SymbolTable *restrict symbol_table, StringView name);

#endif // !EXP_ENV_SYMBOL_TABLE_H
