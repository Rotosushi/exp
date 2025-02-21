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
 * @file targets/x86_64/codegen/intrinsics/get_element_address.c
 */

#include "targets/x86_64/codegen/intrinsics/get_element_address.h"
#include "intrinsics/size_of.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

x86_64_Address x86_64_get_element_address(x86_64_Address base, Type const *type,
                                          u64 index) {
    EXP_ASSERT(type != nullptr);

    x86_64_Address result = base;
    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        // #TODO: This computes the correct size if and only if
        //   there is no padding due to alignment requirements
        //   on any of the fields within the tuple. So we have
        //   to properly account for that here.
        TupleType const *tuple = &type->tuple_type;
        EXP_ASSERT(index < tuple->count);
        for (u64 i = 0; i < index; ++i) {
            u64 element_size = size_of(tuple->types[i]);
            EXP_ASSERT(element_size <= i64_MAX);
            result.offset += (i64)element_size;
        }
        break;
    }

    default: EXP_UNREACHABLE();
    }
    return result;
}
