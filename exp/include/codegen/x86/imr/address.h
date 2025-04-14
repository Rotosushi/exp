// Copyright (C) 2025 cade-weinberg
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

#ifndef EXP_BACKEND_X86_ADDRESS_H
#define EXP_BACKEND_X86_ADDRESS_H

#include "codegen/x86/imr/registers.h"

typedef struct x86_Address {
    x86_64_GPR base;
    x86_64_GPR index;
    u8         scale;
    bool       has_index;
    i64        offset;
} x86_Address;

x86_Address x86_address_create(x86_64_GPR base, i64 offset);

x86_Address x86_address_create_indexed(x86_64_GPR base,
                                       x86_64_GPR index,
                                       u8         scale,
                                       i64        offset);

/**
 * @brief compares two addresses for equality
 *
 * @note this does a lexicographic comparison,
 * and we do not use the values stored within the registers
 * (because we cannot) to confirm true equality. Thus this
 * method should be used with caution. if at all.
 */
bool x86_address_equality(x86_Address A, x86_Address B);

#endif // EXP_BACKEND_X86_ADDRESS_H
