// Copyright (C) 2025 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_CODEGEN_X86_IMR_REGISTER_POOL_H
#define EXP_CODEGEN_X86_IMR_REGISTER_POOL_H

#include "codegen/x86/imr/registers.h"
#include "support/bitset.h"

typedef struct x86_RegisterPool {
    Bitset gpr_pool;
    // Bitset sse_pool;
} x86_RegisterPool;

#define x86_register_count() x86_gpr_count()

inline void x86_register_pool_initialize(x86_RegisterPool *restrict pool) {
    EXP_ASSERT(pool != NULL);
    bitset_initialize(&pool->gpr_pool);
}

inline bool x86_register_pool_gpr_check(x86_RegisterPool const *restrict pool,
                                        x86_GPR gpr) {
    EXP_ASSERT(pool != NULL);
    return bitset_check(&pool->gpr_pool, x86_gpr_to_index(gpr));
}

inline bool x86_register_pool_gpr_aquire(x86_RegisterPool *restrict pool,
                                         x86_GPR gpr) {
    EXP_ASSERT(pool != NULL);
    u8 index = x86_gpr_to_index(gpr);
    if (bitset_check(&pool->gpr_pool, index)) { return false; }

    bitset_set(&pool->gpr_pool, index);
    return true;
}

inline bool
x86_register_pool_gpr_aquire_next_available(x86_RegisterPool *restrict pool,
                                            u8 *restrict gpr_index) {
    EXP_ASSERT(pool != NULL);
    EXP_ASSERT(gpr_index != NULL);
    for (u8 index = 0; index < x86_gpr_count(); ++index) {
        if (bitset_check(&pool->gpr_pool, index)) { continue; }
        bitset_set(&pool->gpr_pool, index);
        *gpr_index = index;
        return true;
    }

    return false;
}

inline bool x86_register_pool_gpr_release(x86_RegisterPool *restrict pool,
                                          x86_GPR gpr) {
    EXP_ASSERT(pool != NULL);
    u8 index = x86_gpr_to_index(gpr);
    if (!bitset_check(&pool->gpr_pool, index)) { return false; }

    bitset_clear(&pool->gpr_pool, index);
    return true;
}

#endif // !EXP_CODEGEN_X86_IMR_REGISTER_POOL_H
