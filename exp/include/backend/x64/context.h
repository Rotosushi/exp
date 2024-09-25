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
  FunctionBody *body;
  x64_FunctionBody *x64_body;
  // x64_FunctionBody _init;
  x64_SymbolTable symbols;
} x64_Context;

// x64 context functions
x64_Context x64_context_create(Context *restrict context);
void x64_context_destroy(x64_Context *restrict x64_context);

// context functions
// context constants functions
Value *x64_context_value_at(x64_Context *restrict context, u64 index);

// context global symbol table functions
StringView x64_context_global_labels_at(x64_Context *restrict x64_context,
                                        u64 idx);

// context x64 function functions
void x64_context_enter_global(x64_Context *restrict x64_context,
                              StringView name);
void x64_context_leave_global(x64_Context *restrict context);

ActualArgumentList *x64_context_call_at(x64_Context *restrict x64_context,
                                        u64 idx);
FormalArgument *x64_context_argument_at(x64_Context *restrict x64_context,
                                        u8 index);

FunctionBody *current_body(x64_Context *restrict x64_context);
Bytecode *current_bc(x64_Context *restrict x64_context);
LocalVariables *current_locals(x64_Context *restrict x64_context);
x64_FunctionBody *current_x64_body(x64_Context *restrict x64_context);
x64_Bytecode *current_x64_bc(x64_Context *restrict x64_context);
x64_Allocator *current_x64_allocator(x64_Context *restrict x64_context);

u64 x64_context_current_offset(x64_Context *restrict x64_context);
void x64_context_insert(x64_Context *restrict x64_context,
                        x64_Instruction I,
                        u64 offset);
void x64_context_prepend(x64_Context *restrict x64_context, x64_Instruction I);
void x64_context_append(x64_Context *restrict x64_context, x64_Instruction I);

LocalVariable *x64_context_lookup_ssa(x64_Context *restrict x64_context,
                                      u64 ssa);

bool x64_context_uses_stack(x64_Context *restrict x64_context);
i64 x64_context_stack_size(x64_Context *restrict x64_context);

x64_Allocation *x64_context_allocation_of(x64_Context *restrict x64_context,
                                          u64 ssa);

void x64_context_release_gpr(x64_Context *restrict x64_context,
                             x64_GPR gpr,
                             u64 Idx);

void x64_context_aquire_gpr(x64_Context *restrict x64_context,
                            x64_GPR gpr,
                            u64 Idx);

x64_Allocation *x64_context_allocate(x64_Context *restrict x64_context,
                                     LocalVariable *restrict local,
                                     u64 Idx);

x64_Allocation *
x64_context_allocate_from_active(x64_Context *restrict x64_context,
                                 LocalVariable *local,
                                 x64_Allocation *active,
                                 u64 Idx);

x64_Allocation *x64_context_allocate_to_gpr(x64_Context *restrict x64_context,
                                            LocalVariable *local,
                                            x64_GPR gpr,
                                            u64 Idx);

x64_Allocation *x64_context_allocate_to_stack(x64_Context *restrict x64_context,
                                              LocalVariable *local,
                                              i64 offset);

x64_Allocation *x64_context_allocate_result(x64_Context *restrict x64_context,
                                            x64_Location location,
                                            Type *restrict type);

void x64_context_reallocate_active(x64_Context *restrict x64_context,
                                   x64_Allocation *restrict active);

x64_GPR x64_context_aquire_any_gpr(x64_Context *restrict x64_context, u64 Idx);

// x64 symbol table functions
x64_Symbol *x64_context_symbol(x64_Context *restrict x64_context,
                               StringView name);

#endif // !EXP_BACKEND_X64_CONTEXT_H
