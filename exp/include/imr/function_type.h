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
#ifndef EXP_IMR_FUNCTION_TYPE_H
#define EXP_IMR_FUNCTION_TYPE_H

#include "imr/tuple_type.h"

typedef struct FunctionType {
    struct Type const *return_type;
    TupleType          argument_types;
} FunctionType;

void function_type_create(FunctionType *restrict function,
                          struct Type const *return_type,
                          TupleType          arguments);
void function_type_destroy(FunctionType *restrict function);

bool function_type_equal(FunctionType const *A, FunctionType const *B);

void print_function_type(String *restrict string,
                         FunctionType const *restrict function);

#endif // !EXP_IMR_FUNCTION_TYPE_H
