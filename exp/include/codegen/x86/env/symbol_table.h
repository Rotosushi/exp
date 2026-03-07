// Copyright (C) 2026 Cade Weinberg
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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_CODEGEN_X86_ENV_SYMBOL_TABLE_H
#define EXP_CODEGEN_X86_ENV_SYMBOL_TABLE_H

#include "codegen/x86/imr/value.h"
#include "support/string_view.h"

typedef struct x86_Symbol {
    StringView  name;
    Type const *type;
    x86_Value  *value;
} x86_Symbol;

typedef struct x86_SymbolTable {
    u32          count;
    u32          capacity;
    x86_Symbol **elements;
} x86_SymbolTable;

void x86_symbol_table_create(x86_SymbolTable *restrict table);
void x86_symbol_table_destroy(x86_SymbolTable *restrict table);

x86_Symbol *x86_symbol_table_at(x86_SymbolTable *restrict table,
                                StringView name);

#endif // !EXP_CODEGEN_X86_ENV_SYMBOL_TABLE_H
