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

FormalArgumentList formal_argument_list_create();
bool formal_argument_list_equality(FormalArgumentList *a1,
                                   FormalArgumentList *a2);

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 FormalArgument arg);

typedef struct FunctionBody {
  FormalArgumentList arguments;
  Type *return_type;
  u16 local_count;
  Bytecode bc;
} FunctionBody;

FunctionBody function_body_create();
void function_body_destroy(FunctionBody *restrict function);

struct Context;
Type *function_body_type_of(FunctionBody const *restrict f,
                            struct Context *restrict context);

void print_function_body(FunctionBody const *restrict f, FILE *restrict file);

#endif // !EXP_IMR_FUNCTION_H