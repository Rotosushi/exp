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

#include "intrinsics/size_of.h"
#include "support/unreachable.h"

u64 size_of(Type *restrict type) {
    assert(type != NULL);

    switch (type->kind) {
    case TYPE_KIND_NIL:     return 1;
    case TYPE_KIND_BOOLEAN: return 1;
    case TYPE_KIND_I64:     return 8;

    case TYPE_KIND_TUPLE: {
        TupleType *tuple = &type->tuple_type;
        u64 acc          = 0;
        for (u64 i = 0; i < tuple->size; ++i) {
            acc += size_of(tuple->types[i]);
        }
        return acc;
    }

    default: EXP_UNREACHABLE();
    }
}
