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
 * @file imr/scalar.h
 */

#ifndef EXP_IMR_SCALAR_H
#define EXP_IMR_SCALAR_H

#include "utility/int_types.h"
#include "utility/string.h"

typedef enum ScalarKind : u8 {
    SCALAR_UNINITIALIZED,
    SCALAR_NIL,
    SCALAR_BOOL,
    SCALAR_U8,
    SCALAR_U16,
    SCALAR_U32,
    SCALAR_U64,
    SCALAR_I8,
    SCALAR_I16,
    SCALAR_I32,
    SCALAR_I64,
} ScalarKind;

typedef union ScalarData {
    bool nil;
    bool bool_;
    u8 u8_;
    u16 u16_;
    u32 u32_;
    u64 u64_;
    i8 i8_;
    i16 i16_;
    i32 i32_;
    i64 i64_;
} ScalarData;

typedef struct Scalar {
    ScalarKind kind;
    ScalarData data;
} Scalar;

Scalar scalar_uninitialized();
Scalar scalar_create(ScalarKind kind, ScalarData data);
Scalar scalar_nil();
Scalar scalar_bool(bool bool_);
Scalar scalar_u8(u8 u8_);
Scalar scalar_u16(u16 u16_);
Scalar scalar_u32(u32 u32_);
Scalar scalar_u64(u64 u64_);
Scalar scalar_i8(i8 i8_);
Scalar scalar_i16(i16 i16_);
Scalar scalar_i32(i32 i32_);
Scalar scalar_i64(i64 i64_);

bool scalar_equal(Scalar A, Scalar B);

/**
 * @brief returns true if the scalar is an index type (u8, u16, u32, u64)
 * or a positive signed index type (i8, i16, i32, i64) and false otherwise.
 */
bool scalar_is_index(Scalar scalar);

/**
 * @brief returns the index value of the scalar.
 *
 * @note the scalar must be an index type.
 */
u64 scalar_index(Scalar scalar);

void print_scalar(String *buffer, Scalar A);

#endif // EXP_IMR_SCALAR_H
