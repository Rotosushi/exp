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
#include "codegen/x86/intrinsics/ptr_kind_of.h"
#include "support/assert.h"
#include "support/unreachable.h"

x86_PtrKind x86_ptr_kind_of_primary(LayoutPrimary const *restrict primary) {
    EXP_ASSERT(primary != NULL);
    // #NOTE: when would we want to return a byte pointer to a larger
    // allocation?
    switch (primary->alignment) {
    case 0: EXP_UNREACHABLE();
    case 1: return X86_BYTE_PTR;
    case 2: return X86_WORD_PTR;
    case 3:
    case 4: return X86_DWORD_PTR;
    case 5:
    case 6:
    case 7:
    case 8: return X86_QWORD_PTR;
    // #TODO:
    // ...
    // case 16: return X86_OWORD_PTR | return X86_XMMWORD_PTR
    // ...
    // case 32: return X86_YMMWORD_PTR
    // ...
    // case 64: return X86_ZMMWORD_PTR
    default: EXP_UNREACHABLE();
    }
}

x86_PtrKind x86_ptr_kind_of_tuple(LayoutTuple const *restrict tuple) {
    EXP_ASSERT(tuple != NULL);
    switch (tuple->primary.alignment) {
    case 0:  EXP_UNREACHABLE();
    case 1:  return X86_BYTE_PTR;
    case 2:  return X86_WORD_PTR;
    case 3:
    case 4:  return X86_DWORD_PTR;
    case 5:
    case 6:
    case 7:
    case 8:  return X86_QWORD_PTR;
    default: EXP_UNREACHABLE();
    }
}

x86_PtrKind x86_ptr_kind_of(Layout const *restrict layout) {
    EXP_ASSERT(layout != NULL);
    switch (layout->kind) {
    case LAYOUT_KIND_PRIMARY:
        return x86_ptr_kind_of_primary(&layout->data.primary);
    case LAYOUT_KIND_TUPLE: return x86_ptr_kind_of_tuple(&layout->data.tuple);

    case LAYOUT_KIND_PADDING:
    default:                  EXP_UNREACHABLE();
    }
}
