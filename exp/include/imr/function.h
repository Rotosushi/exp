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
#include "imr/locals.h"
#include "imr/type.h"
#include "support/string_view.h"

typedef struct FormalArgumentList {
    u8      size;
    u8      capacity;
    Local **list;
} FormalArgumentList;

/*
 * #TODO: The body of a function is currently a single block of instructions.
 * This will need to be changed to a list of blocks, with the addition of
 * instructions to jump between blocks. This will allow for structured control
 * flow.
 *
 * something like
 * typedef struct Body {
 *  u32 size;
 *  u32 capacity;
 *  Bytecode *buffer;
 * } Body;
 */

typedef struct Function {
    FormalArgumentList arguments;
    Locals             locals;
    Bytecode           bc;
    Type const        *return_type;
} Function;

void function_create(Function *restrict function);
void function_destroy(Function *restrict function);

Local *function_declare_argument(Function *restrict function);
Local *function_declare_local(Function *restrict function);
Local *function_lookup_argument(Function *restrict function, u8 index);
Local *function_lookup_local(Function *restrict function, u32 ssa);
Local *function_lookup_local_name(Function *restrict function, StringView name);

struct Context;
void print_function(String *restrict string,
                    Function const *restrict function,
                    struct Context *restrict context);

#endif // !EXP_IMR_FUNCTION_H
