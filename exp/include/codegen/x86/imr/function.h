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

#include "codegen/x86/imr/body.h"
#include "codegen/x86/imr/local_allocator.h"
#include "env/context.h"
#include "imr/function.h"

typedef struct x86_FormalArgumentList {
    u8               length;
    u8               capacity;
    x86_Allocation **buffer;
} x86_FormalArgumentList;

void x86_formal_argument_list_append(x86_FormalArgumentList *restrict arguments,
                                     x86_Allocation *allocation);

typedef struct x86_Function {
    StringView             name;
    x86_FormalArgumentList arguments;
    x86_Body               body;
    x86_LocalAllocator     local_allocator;
    x86_Allocation        *result;
    u32                    current_block;
} x86_Function;

void x86_function_create(x86_Function *restrict function);
void x86_function_destroy(x86_Function *restrict function);

void x86_function_setup(x86_Function *restrict x86_function,
                        Function const *restrict function,
                        Context *restrict context);

x86_Allocation *x86_function_formal_argument_at(x86_Function *restrict function,
                                                u8 index);

x86_Allocation *x86_function_allocation_at(x86_Function *restrict function,
                                           u32 ssa);

void x86_function_insert_block(x86_Function *restrict function, u32 position);
void x86_function_prepend_block(x86_Function *restrict function);
u32  x86_function_append_block(x86_Function *restrict function);

void x86_function_target_block(x86_Function *restrict function, u32 block);
u32  x86_function_current_block(x86_Function *restrict function);

void x86_function_insert(x86_Function *restrict function,
                         x86_Instruction instruction,
                         u32             block_index);
void x86_function_prepend(x86_Function *restrict function,
                          x86_Instruction instruction);
void x86_function_append(x86_Function *restrict function,
                         x86_Instruction instruction);

void x86_function_header(x86_Function *restrict function);
void x86_function_footer(x86_Function *restrict function);

#endif // !EXP_BACKEND_X86_FUNCTION_BODY_H
