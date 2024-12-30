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

typedef struct FormalArgument {
    StringView name;
    Type const *type;
    u64 ssa;
} FormalArgument;

typedef struct FormalArgumentList {
    FormalArgument *list;
    u8 size;
    u8 capacity;
} FormalArgumentList;

typedef struct FunctionBody {
    FormalArgumentList arguments;
    Type const *return_type;
    LocalAllocator allocator;
    Block block;
} FunctionBody;

void function_body_initialize(FunctionBody *function_body);
void function_body_terminate(FunctionBody *function);

void function_body_allocate_argument(FunctionBody *function,
                                     FormalArgument arg);
FormalArgument *function_body_arguments_lookup(FunctionBody *function,
                                               StringView name);
FormalArgument *function_body_arguments_at(FunctionBody *function, u8 index);

u64 function_body_declare_local(FunctionBody *function);
Local *function_body_local_at(FunctionBody *function, u64 ssa);
Local *function_body_local_at_name(FunctionBody *function, StringView name);
void function_body_allocate_local(FunctionBody *function,
                                  Local *local,
                                  u64 block_index);

void function_body_append_instruction(FunctionBody *function,
                                      Instruction instruction);

struct Context;
void print_function_body(String *buffer,
                         FunctionBody const *f,
                         struct Context *context);

#endif // !EXP_IMR_FUNCTION_H
