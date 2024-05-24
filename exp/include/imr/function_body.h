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
#include "utility/io.h"
#include "utility/string_view.h"

typedef struct FormalArgument {
  StringView name;
  Type *type;
} FormalArgument;

typedef struct FormalArgumentList {
  FormalArgument *list;
  u8 size;
  u8 capacity;
} FormalArgumentList;

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 FormalArgument arg);

typedef struct LocalVariable {
  StringView name;
  Type *type;
  u16 ssa;
} LocalVariable;

typedef struct LocalVariables {
  u16 size;
  u16 capacity;
  LocalVariable *buffer;
} LocalVariables;

void local_variables_append(LocalVariables *restrict lv, LocalVariable var);
LocalVariable *local_variables_lookup(LocalVariables *restrict lv,
                                      StringView name);
LocalVariable *local_variables_lookup_ssa(LocalVariables *restrict lv, u16 ssa);

typedef struct FunctionBody {
  FormalArgumentList arguments;
  LocalVariables locals;
  Type *return_type;
  u16 ssa_count;
  Bytecode bc;
} FunctionBody;

FunctionBody function_body_create();
void function_body_destroy(FunctionBody *restrict function);

void function_body_new_local(FunctionBody *restrict function,
                             StringView name,
                             u16 ssa);
Operand function_body_new_ssa(FunctionBody *restrict function);

void print_function_body(FunctionBody const *restrict f, FILE *restrict file);

#endif // !EXP_IMR_FUNCTION_H