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
#include <stddef.h>

#include "codegen/x86/env/context.h"
#include "codegen/x86/imr/allocation.h"
#include "support/allocation.h"
#include "support/assert.h"
#include "support/unreachable.h"

x86_Allocation *x86_allocation_allocate(Local const *restrict local,
                                        Context *restrict context) {
    exp_assert(local != NULL);
    exp_assert(context != NULL);
    x86_Allocation *allocation = callocate(1, sizeof(x86_Allocation));
    allocation->ssa            = local->ssa;
    allocation->alive          = true;
    allocation->name           = local->name;
    allocation->lifetime       = local->lifetime;
    allocation->type           = local->type;
    allocation->layout = x86_context_layout_of_type(context, local->type);
    return allocation;
}

void x86_allocation_deallocate(x86_Allocation *restrict allocation) {
    exp_assert(allocation != NULL);
    deallocate(allocation);
}

bool x86_allocation_alive(x86_Allocation const *restrict allocation,
                          u32 block_index) {
    exp_assert(allocation != NULL);
    if (!allocation->alive) { return false; }
    if (block_index < allocation->lifetime.start) { return false; }
    return block_index <= allocation->lifetime.end;
}

void x86_allocation_expire(x86_Allocation *restrict allocation) {
    exp_assert(allocation != NULL);
    exp_assert(allocation->alive);
    allocation->alive    = false;
    allocation->location = x86_location_expire();
}

u64 x86_allocation_size_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    return x86_layout_size_of(allocation->layout);
}

u64 x86_allocation_align_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    return x86_layout_align_of(allocation->layout);
}

static x86_PtrKind
x86_ptr_kind_for_integral(x86_Layout const *restrict layout) {
    u64 size = x86_layout_size_of(layout);
    switch (size) {
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

static x86_PtrKind x86_ptr_kind_for_scalar(x86_Layout const *restrict layout) {
    u64 size = x86_layout_size_of(layout);
    switch (size) {
    case 0: EXP_UNREACHABLE();
    case 1: return X86_BYTE_PTR;
    case 2: return X86_WORD_PTR;
    case 3:
    case 4: return X86_DWORD_PTR;
    case 5:
    case 6:
    case 7:
    case 8: return X86_QWORD_PTR;
    // ...
    // case 16: return X86_OWORD_PTR | return X86_XMMWORD_PTR
    // ...
    // case 32: return X86_YMMWORD_PTR
    // ...
    // case 64: return X86_ZMMWORD_PTR
    default: EXP_UNREACHABLE();
    }
}

static x86_PtrKind x86_ptr_kind_for_layout(x86_Layout const *restrict layout) {
    switch (layout->kind) {
    case X86_LAYOUT_KIND_SCALAR: return x86_ptr_kind_for_scalar(layout);
    case X86_LAYOUT_KIND_TUPLE:  return X86_QWORD_PTR;
    default:                     EXP_UNREACHABLE();
    }
}

x86_PtrKind
x86_allocation_ptr_kind_of(x86_Allocation const *restrict allocation) {
    exp_assert(allocation != NULL);
    if (type_is_integral(allocation->type)) {
        return x86_ptr_kind_for_integral(allocation->layout);
    }

    return x86_ptr_kind_for_layout(allocation->layout);
}
