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
#include "utility/string_view.h"

typedef struct FormalArgument {
  StringView name;
  Type *type;
} FormalArgument;

typedef struct FormalArgumentList {
  FormalArgument *list;
  u64 size;
  u64 capacity;
} FormalArgumentList;

FormalArgumentList formal_argument_list_create();
void formal_argument_list_destroy(FormalArgumentList *restrict fal);
void formal_argument_list_clone(FormalArgumentList *target,
                                FormalArgumentList *source);
bool formal_argument_list_equality(FormalArgumentList *a1,
                                   FormalArgumentList *a2);

void formal_argument_list_append(FormalArgumentList *restrict fal,
                                 StringView name, Type *type);

typedef struct Function {
  StringView name;
  FormalArgumentList arguments;
  Type *return_type;
  Bytecode body;
} Function;

Function function_create();
void function_destroy(Function *restrict function);
void function_clone(Function *target, Function *source);
bool function_equality(Function *f1, Function *f2);
struct Context;
Type *function_type_of(Function const *restrict f,
                       struct Context *restrict context);

void function_add_argument(Function *restrict function, StringView name,
                           Type *type);

#endif // !EXP_IMR_FUNCTION_H