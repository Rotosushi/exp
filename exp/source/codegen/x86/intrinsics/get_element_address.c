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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "codegen/x86/intrinsics/get_element_address.h"
#include "codegen/x86/intrinsics/size_of.h"
#include "support/unreachable.h"

x86_Location
x86_get_element_address(x86_Location *src, Type const *type, u64 index) {
    x86_Location result = *src;

    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        TupleType const *tuple = &type->tuple;
        assert(index < tuple->length);

        for (u64 i = 0; i < index; ++i) {
            Type const *element_type = tuple->types[i];
            u64         element_size = x86_size_of(element_type);
            assert(element_size <= i32_MAX);
            i32 offset = (i32)element_size;

            result.offset += offset;
        }

        break;
    }

    default: EXP_UNREACHABLE();
    }

    return result;
}
