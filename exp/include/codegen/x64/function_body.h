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
#ifndef EXP_BACKEND_X64_FUNCTION_BODY_H
#define EXP_BACKEND_X64_FUNCTION_BODY_H

#include "codegen/x64/allocator.h"
#include "codegen/x64/bytecode.h"
#include "imr/function_body.h"

typedef struct x64_FormalArgument {
    u8 index;
    x64_Allocation *allocation;
    Type *type;
} x64_FormalArgument;

typedef struct x64_FormalArgumentList {
    u8 size;
    x64_FormalArgument *buffer;
} x64_FormalArgumentList;

x64_FormalArgumentList x64_formal_argument_list_create(u8 size);
x64_FormalArgument *
x64_formal_argument_list_at(x64_FormalArgumentList *restrict args, u8 idx);

typedef struct x64_FunctionBody {
    x64_FormalArgumentList arguments;
    x64_Allocation *result;
    x64_Bytecode bc;
    x64_Allocator allocator;
} x64_FunctionBody;

struct x64_Context;
x64_FunctionBody x64_function_body_create(FunctionBody *restrict body,
                                          struct x64_Context *restrict context);
void x64_function_body_destroy(x64_FunctionBody *restrict body);

#endif // !EXP_BACKEND_X64_FUNCTION_BODY_H
