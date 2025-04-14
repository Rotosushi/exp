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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X86_SYMBOLS_H
#define EXP_BACKEND_X86_SYMBOLS_H

#include "codegen/x86/imr/function.h"

typedef struct x86_Symbol {
    StringView   name;
    x86_Function body;
} x86_Symbol;

void x86_symbol_destroy(x86_Symbol *restrict symbol);

typedef struct x64_SymbolTable {
    u64         count;
    x86_Symbol *buffer;
} x86_SymbolTable;

x86_SymbolTable x86_symbol_table_create(u64 count);
void            x86_symbol_table_destroy(x86_SymbolTable *restrict symbols);
x86_Symbol     *x86_symbol_table_at(x86_SymbolTable *restrict symbols,
                                    StringView name);

#endif // !EXP_BACKEND_X86_SYMBOLS_H
