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
#ifndef EXP_IMR_FUNCTION_H
#define EXP_IMR_FUNCTION_H

#include "imr/block.h"
#include "imr/local_allocator.h"
#include "imr/type.h"

typedef struct FormalArgumentList {
    Local **list;
    u8 size;
    u8 capacity;
} FormalArgumentList;

typedef struct Function {
    FormalArgumentList arguments;
    Type const *return_type;
    LocalAllocator allocator;
    Block block;
} Function;

void function_initialize(Function *function_body);
void function_terminate(Function *function);

void function_append_argument(Function *function, Local *arg);
Local *function_arguments_lookup(Function *function, StringView name);
Local *function_arguments_at(Function *function, u8 index);

u32 function_declare_local(Function *function);
Local *function_local_at(Function *function, u32 ssa);
// Local *function_local_at_name(Function *function, StringView name);

/*
void function_allocate_result(Function *function, Local *local);
void function_allocate_formal_argument(Function *function,
                                       Local *local,
                                       u8 argument_index);
void function_allocate_actual_argument(Function *function,
                                       Local *local,
                                       u8 argument_index,
                                       u32 block_index);
*/
void function_allocate_local(Function *function, Local *local, u32 block_index);

void function_append_instruction(Function *function, Instruction instruction);

struct Context;
void print_function(String *buffer,
                    Function const *function,
                    struct Context *context);

#endif // !EXP_IMR_FUNCTION_H
