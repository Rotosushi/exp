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

#include "backend/x64/operand.h"
#include "utility/unreachable.h"

x64_Operand x64_operand_gpr(x64_GPR gpr) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_GPR, .data.gpr = gpr};
    return opr;
}

x64_Operand x64_operand_address(u16 address) {
    x64_Operand opr = {.kind         = X64_OPERAND_KIND_ADDRESS,
                       .data.address = address};
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

x64_Operand x64_operand_constant(u16 index) {
    x64_Operand opr = {.kind          = X64_OPERAND_KIND_CONSTANT,
                       .data.constant = index};
    return opr;
}

x64_Operand x64_operand_immediate(i16 value) {
    x64_Operand opr = {.kind           = X64_OPERAND_KIND_IMMEDIATE,
                       .data.immediate = value};
    return opr;
}

x64_Operand x64_operand_label(u16 index) {
    x64_Operand opr = {.kind = X64_OPERAND_KIND_LABEL, .data.label = index};
    return opr;
}
