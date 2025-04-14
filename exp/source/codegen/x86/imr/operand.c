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

#include "codegen/x86/imr/operand.h"
#include "support/unreachable.h"

x86_Operand x86_operand_gpr(x86_64_GPR gpr) {
    x86_Operand opr = {.kind = X86_OPERAND_KIND_GPR, .gpr = gpr};
    return opr;
}

x86_Operand x86_operand_address(x86_Address address) {
    x86_Operand opr = {.kind = X86_OPERAND_KIND_ADDRESS, .address = address};
    return opr;
}

x86_Operand x86_operand_location(x86_Location location) {
    switch (location.kind) {
    case X86_LOCATION_GPR: {
        return x86_operand_gpr(location.gpr);
    }

    case X86_LOCATION_ADDRESS: {
        return x86_operand_address(location.address);
    }

    default: EXP_UNREACHABLE();
    }
}

x86_Operand x86_operand_alloc(x86_Allocation *alloc) {
    return x86_operand_location(alloc->location);
}

x86_Operand x86_operand_constant(u32 index) {
    x86_Operand opr = {.kind = X86_OPERAND_KIND_CONSTANT, .constant = index};
    return opr;
}

x86_Operand x86_operand_label(u32 index) {
    x86_Operand opr = {.kind = X86_OPERAND_KIND_LABEL, .label = index};
    return opr;
}

x86_Operand x86_operand_immediate(i64 value) {
    x86_Operand opr = {.kind = X86_OPERAND_KIND_IMMEDIATE, .immediate = value};
    return opr;
}
