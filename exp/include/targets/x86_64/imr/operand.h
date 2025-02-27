// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H
#define EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H

#include "targets/x86_64/imr/address.h"
#include "utility/string.h"

typedef enum x86_64_OperandKind : u8 {
    X86_64_OPERAND_KIND_GPR,
    X86_64_OPERAND_KIND_ADDRESS,
    X86_64_OPERAND_KIND_I32,
    X86_64_OPERAND_KIND_LABEL,
} x86_64_OperandKind;

typedef union x86_64_OperandData {
    x86_64_GPR gpr;
    x86_64_Address address;
    // #TODO: replace this with Scalar
    i32 i32_;
    u32 label;
} x86_64_OperandData;

typedef struct x86_64_Operand {
    x86_64_OperandKind kind;
    x86_64_OperandData data;
} x86_64_Operand;

x86_64_Operand x86_64_operand_gpr(x86_64_GPR gpr);
x86_64_Operand x86_64_operand_address(x86_64_Address address);
x86_64_Operand x86_64_operand_i32(i32 i32_);
x86_64_Operand x86_64_operand_label(u32 label);
u8 x86_64_operand_size(x86_64_OperandKind kind, x86_64_OperandData data);

struct Context;
void print_x86_64_operand(String *buffer, x86_64_OperandKind kind,
                          x86_64_OperandData data, struct Context *context);

#endif // EXP_TARGETS_X86_64_INSTRUCTIONS_OPERAND_H
