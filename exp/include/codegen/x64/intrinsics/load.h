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

#ifndef EXP_BACKEND_X64_INTRINSICS_LOAD_H
#define EXP_BACKEND_X64_INTRINSICS_LOAD_H

#include "codegen/x64/env/context.h"

void x64_codegen_load_address_from_operand(x64_Address *restrict dst,
                                           Operand src,
                                           Type const *restrict type,
                                           u64 Idx,
                                           x64_Context *restrict context);

void x64_codegen_load_gpr_from_operand(x86_64_GPR dst,
                                       Operand    src,
                                       u64        Idx,
                                       x64_Context *restrict context);

void x64_codegen_load_argument_from_operand(x64_Address *restrict dst,
                                            Operand src,
                                            Type const *restrict type,
                                            u64 Idx,
                                            x64_Context *restrict context);

void x64_codegen_load_allocation_from_operand(x64_Allocation *restrict dst,
                                              Operand src,
                                              u64     Idx,
                                              x64_Context *restrict context);

void x64_codegen_load_allocation_from_value(x64_Allocation *restrict dst,
                                            Value *value,
                                            u64    Idx,
                                            x64_Context *restrict context);

#endif // !EXP_BACKEND_X64_INTRINSICS_LOAD_H
