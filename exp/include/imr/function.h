// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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

void function_append_instruction(Function *function, Instruction instruction);

void print_function(String *buffer, Function const *function);

#endif // !EXP_IMR_FUNCTION_H
