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

#include "codegen/x64/imr/operand.h"
#include "support/unreachable.h"

x64_Operand x64_operand_gpr(x64_GPR gpr) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_GPR, .gpr = gpr};
    return opr;
}

x64_Operand x64_operand_address(x64_Address address) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_ADDRESS, .address = address};
    return opr;
}

x64_Operand x64_operand_location(x64_Location location) {
    switch (location.kind) {
    case LOCATION_GPR: {
        return x64_operand_gpr(location.gpr);
    }

    case LOCATION_ADDRESS: {
        return x64_operand_address(location.address);
    }

    default: EXP_UNREACHABLE();
    }
}

x64_Operand x64_operand_alloc(x64_Allocation *alloc) {
    return x64_operand_location(alloc->location);
}

x64_Operand x64_operand_constant(u32 index) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_CONSTANT, .constant = index};
    return opr;
}

x64_Operand x64_operand_label(u32 index) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_LABEL, .label = index};
    return opr;
}

x64_Operand x64_operand_immediate(i64 value) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_IMMEDIATE, .immediate = value};
    return opr;
}
