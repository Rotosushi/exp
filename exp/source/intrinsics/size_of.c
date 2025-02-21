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
 * @file intrinsics/size_of.c
 */

#include <assert.h>

#include "intrinsics/size_of.h"
#include "utility/unreachable.h"

u64 size_of(Type const *type) {
    assert(type != NULL);

    switch (type->kind) {
    case TYPE_NIL:  return 1;
    case TYPE_BOOL: return 1;
    case TYPE_I8:   return 1;
    case TYPE_I16:  return 2;
    case TYPE_I32:  return 4;
    case TYPE_I64:  return 8;
    case TYPE_U8:   return 1;
    case TYPE_U16:  return 2;
    case TYPE_U32:  return 4;
    case TYPE_U64:  return 8;

    case TYPE_TUPLE: {
        TupleType const *tuple = &type->tuple_type;
        u64 acc                = 0;
        for (u64 i = 0; i < tuple->count; ++i) {
            acc += size_of(tuple->types[i]);
        }
        return acc;
    }

    default: EXP_UNREACHABLE();
    }
}
