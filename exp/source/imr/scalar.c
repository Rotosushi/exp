/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file imr/scalar.c
 */

#include "imr/scalar.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

static Scalar scalar_construct(ScalarKind kind, ScalarData data) {
    Scalar scalar = {.kind = kind, .data = data};
    return scalar;
}

Scalar scalar_uninitialized() {
    return scalar_construct(SCALAR_UNINITIALIZED, (ScalarData){});
}

Scalar scalar_nil() {
    return scalar_construct(SCALAR_NIL, (ScalarData){.nil = 0});
}

Scalar scalar_bool(bool bool_) {
    return scalar_construct(SCALAR_BOOL, (ScalarData){.bool_ = bool_});
}

Scalar scalar_u8(u8 u8_) {
    return scalar_construct(SCALAR_U8, (ScalarData){.u8_ = u8_});
}

Scalar scalar_u16(u16 u16_) {
    return scalar_construct(SCALAR_U16, (ScalarData){.u16_ = u16_});
}

Scalar scalar_u32(u32 u32_) {
    return scalar_construct(SCALAR_U32, (ScalarData){.u32_ = u32_});
}

Scalar scalar_u64(u64 u64_) {
    return scalar_construct(SCALAR_U64, (ScalarData){.u64_ = u64_});
}

Scalar scalar_i8(i8 i8_) {
    return scalar_construct(SCALAR_I8, (ScalarData){.i8_ = i8_});
}

Scalar scalar_i16(i16 i16_) {
    return scalar_construct(SCALAR_I16, (ScalarData){.i16_ = i16_});
}

Scalar scalar_i32(i32 i32_) {
    return scalar_construct(SCALAR_I32, (ScalarData){.i32_ = i32_});
}

Scalar scalar_i64(i64 i64_) {
    return scalar_construct(SCALAR_I64, (ScalarData){.i64_ = i64_});
}

bool scalar_equal(Scalar A, Scalar B) {
    if (A.kind != B.kind) { return false; }
    switch (A.kind) {
    // #NOTE: iff the scalar's are uninitialized, then strictly speaking
    //  they are never to be considered equal. because their values are
    //  unknowable. and thus we cannot know if they are equal or not.
    case SCALAR_UNINITIALIZED: return false;
    case SCALAR_NIL:           return true;
    case SCALAR_BOOL:          return A.data.bool_ == B.data.bool_;
    case SCALAR_U8:            return A.data.u8_ == B.data.u8_;
    case SCALAR_U16:           return A.data.u16_ == B.data.u16_;
    case SCALAR_U32:           return A.data.u32_ == B.data.u32_;
    case SCALAR_U64:           return A.data.u64_ == B.data.u64_;
    case SCALAR_I8:            return A.data.i8_ == B.data.i8_;
    case SCALAR_I16:           return A.data.i16_ == B.data.i16_;
    case SCALAR_I32:           return A.data.i32_ == B.data.i32_;
    case SCALAR_I64:           return A.data.i64_ == B.data.i64_;
    default:                   EXP_UNREACHABLE();
    }
}

bool scalar_is_index(Scalar scalar) {
    switch (scalar.kind) {
    case SCALAR_U8:
    case SCALAR_U16:
    case SCALAR_U32:
    case SCALAR_U64: return true;
    case SCALAR_I8:  return scalar.data.i8_ >= 0;
    case SCALAR_I16: return scalar.data.i16_ >= 0;
    case SCALAR_I32: return scalar.data.i32_ >= 0;
    case SCALAR_I64: return scalar.data.i64_ >= 0;
    default:         return false;
    }
}

u64 scalar_index(Scalar scalar) {
    EXP_ASSERT(scalar_is_index(scalar));
    switch (scalar.kind) {
    case SCALAR_U8:  return scalar.data.u8_;
    case SCALAR_U16: return scalar.data.u16_;
    case SCALAR_U32: return scalar.data.u32_;
    case SCALAR_U64: return scalar.data.u64_;
    case SCALAR_I8:
        EXP_ASSERT(scalar.data.i8_ >= 0);
        return (u64)scalar.data.i8_;
    case SCALAR_I16:
        EXP_ASSERT(scalar.data.i16_ >= 0);
        return (u64)scalar.data.i16_;
    case SCALAR_I32:
        EXP_ASSERT(scalar.data.i32_ >= 0);
        return (u64)scalar.data.i32_;
    case SCALAR_I64:
        EXP_ASSERT(scalar.data.i64_ >= 0);
        return (u64)scalar.data.i64_;
    default: EXP_UNREACHABLE();
    }
}

void print_scalar(String *buffer, Scalar A) {
    switch (A.kind) {
    case SCALAR_UNINITIALIZED: string_append(buffer, SV("uninitialized"));
    case SCALAR_NIL:           string_append(buffer, SV("nil"));
    case SCALAR_BOOL:
        string_append(buffer, A.data.bool_ ? SV("true") : SV("false"));
    case SCALAR_U8:  string_append_u64(buffer, A.data.u8_);
    case SCALAR_U16: string_append_u64(buffer, A.data.u16_);
    case SCALAR_U32: string_append_u64(buffer, A.data.u32_);
    case SCALAR_U64: string_append_u64(buffer, A.data.u64_);
    case SCALAR_I8:  string_append_i64(buffer, A.data.i8_);
    case SCALAR_I16: string_append_i64(buffer, A.data.i16_);
    case SCALAR_I32: string_append_i64(buffer, A.data.i32_);
    case SCALAR_I64: string_append_i64(buffer, A.data.i64_);
    default:         EXP_UNREACHABLE();
    }
}
