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
#include "imr/type.h"
#include "utility/io.h"
#include "utility/string_view.h"

typedef struct FormalArgument {
    StringView name;
    Type const *type;
    u8 index;
    u16 ssa;
} FormalArgument;

typedef struct FormalArgumentList {
    FormalArgument *list;
    u8 size;
    u8 capacity;
} FormalArgumentList;

/*
void formal_argument_list_append(FormalArgumentList *fal, FormalArgument arg);
FormalArgument *formal_argument_list_at(FormalArgumentList *fal, u8 index);
FormalArgument *formal_argument_list_lookup(FormalArgumentList *fal,
                                            StringView name);
*/
typedef struct LocalVariable {
    StringView name;
    Type const *type;
    u16 ssa;
} LocalVariable;

typedef struct LocalVariables {
    u64 size;
    u64 capacity;
    LocalVariable *buffer;
} LocalVariables;

/*
void local_variables_append(LocalVariables *lv, LocalVariable var);
LocalVariable *local_variables_lookup(LocalVariables *lv, StringView name);
LocalVariable *local_variables_lookup_ssa(LocalVariables *lv, u16 ssa);
*/
typedef struct FunctionBody {
    FormalArgumentList arguments;
    LocalVariables locals;
    Type const *return_type;
    u64 ssa_count;
    Block block;
} FunctionBody;

void function_body_initialize(FunctionBody *function_body);
void function_body_terminate(FunctionBody *function);

void function_body_new_argument(FunctionBody *function, FormalArgument arg);
FormalArgument *function_body_arguments_lookup(FunctionBody *function,
                                               StringView name);
FormalArgument *function_body_arguments_at(FunctionBody *function, u8 index);
void function_body_new_local(FunctionBody *function, StringView name, u16 ssa);
Operand function_body_new_ssa(FunctionBody *function);
LocalVariable *function_body_locals_lookup(FunctionBody *function,
                                           StringView name);
LocalVariable *function_body_locals_ssa(FunctionBody *function, u16 ssa);

struct Context;
void print_function_body(FunctionBody const *f,
                         FILE *file,
                         struct Context *context);

#endif // !EXP_IMR_FUNCTION_H
