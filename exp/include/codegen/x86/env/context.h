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
#ifndef EXP_BACKEND_X86_CONTEXT_H
#define EXP_BACKEND_X86_CONTEXT_H

#include "codegen/x86/env/symbols.h"
#include "env/context.h"

typedef struct x86_Context {
    x86_SymbolTable symbols;
    Context        *context;
    Function       *body;
    x64_Function   *x64_body;
} x86_Context;

// x64 context functions
x86_Context x86_context_create(Context *context);
void        x86_context_destroy(x86_Context *x64_context);

// x64 symbol table functions
x86_Symbol *x86_context_symbol(x86_Context *x86_context, StringView name);

// context functions
// context constants functions
Value *x86_context_value_at(x86_Context *x86_context, u32 index);

// context global symbol table functions
StringView x86_context_global_labels_at(x86_Context *x86_context, u32 index);

// context x64 function functions
void x86_context_enter_function(x86_Context *x86_context, StringView name);
void x86_context_leave_function(x86_Context *context);

FormalArgument *x86_context_argument_at(x86_Context *x86_context, u8 index);

Function       *x86_context_current_body(x86_Context *x86_context);
Bytecode       *x86_context_current_bc(x86_Context *x86_context);
LocalVariables *x86_context_current_locals(x86_Context *x86_context);
x64_Function   *x86_context_current_x86_body(x86_Context *x86_context);
x64_Bytecode   *x86_context_current_x86_bc(x86_Context *x86_context);
x86_Allocator  *x86_context_current_x86_allocator(x86_Context *x86_context);

u64  x86_context_current_offset(x86_Context *x86_context);
void x86_context_insert(x86_Context    *x86_context,
                        x64_Instruction I,
                        u64             offset);
void x86_context_prepend(x86_Context *x86_context, x64_Instruction I);
void x86_context_append(x86_Context *x86_context, x64_Instruction I);

LocalVariable *x86_context_lookup_ssa(x86_Context *x86_context, u32 ssa);

bool x86_context_uses_stack(x86_Context *x86_context);
i64  x86_context_stack_size(x86_Context *x86_context);

x86_Allocation *x86_context_allocation_of(x86_Context *x86_context, u32 ssa);

void x86_context_release_gpr(x86_Context *x86_context, x86_64_GPR gpr, u64 Idx);

void x86_context_aquire_gpr(x86_Context *x86_context, x86_64_GPR gpr, u64 Idx);

x86_Allocation *
x86_context_allocate(x86_Context *x86_context, LocalVariable *local, u64 Idx);

x86_Allocation *x86_context_allocate_from_active(x86_Context    *x86_context,
                                                 LocalVariable  *local,
                                                 x86_Allocation *active,
                                                 u64             Idx);

x86_Allocation *x86_context_allocate_to_any_gpr(x86_Context   *x86_context,
                                                LocalVariable *local);

x86_Allocation *x86_context_allocate_to_gpr(x86_Context *restrict x86_context,
                                            LocalVariable *local,
                                            x86_64_GPR     gpr,
                                            u64            Idx);

x86_Allocation *x86_context_allocate_to_stack(x86_Context   *x86_context,
                                              LocalVariable *local,
                                              i64            offset);

x86_Allocation *x86_context_allocate_result(x86_Context *x86_context,
                                            x64_Location location,
                                            Type        *type);

void x86_context_reallocate_active(x86_Context    *x86_context,
                                   x86_Allocation *active);

x86_64_GPR
x86_context_aquire_any_gpr(x86_Context *x86_context, u64 size, u64 Idx);

#endif // !EXP_BACKEND_X86_CONTEXT_H
