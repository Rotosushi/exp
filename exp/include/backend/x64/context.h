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
#ifndef EXP_BACKEND_X64_CONTEXT_H
#define EXP_BACKEND_X64_CONTEXT_H

#include "backend/x64/symbols.h"
#include "env/context.h"

typedef struct x64_Context {
  Context *context;
  x64_SymbolTable symbols;
} x64_Context;

// x64 context functions
x64_Context x64_context_create(Context *restrict context);
void x64_context_destroy(x64_Context *restrict x64_context);

// context functions
// context global symbol table functions
StringView x64_context_global_symbols_at(x64_Context *restrict x64_context,
                                         u16 idx);

// context function functions
FunctionBody *x64_context_enter_function(x64_Context *restrict x64_context,
                                         StringView name);
void x64_context_leave_function(x64_Context *restrict context);

ActualArgumentList *x64_context_call_at(x64_Context *restrict x64_context,
                                        u16 idx);
FormalArgument *x64_context_argument_at(x64_Context *restrict x64_context,
                                        u8 index);

// x64 symbol table functions
x64_Symbol *x64_context_symbol(x64_Context *restrict x64_context,
                               StringView name);

#endif // !EXP_BACKEND_X64_CONTEXT_H