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
 * @file intrinsics/align_of.c
 */

#include <assert.h>

#include "intrinsics/align_of.h"
#include "utility/minmax.h"
#include "utility/unreachable.h"

u64 align_of(Type const *type) {
    assert(type != NULL);

    switch (type->kind) {
    // #NOTE: single byte objects do not
    //  have an alignment specified by gcc or
    //  clang. I believe this is
    //  because we are aligning all other
    //  objects, so when we allocate the byte
    //  we are guaranteed to be at a location counter
    //  that is valid for a single byte.
    //  for other objects, the alignment is often equal
    //  to their size. quads are 8 bytes, and their
    //  alignment is 8. ints are 4 bytes, and their
    //  alignment is 4. shorts are 2 bytes, and their
    //  alignment is also 2.
    //  string literals are align 8 as well. I assume
    //  this is due to machine word size alignment is
    //  the fastest to access.
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
        // #NOTE: the alignment of a Tuple is the
        //  largest alignment of it's elements.
        u64 result             = 0;
        TupleType const *tuple = &type->tuple_type;
        for (u32 index = 0; index < tuple->count; ++index) {
            Type const *element = tuple->types[index];
            result              = max_u64(result, align_of(element));
        }
        return result;
    }

    default: EXP_UNREACHABLE();
    }
}
