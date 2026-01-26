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

#include "imr/type/primary.h"
#include "support/assert.h"
#include "support/unreachable.h"

extern TypePrimary type_primary_nil();
extern TypePrimary type_primary_bool();
extern TypePrimary type_primary_u8();
extern TypePrimary type_primary_u16();
extern TypePrimary type_primary_u32();
extern TypePrimary type_primary_u64();
extern TypePrimary type_primary_i8();
extern TypePrimary type_primary_i16();
extern TypePrimary type_primary_i32();
extern TypePrimary type_primary_i64();
extern bool        type_primary_equality(TypePrimary A, TypePrimary B);

bool type_primary_is_integral(TypePrimary primary) {
    switch (primary.kind) {
    case TYPE_PRIMARY_KIND_U8:
    case TYPE_PRIMARY_KIND_U16:
    case TYPE_PRIMARY_KIND_U32:
    case TYPE_PRIMARY_KIND_U64:
    case TYPE_PRIMARY_KIND_I8:
    case TYPE_PRIMARY_KIND_I16:
    case TYPE_PRIMARY_KIND_I32:
    case TYPE_PRIMARY_KIND_I64: return true;

    default: return false;
    }
}

bool type_primary_is_index(TypePrimary primary) {
    return type_primary_is_integral(primary);
}

void print_type_primary(String *restrict buffer, TypePrimary primary) {
    EXP_ASSERT(buffer != NULL);

    switch (primary.kind) {
    case TYPE_PRIMARY_KIND_NIL:  string_append(buffer, SV("nil")); break;
    case TYPE_PRIMARY_KIND_BOOL: string_append(buffer, SV("bool")); break;
    case TYPE_PRIMARY_KIND_U8:   string_append(buffer, SV("u8")); break;
    case TYPE_PRIMARY_KIND_U16:  string_append(buffer, SV("u16")); break;
    case TYPE_PRIMARY_KIND_U32:  string_append(buffer, SV("u32")); break;
    case TYPE_PRIMARY_KIND_U64:  string_append(buffer, SV("u64")); break;
    case TYPE_PRIMARY_KIND_I8:   string_append(buffer, SV("i8")); break;
    case TYPE_PRIMARY_KIND_I16:  string_append(buffer, SV("i16")); break;
    case TYPE_PRIMARY_KIND_I32:  string_append(buffer, SV("i32")); break;
    case TYPE_PRIMARY_KIND_I64:  string_append(buffer, SV("i64")); break;

    default: EXP_UNREACHABLE();
    }
}
