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

#include "codegen/x86/intrinsics/size_of.h"
#include "support/assert.h"
#include "support/unreachable.h"

u64 x86_size_of_primary(TypePrimary const *restrict type) {
    EXP_ASSERT(type != NULL);
    switch (type->kind) {
    case TYPE_PRIMARY_KIND_NIL:  return 1;
    case TYPE_PRIMARY_KIND_BOOL: return 1;
    case TYPE_PRIMARY_KIND_U8:   return 1;
    case TYPE_PRIMARY_KIND_U16:  return 2;
    case TYPE_PRIMARY_KIND_U32:  return 4;
    case TYPE_PRIMARY_KIND_U64:  return 8;
    case TYPE_PRIMARY_KIND_I8:   return 1;
    case TYPE_PRIMARY_KIND_I16:  return 2;
    case TYPE_PRIMARY_KIND_I32:  return 4;
    case TYPE_PRIMARY_KIND_I64:  return 8;
    default:                     EXP_UNREACHABLE();
    }
}

u64 x86_size_of_function() { return 8; }
