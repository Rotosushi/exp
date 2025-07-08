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
#ifndef EXP_IMR_TYPE_PRIMARY_H
#define EXP_IMR_TYPE_PRIMARY_H

#include "support/string.h"

typedef enum TypePrimaryKind {
    TYPE_PRIMARY_KIND_NIL,
    TYPE_PRIMARY_KIND_BOOL,
    TYPE_PRIMARY_KIND_U8,
    TYPE_PRIMARY_KIND_U16,
    TYPE_PRIMARY_KIND_U32,
    TYPE_PRIMARY_KIND_U64,
    TYPE_PRIMARY_KIND_I8,
    TYPE_PRIMARY_KIND_I16,
    TYPE_PRIMARY_KIND_I32,
    TYPE_PRIMARY_KIND_I64,
} TypePrimaryKind;

typedef struct TypePrimary {
    TypePrimaryKind kind;
} TypePrimary;

inline TypePrimary type_primary_nil() {
    return (TypePrimary){TYPE_PRIMARY_KIND_NIL};
}

inline TypePrimary type_primary_bool() {
    return (TypePrimary){TYPE_PRIMARY_KIND_BOOL};
}

inline TypePrimary type_primary_u8() {
    return (TypePrimary){TYPE_PRIMARY_KIND_U8};
}

inline TypePrimary type_primary_u16() {
    return (TypePrimary){TYPE_PRIMARY_KIND_U16};
}

inline TypePrimary type_primary_u32() {
    return (TypePrimary){TYPE_PRIMARY_KIND_U32};
}

inline TypePrimary type_primary_u64() {
    return (TypePrimary){TYPE_PRIMARY_KIND_U64};
}

inline TypePrimary type_primary_i8() {
    return (TypePrimary){TYPE_PRIMARY_KIND_I8};
}

inline TypePrimary type_primary_i16() {
    return (TypePrimary){TYPE_PRIMARY_KIND_I16};
}

inline TypePrimary type_primary_i32() {
    return (TypePrimary){TYPE_PRIMARY_KIND_I32};
}

inline TypePrimary type_primary_i64() {
    return (TypePrimary){TYPE_PRIMARY_KIND_I64};
}

inline bool type_primary_equality(TypePrimary A, TypePrimary B) {
    return A.kind == B.kind;
}

bool type_primary_is_integral(TypePrimary primary);
bool type_primary_is_index(TypePrimary primary);

void print_type_primary(String *restrict buffer, TypePrimary primary);

#endif // !EXP_IMR_TYPE_PRIMARY_H
