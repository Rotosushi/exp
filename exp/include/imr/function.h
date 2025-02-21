// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file imr/function.h
 */

#ifndef EXP_IMR_FUNCTION_H
#define EXP_IMR_FUNCTION_H

#include "imr/block.h"
#include "imr/type.h"

typedef struct FormalArgument {
    StringView name;
    Type const *type;
} FormalArgument;

typedef struct FormalArguments {
    u8 length;
    u8 capacity;
    FormalArgument *buffer;
} FormalArguments;

typedef struct Function {
    FormalArguments arguments;
    Type const *return_type;
    Block block;
} Function;

void function_initialize(Function *function);
void function_terminate(Function *function);

void function_arguments_append(Function *function, FormalArgument arg);
FormalArgument *function_arguments_lookup(Function *function, StringView name);
FormalArgument function_arguments_at(Function *function, u8 index);

struct Context;
Operand function_append_return(Function *function, struct Context *context,
                               Operand result);
Operand function_append_call(Function *function, struct Context *context,
                             Operand label, Operand args);
Operand function_append_dot(Function *function, struct Context *context,
                            Operand src, Operand index);
Operand function_append_load(Function *function, struct Context *context,
                             Operand src);
Operand function_append_neg(Function *function, struct Context *context,
                            Operand src);
Operand function_append_add(Function *function, struct Context *context,
                            Operand left, Operand right);
Operand function_append_sub(Function *function, struct Context *context,
                            Operand left, Operand right);
Operand function_append_mul(Function *function, struct Context *context,
                            Operand left, Operand right);
Operand function_append_div(Function *function, struct Context *context,
                            Operand left, Operand right);
Operand function_append_mod(Function *function, struct Context *context,
                            Operand left, Operand right);

void print_function(String *buffer, Function const *function);

#endif // !EXP_IMR_FUNCTION_H
