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

#include "codegen/x64/intrinsics/get_element_address.h"
#include "intrinsics/size_of.h"
#include "support/unreachable.h"

x64_Address
x64_get_element_address(x64_Address *src, Type const *type, u64 index) {

    x64_Address result = *src;

    switch (type->kind) {
    case TYPE_KIND_TUPLE: {
        TupleType const *tuple = &type->tuple_type;
        assert(index < tuple->size);

        for (u64 i = 0; i < index; ++i) {
            Type const *element_type = tuple->types[i];
            u64         element_size = size_of(element_type);
            assert(element_size <= i64_MAX);
            i64 offset = (i64)element_size;

            result.offset += offset;
        }

        break;
    }

    default: EXP_UNREACHABLE();
    }

    return result;
}
