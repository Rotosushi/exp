// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X86_LOCATION_H
#define EXP_BACKEND_X86_LOCATION_H

#include "codegen/x86/imr/registers.h"
#include "support/string.h"

/*
 * #NOTE: We need to handle floating point registers
 * and the status registers.
 */

/**
 * @brief specifies the size of the data pointed to by the
 * address. In Intel syntax this is used by the memory operand
 * itself, in AT&T this is used to add a suffix to the instruction
 * mnemonic.
 * from GNU as docs (9.16.3.1 AT&T Syntax versus Intel Syntax):
 * "... Thus, Intel syntax
 * `mov al, byte ptr foo` is
 * `movb foo, %al`
 * in AT&T syntax. ..."
 *
 * @note I think we only need a single enum within the Location,
 * otherwise we would
 */
typedef enum x86_PtrKind {
    X86_BYTE_PTR,
    X86_WORD_PTR,
    X86_DWORD_PTR,
    X86_QWORD_PTR,
    // X86_XMMWORD_PTR,
    // X86_YMMWORD_PTR,
    // X86_ZMMWORD_PTR,
    // X86_FWORD_PTR, (48 bit ptr)
    // X86_TBYTE_PTR, (80 bit ptr)
    // X86_OWORD_PTR, (128 bit ptr)
} x86_PtrKind;

typedef struct x86_Location {
    union {
        x86_GPR base;
        x86_GPR gpr;
    };
    x86_GPR  index;
    unsigned scale      : 4;
    bool     has_index  : 1;
    bool     is_address : 1;
    unsigned ptr_kind   : 4;
    i32      offset;
} x86_Location;

x86_Location x86_location_gpr(x86_GPR gpr);
x86_Location
x86_location_address(x86_GPR base, x86_PtrKind ptr_kind, i32 offset);
x86_Location x86_location_address_indexed(
    x86_GPR base, x86_PtrKind ptr_kind, x86_GPR index, u8 scale, i32 offset);

/**
 * @brief compares two locations for equality
 *
 * @note this does a lexicographic comparison,
 * and we do not use the values stored within the registers
 * (because we cannot) to confirm true equality. Thus this
 * method should be used with caution. if at all.
 */
bool x86_location_equality(x86_Location A, x86_Location B);

void print_x86_location(String *restrict buffer, x86_Location location);

#endif // !EXP_BACKEND_X86_LOCATION_H
