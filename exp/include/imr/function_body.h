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

#include "imr/bytecode.h"
#include "imr/type.h"
#include "support/string_view.h"

typedef struct FormalArgument {
    StringView name;
    Type *type;
    u8 index;
    u32 ssa;
} FormalArgument;

typedef struct FormalArgumentList {
    FormalArgument *list;
    u8 size;
    u8 capacity;
} FormalArgumentList;

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 FormalArgument arg);
FormalArgument *formal_argument_list_at(FormalArgumentList *restrict fal,
                                        u8 index);
FormalArgument *formal_argument_list_lookup(FormalArgumentList *restrict fal,
                                            StringView name);

typedef struct LocalVariable {
    StringView name;
    Type *type;
    u32 ssa;
} LocalVariable;

typedef struct LocalVariables {
    u64 size;
    u64 capacity;
    LocalVariable *buffer;
} LocalVariables;

void local_variables_append(LocalVariables *restrict lv, LocalVariable var);
LocalVariable *local_variables_lookup(LocalVariables *restrict lv,
                                      StringView name);
LocalVariable *local_variables_lookup_ssa(LocalVariables *restrict lv, u32 ssa);

/*
 * #TODO: The body of a function is currently a single block of instructions.
 * This will need to be changed to a list of blocks, with the addition of
 * instructions to jump between blocks. This will allow for structured control
 * flow.
 */

typedef struct FunctionBody {
    FormalArgumentList arguments;
    LocalVariables locals;
    Type *return_type;
    u32 ssa_count;
    Bytecode bc;
} FunctionBody;

FunctionBody function_body_create();
void function_body_destroy(FunctionBody *restrict function);

void function_body_new_argument(FunctionBody *restrict function,
                                FormalArgument arg);
void function_body_new_local(FunctionBody *restrict function,
                             StringView name,
                             u32 ssa);
Operand function_body_new_ssa(FunctionBody *restrict function);

struct Context;
void print_function_body(String *restrict string,
                         FunctionBody const *restrict function,
                         struct Context *restrict context);

#endif // !EXP_IMR_FUNCTION_H
