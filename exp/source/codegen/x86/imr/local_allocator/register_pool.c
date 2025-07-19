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

#include "codegen/x86/imr/local_allocator/register_pool.h"

extern void x86_register_pool_initialize(x86_RegisterPool *restrict pool);
extern bool x86_register_pool_gpr_check(x86_RegisterPool const *restrict pool,
                                        x86_GPR gpr);
extern bool x86_register_pool_gpr_aquire(x86_RegisterPool *restrict pool,
                                         x86_GPR gpr);
extern bool
x86_register_pool_gpr_aquire_next_available(x86_RegisterPool *restrict pool,
                                            u8 *restrict gpr_index);
extern bool x86_register_pool_gpr_release(x86_RegisterPool *restrict pool,
                                          x86_GPR gpr);
