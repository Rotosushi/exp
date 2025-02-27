/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
    //  case TYPE_KIND_NIL:     return 4;
    //  case TYPE_KIND_BOOLEAN: return 4;
    case TYPE_KIND_I32:   return 4;
    case TYPE_KIND_TUPLE: {
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
