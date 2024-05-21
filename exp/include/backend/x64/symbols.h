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
#ifndef EXP_BACKEND_X64_SYMBOLS_H
#define EXP_BACKEND_X64_SYMBOLS_H

#include "backend/x64/function_body.h"

typedef struct x64_Symbol {
  StringView name;
  x64_FunctionBody body;
} x64_Symbol;

void x64_symbol_destroy(x64_Symbol *restrict symbol);

typedef struct x64_SymbolTable {
  u64 count;
  x64_Symbol *buffer;
} x64_SymbolTable;

x64_SymbolTable x64_symbol_table_create(u64 count);
void x64_symbol_table_destroy(x64_SymbolTable *restrict symbols);
x64_Symbol *x64_symbol_table_at(x64_SymbolTable *restrict symbols,
                                StringView name);

#endif // !EXP_BACKEND_X64_SYMBOLS_H