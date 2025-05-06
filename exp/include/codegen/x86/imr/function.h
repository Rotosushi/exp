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

#include "codegen/x86/imr/bytecode.h"
#include "codegen/x86/imr/gprp.h"
#include "codegen/x86/imr/locations.h"
#include "env/context.h"

typedef struct x86_FormalArgument {
    u8    index;
    Type *type;
} x86_FormalArgument;

typedef struct x86_FormalArgumentList {
    u8                  size;
    x86_FormalArgument *buffer;
} x86_FormalArgumentList;

typedef struct x86_Function {
    x86_FormalArgumentList arguments;
    x86_Bytecode           body;
    x86_Locations          locations;
    x86_GPRP               gprp;
} x86_Function;

void x86_function_create(x86_Function *restrict x86_body,
                         Function const *restrict body,
                         Context *restrict context);
void x86_function_destroy(x86_Function *restrict body);

void print_x86_function(String *restrict buffer,
                        x86_Function const *restrict function);

#endif // !EXP_BACKEND_X86_FUNCTION_BODY_H
