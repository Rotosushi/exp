// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_FUNCTION_H
#define EXP_IMR_FUNCTION_H

#include "imr/block.h"
#include "imr/local.h"
#include "imr/type.h"

typedef struct FormalArguments {
    u8 length;
    u8 capacity;
    Local *buffer;
} FormalArguments;

typedef struct Locals {
    u32 length;
    u32 capacity;
    Local *buffer;
} Locals;

typedef struct Function {
    FormalArguments arguments;
    Locals locals;
    Type const *return_type;
    Block block;
} Function;

void function_initialize(Function *function_body);
void function_terminate(Function *function);

void function_arguments_append(Function *function, Local arg);
Local *function_arguments_lookup(Function *function, StringView name);
Local *function_arguments_at(Function *function, u8 index);

u32 function_declare_local(Function *function);
Local *function_local_at(Function *function, u32 ssa);

void function_append_instruction(Function *function, Instruction instruction);

struct Context;
void print_function(String *buffer, Function const *function,
                    struct Context *context);

#endif // !EXP_IMR_FUNCTION_H
