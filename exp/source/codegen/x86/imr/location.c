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

#include "codegen/x86/imr/location.h"
#include "support/assert.h"
#include "support/unreachable.h"

static StringView x86_ptr_kind_mnemonic(x86_PtrKind ptr_kind) {
    switch (ptr_kind) {
    case X86_BYTE_PTR:  return SV("BYTE PTR");
    case X86_WORD_PTR:  return SV("WORD PTR");
    case X86_DWORD_PTR: return SV("DWORD PTR");
    case X86_QWORD_PTR: return SV("QWORD PTR");
    default:            EXP_UNREACHABLE();
    }
}

x86_Location x86_location_gpr(x86_GPR gpr) {
    return (x86_Location){.gpr = gpr, .is_address = false};
}

x86_Location
x86_location_address(x86_GPR base, x86_PtrKind ptr_kind, i32 offset) {
    return (x86_Location){.base       = base,
                          .has_index  = false,
                          .is_address = true,
                          .ptr_kind   = ptr_kind,
                          .offset     = offset};
}

[[maybe_unused]] static bool valid_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x86_Location x86_location_address_indexed(
    x86_GPR base, x86_PtrKind ptr_kind, x86_GPR index, u8 scale, i32 offset) {
    exp_assert(valid_scale(scale));
    return (x86_Location){
        .base     = base,
        .ptr_kind = ptr_kind,
        .index    = index,
        // #NOTE: This oddity suppresses a conversion warning,
        // Which is garanteed to not be a problem as we only allow values which
        // are representable within 4 bits, checked by "valid_scale" above.
        // But alas, GCC has failed to notice this.
        .scale      = (u8)(scale & 0xF),
        .has_index  = true,
        .is_address = true,
        .offset     = offset};
}

static bool x86_location_address_equality(x86_Location A, x86_Location B) {
    if (A.has_index && B.has_index) {
        return (A.base == B.base) && (A.ptr_kind == B.ptr_kind) &&
               (A.index == B.index) && (A.scale == B.scale) &&
               (A.offset == B.offset);
    } else if (!A.has_index && !B.has_index) {
        return (A.base == B.base) && (A.ptr_kind == B.ptr_kind) &&
               (A.offset == B.offset);
    } else {
        return false;
    }
}

bool x86_location_equality(x86_Location A, x86_Location B) {
    if (A.is_address && B.is_address) {
        return x86_location_address_equality(A, B);
    } else if (!A.is_address && !B.is_address) {
        return A.base == B.base;
    } else {
        return false;
    }
}

// effective address: ptr_kind [base + index * scale + offset]
static void print_x86_address(String *restrict buffer, x86_Location address) {
    string_append(buffer, x86_ptr_kind_mnemonic(address.ptr_kind));
    string_append(buffer, SV(" ["));

    string_append(buffer, x86_gpr_mnemonic(address.base));

    if (address.has_index) {
        exp_assert(valid_scale(address.scale));
        string_append(buffer, SV(" + "));
        string_append(buffer, x86_gpr_mnemonic(address.index));
        string_append(buffer, SV(" * "));
        string_append_u64(buffer, address.scale);
    }

    if (address.offset < 0) {
        string_append(buffer, SV(" - "));
        string_append_i64(buffer, -address.offset);
    } else if (address.offset > 0) {
        string_append(buffer, SV(" + "));
        string_append_i64(buffer, address.offset);
    }

    string_append(buffer, SV("]"));
}

void print_x86_location(String *restrict buffer, x86_Location location) {
    exp_assert(buffer != NULL);
    if (location.is_address) {
        print_x86_address(buffer, location);
        return;
    }

    string_append(buffer, x86_gpr_mnemonic(location.gpr));
}
