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
#ifndef EXP_BACKEND_X64_INTRINSICS_H
#define EXP_BACKEND_X64_INTRINSICS_H

#include "backend/x64/context.h"

void x64_codegen_copy_scalar_memory(x64_Address target,
                                    x64_Address source,
                                    u64 size,
                                    u64 block_index,
                                    x64_Context *context);

void x64_codegen_copy_composite_memory(x64_Address target,
                                       x64_Address source,
                                       Type const *type,
                                       u64 block_index,
                                       x64_Context *context);

void x64_codegen_copy_memory(x64_Address target,
                             x64_Address source,
                             Type const *type,
                             u64 block_index,
                             x64_Context *context);

void x64_codegen_copy_allocation_from_memory(x64_Allocation *target,
                                             x64_Address source,
                                             Type const *type,
                                             u64 block_index,
                                             x64_Context *context);

void x64_codegen_copy_allocation(x64_Allocation *target,
                                 x64_Allocation *source,
                                 u64 block_index,
                                 x64_Context *context);

#endif // !EXP_BACKEND_X64_INTRINSICS_H
