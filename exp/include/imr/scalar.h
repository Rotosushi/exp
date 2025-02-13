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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_SCALAR_H
#define EXP_IMR_SCALAR_H

#include "utility/int_types.h"
#include "utility/string.h"

// #NOTE: the idea behind the Scalar type is that it represents a value that can
//  fit in an abstract register of the abstract machine, where as a Value can be
//  a scalar or it can be a larger layout type. (and it just so happens that we
//  use a size which fits in an actual register on an actual 64 bit machine)

typedef enum ScalarKind : u8 {
    SCALAR_UNINITIALIZED,
    // SCALAR_NIL,
    // SCALAR_BOOL,
    // SCALAR_U8,
    // SCALAR_U16,
    // SCALAR_U32,
    // SCALAR_U64,
    // SCALAR_I8,
    // SCALAR_I16,
    SCALAR_I32,
    // SCALAR_I64,
} ScalarKind;

typedef union ScalarData {
    // bool nil;
    // bool bool_;
    //  u8 u8_;
    //  u16 u16_;
    //  u32 u32_;
    //  u64 u64_;
    //  i8 i8_;
    //  i16 i16_;
    i32 i32_;
} ScalarData;

typedef struct Scalar {
    ScalarKind kind;
    ScalarData data;
} Scalar;

Scalar scalar_create();
// Scalar scalar_nil();
// Scalar scalar_bool(bool bool_);
Scalar scalar_i32(i32 i32_);

bool scalar_equality(Scalar A, Scalar B);

void print_scalar(String *buffer, Scalar A);

#endif // EXP_IMR_SCALAR_H
