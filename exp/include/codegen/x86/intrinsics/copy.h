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
#ifndef EXP_BACKEND_X86_INTRINSICS_H
#define EXP_BACKEND_X86_INTRINSICS_H

#include "codegen/x86/imr/function.h"
#include "env/context.h"

void x86_codegen_copy(x86_Location dst,
                      x86_Location src,
                      Type const  *type,
                      u64          block_index,
                      x86_Function *restrict x86_function,
                      Context *restrict context);

void x86_codegen_copy_value(x86_Location dst,
                            Value const *restrict value,
                            u64 block_index,
                            x86_Function *restrict x86_function,
                            Context *restrict context);

// void x86_codegen_copy_scalar_memory(x86_Location *restrict dst,
//                                     x86_Location *restrict src,
//                                     u64 size,
//                                     u64 Idx,
//                                     x86_Context *restrict context);

// void x86_codegen_copy_composite_memory(x86_Location *restrict dst,
//                                        x86_Location *restrict src,
//                                        Type const *type,
//                                        u64         Idx,
//                                        x86_Context *restrict context);

// void x86_codegen_copy_memory(x86_Location *restrict dst,
//                              x86_Location *restrict src,
//                              Type const *type,
//                              u64         Idx,
//                              x86_Context *restrict context);

// void x86_codegen_copy_allocation_from_memory(x86_Allocation *restrict dst,
//                                              x86_Location *restrict src,
//                                              Type const *restrict type,
//                                              u64 Idx,
//                                              x86_Context *restrict context);

#endif // !EXP_BACKEND_X86_INTRINSICS_H
