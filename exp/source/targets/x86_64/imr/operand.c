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

/**
 * @file targets/x86_64/imr/operand.c
 */

#include "targets/x86_64/imr/operand.h"
#include "env/context.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

static x86_64_Operand operand_create(x86_64_OperandKind kind,
                                     x86_64_OperandData data) {
    x86_64_Operand operand = {.kind = kind, .data = data};
    return operand;
}

x86_64_Operand x86_64_operand_gpr(x86_64_GPR gpr) {
    return operand_create(X86_64_OPERAND_KIND_GPR,
                          (x86_64_OperandData){.gpr = gpr});
}

x86_64_Operand x86_64_operand_address(x86_64_Address address) {
    return operand_create(X86_64_OPERAND_KIND_ADDRESS,
                          (x86_64_OperandData){.address = address});
}

x86_64_Operand x86_64_operand_i32(i32 i32_) {
    return operand_create(X86_64_OPERAND_KIND_I32,
                          (x86_64_OperandData){.i32_ = i32_});
}

x86_64_Operand x86_64_operand_label(u32 label) {
    return operand_create(X86_64_OPERAND_KIND_LABEL,
                          (x86_64_OperandData){.label = label});
}

u8 x86_64_operand_size(x86_64_OperandKind kind, x86_64_OperandData data) {
    switch (kind) {
    case X86_64_OPERAND_KIND_GPR: return x86_64_gpr_size(data.gpr);
    // #TODO: an address (as in a pointer) is always 64 bits, but in the
    //  context of an x86_64 instruction, what is meant is the size of the
    //  target of the address. it isn't very robust, but we use the size of
    //  the base register as a stand-in here. for now.
    case X86_64_OPERAND_KIND_ADDRESS: return x86_64_gpr_size(data.address.base);
    case X86_64_OPERAND_KIND_I32:     return 4;
    // #NOTE: a label is effectively a pointer in assembly, so it's size is
    //  always 8.
    case X86_64_OPERAND_KIND_LABEL: return 8;
    default:                        EXP_UNREACHABLE();
    }
}

void print_x86_64_operand(String *buffer, x86_64_OperandKind kind,
                          x86_64_OperandData data, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    switch (kind) {
    case X86_64_OPERAND_KIND_GPR: print_x86_64_gpr(buffer, data.gpr); break;
    case X86_64_OPERAND_KIND_ADDRESS:
        print_x86_64_address(buffer, data.address);
        break;
    case X86_64_OPERAND_KIND_I32:
        string_append(buffer, SV("%"));
        string_append_i64(buffer, data.i32_);
        break;
    case X86_64_OPERAND_KIND_LABEL:
        StringView view = context_labels_at(context, data.label);
        string_append(buffer, view);
        break;
    default: EXP_UNREACHABLE();
    }
}
