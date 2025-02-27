/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
