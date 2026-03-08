// Copyright (C) 2025 Cade Weinberg
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

#ifndef EXP_CODEGEN_X86_ENV_CONTEXT_H
#define EXP_CODEGEN_X86_ENV_CONTEXT_H

#include "codegen/x86/env/symbol_table.h"
#include "env/context.h"

typedef struct x86_Context {
    x86_SymbolTable symbols;
} x86_Context;

x86_Context *x86_context_allocate();
void         x86_context_deallocate(x86_Context *restrict context);

x86_Symbol *x86_context_symbol_table_at(x86_Context *context, StringView name);

#endif // !EXP_CODEGEN_X86_ENV_CONTEXT_H
