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
#ifndef EXP_BACKEND_X86_FUNCTION_BODY_H
#define EXP_BACKEND_X86_FUNCTION_BODY_H

#include "codegen/x86/imr/allocator.h"
#include "codegen/x86/imr/bytecode.h"
#include "imr/function.h"

typedef struct x64_FormalArgument {
    u8              index;
    x86_Allocation *allocation;
    Type           *type;
} x86_FormalArgument;

typedef struct x64_FormalArgumentList {
    u8                  size;
    x86_FormalArgument *buffer;
} x86_FormalArgumentList;

x86_FormalArgumentList x86_formal_argument_list_create(u8 size);
x86_FormalArgument *
x86_formal_argument_list_at(x86_FormalArgumentList *restrict args, u8 idx);

typedef struct x86_Function {
    x86_FormalArgumentList arguments;
    x86_Allocation        *result;
    x86_Bytecode           bc;
    x86_Allocator          allocator;
} x86_Function;

void x86_function_create(x86_Function *restrict x86_body,
                         Function *restrict body);
void x86_function_destroy(x86_Function *restrict body);

#endif // !EXP_BACKEND_X86_FUNCTION_BODY_H
