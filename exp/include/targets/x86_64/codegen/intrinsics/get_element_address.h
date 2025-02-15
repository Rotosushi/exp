// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_TARGETS_X86_64_CODEGEN_INTRINSICS_GET_ELEMENT_ADDRESS_H
#define EXP_TARGETS_X86_64_CODEGEN_INTRINSICS_GET_ELEMENT_ADDRESS_H

#include "imr/type.h"
#include "targets/x86_64/imr/address.h"

x86_64_Address x86_64_get_element_address(x86_64_Address base, Type const *type,
                                          u64 index);

#endif // EXP_TARGETS_X86_64_CODEGEN_INTRINSICS_GET_ELEMENT_ADDRESS_H
