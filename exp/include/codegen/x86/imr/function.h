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
#include "imr/function.h"
#include "imr/type.h"

typedef struct x86_FormalArgument {
    Type const *type;
} x86_FormalArgument;

typedef struct x86_FormalArgumentList {
    u8                  length;
    x86_FormalArgument *buffer;
} x86_FormalArgumentList;

typedef struct x86_Function {
    x86_FormalArgumentList arguments;
    x86_Bytecode           body;
    x86_Locations          locations;
    x86_GPRP               gprp;
    x86_Location           return_location;
} x86_Function;

void x86_function_create(x86_Function *restrict function);
void x86_function_destroy(x86_Function *restrict function);

void x86_function_setup(x86_Function *restrict x86_function,
                        Function const *restrict function,
                        Context *restrict context);

x86_FormalArgument const *
x86_function_formal_argument_at(x86_Function *restrict x86_function, u8 index);

void x86_function_header(x86_Function *restrict x86_function);
void x86_function_footer(x86_Function *restrict x86_function);

void x86_function_append(x86_Function *restrict function,
                         x86_Instruction instruction);

#endif // !EXP_BACKEND_X86_FUNCTION_BODY_H
