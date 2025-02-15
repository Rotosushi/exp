// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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
