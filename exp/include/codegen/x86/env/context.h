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

#include "codegen/x86/imr/layout.h"
#include "env/context.h"

typedef struct x86_Context {
    x86_Layouts layouts;
} x86_Context;

void *x86_context_allocate();
void  x86_context_deallocate(void *restrict context);

x86_Layout const *x86_context_layout_of(Context *restrict context,
                                        Type const *type);

#endif // !EXP_CODEGEN_X86_ENV_CONTEXT_H
