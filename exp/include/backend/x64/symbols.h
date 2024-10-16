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

typedef enum x64_SymbolKind : u8 {
  X64SYM_UNDEFINED,
  X64SYM_FUNCTION,
  X64SYM_CONSTANT,
} x64_SymbolKind;

typedef struct x64_Symbol {
  x64_SymbolKind kind;
  StringView name;
  Type *type;
  x64_FunctionBody body;
} x64_Symbol;

void x64_symbol_destroy(x64_Symbol *restrict symbol);

typedef struct x64_SymbolTable {
  u64 size;
  u64 capacity;
  x64_Symbol *buffer;
} x64_SymbolTable;

x64_SymbolTable x64_symbol_table_create();
void x64_symbol_table_destroy(x64_SymbolTable *restrict symbols);
x64_Symbol *x64_symbol_table_at(x64_SymbolTable *restrict symbols,
                                StringView name);

typedef struct x64_SymbolIterator {
  x64_Symbol *symbol;
  x64_Symbol *end;
} x64_SymbolIterator;

x64_SymbolIterator x64_symbol_iterator(x64_SymbolTable *restrict table);

void x64_symbol_iterator_next(x64_SymbolIterator *restrict iter);

bool x64_symbol_iterator_done(x64_SymbolIterator *restrict iter);

#endif // !EXP_BACKEND_X64_SYMBOLS_H
