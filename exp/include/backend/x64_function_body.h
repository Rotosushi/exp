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
#ifndef EXP_BACKEND_X64_FUNCTION_BODY_H
#define EXP_BACKEND_X64_FUNCTION_BODY_H

#include "backend/x64_allocation.h"
#include "backend/x64_bytecode.h"

typedef struct X64FormalArgument {
  X64Allocation allocation;
} X64FormalArgument;

typedef struct X64FormalArguments {
  u8 size;
  X64FormalArgument *buffer;
} X64FormalArguments;

X64FormalArguments x64formal_arguments_create(u8 size);
X64FormalArgument *x64formal_arguments_at(X64FormalArguments *restrict args,
                                          u8 idx);
void x64formal_arguments_assign(X64FormalArguments *restrict args, u8 idx,
                                X64Allocation allocation);

typedef struct X64FunctionBody {
  X64FormalArguments arguments;
  X64Allocation return_allocation;
  u16 stack_size;
  X64Bytecode bc;
} X64FunctionBody;

X64FunctionBody x64function_body_create(u8 arg_count);
void x64function_body_destroy(X64FunctionBody *restrict body);

#endif // !EXP_BACKEND_X64_FUNCTION_BODY_H