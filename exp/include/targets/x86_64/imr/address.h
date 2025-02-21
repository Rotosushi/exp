// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/instructions/address.h
 */

#ifndef EXP_TARGETS_X86_64_INSTRUCTIONS_ADDRESS_H
#define EXP_TARGETS_X86_64_INSTRUCTIONS_ADDRESS_H

#include "targets/x86_64/imr/registers.h"
#include "utility/string.h"

typedef struct x86_64_Address {
    x86_64_GPR base;
    x86_64_GPR index;
    u8 scale;
    bool has_index;
    i64 offset;
} x86_64_Address;

x86_64_Address x86_64_address_create(x86_64_GPR base, i64 offset);
x86_64_Address x86_64_address_create_indexed(x86_64_GPR base, x86_64_GPR index,
                                             u8 scale, i64 offset);

void print_x86_64_address(String *buffer, x86_64_Address address);

#endif // EXP_TARGETS_X86_64_INSTRUCTIONS_ADDRESS_H
