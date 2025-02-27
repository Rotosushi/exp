/**
 * Copyright (C) 2024 Cade Weinberg
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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "intrinsics/align_of.h"
#include "utility/unreachable.h"

u64 align_of(Type *restrict type) {
    assert(type != NULL);

    switch (type->kind) {
    // #NOTE: single byte objects do not
    // have an alignment specified by gcc or
    // clang. I believe this is
    // because we are aligning all other
    // objects, so when we allocate the byte
    // we are guaranteed to be at a location counter
    // that is valid for a single byte.
    // for other objects, the alignment is often equal
    // to their size. quads are 8 bytes, and their
    // alignment is 8. ints are 4 bytes, and their
    // alignment is 4.
    // string literals are align 8 as well.
    case TYPE_KIND_NIL:     return 4;
    case TYPE_KIND_BOOLEAN: return 4;
    case TYPE_KIND_I32:     return 4;
    case TYPE_KIND_TUPLE:   return 4;

    default: EXP_UNREACHABLE();
    }
}
