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

#include "backend/x64_function_body.h"

typedef struct X64Symbol {
  StringView name;
  X64FunctionBody body;
} X64Symbol;

void x64symbol_destroy(X64Symbol *restrict symbol);

typedef struct X64Symbols {
  u64 count;
  X64Symbol *buffer;
} X64Symbols;

X64Symbols x64symbols_create(u64 count);
void x64symbols_destroy(X64Symbols *restrict symbols);
X64Symbol *x64symbols_at(X64Symbols *restrict symbols, StringView name);

#endif // !EXP_BACKEND_X64_SYMBOLS_H