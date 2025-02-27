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
#include <assert.h>
#include <stddef.h>

#include "backend/lifetimes.h"
#include "utility/alloc.h"
#include "utility/unreachable.h"

Lifetime lifetime_immortal() {
    Lifetime lifetime = {.first_use = 0, .last_use = u64_MAX};
    return lifetime;
}

Lifetimes lifetimes_create(u64 count) {
    Lifetimes lifetiems = {.count  = count,
                           .buffer = callocate(count, sizeof(Lifetime))};
    return lifetiems;
}

void lifetimes_destroy(Lifetimes *restrict lifetiems) {
    lifetiems->count = 0;
    deallocate(lifetiems->buffer);
    lifetiems->buffer = NULL;
}

Lifetime *lifetimes_at(Lifetimes *restrict lifetiems, u64 ssa) {
    assert(ssa < lifetiems->count);
    return lifetiems->buffer + ssa;
}

static void lifetimes_compute_operand(Operand operand,
                                      u64 block_index,
                                      Lifetimes *lifetimes,
                                      Context *context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: {
        Lifetime *lifetime = lifetimes_at(lifetimes, operand.data.ssa);
        if (block_index > lifetime->last_use) {
            lifetime->last_use = block_index;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant = context_values_at(context, operand.data.constant);
        if (constant->kind == VALUEKIND_TUPLE) {
            Tuple *tuple = &constant->tuple;

            for (u64 i = 0; i < tuple->size; ++i) {
                lifetimes_compute_operand(
                    tuple->elements[i], block_index, lifetimes, context);
            }
        }
        break;
    }

    default: break;
    }
}

// walk the bytecode representing the function body.
// if an instruction assigns a value to a SSA local
// that is the first use of the SSA local.
// The last use is the last instruction which uses
// a SSA local as an operand.
//
// if we walk the bytecode in reverse, then we know
// the last use is the first use we encounter, and
// the first use is the instruction which defines
// the local (has the local in operand A)
Lifetimes lifetimes_compute(FunctionBody *restrict body,
                            Context *restrict context) {
    Bytecode *bc        = &body->bc;
    Lifetimes lifetimes = lifetimes_create(body->ssa_count);

    for (u64 i = bc->length; i > 0; --i) {
        u64 block_index = i - 1;
        Instruction I   = bc->buffer[block_index];
        switch (I.format) {
        case IFMT_B: {
            lifetimes_compute_operand(I.B, block_index, &lifetimes, context);
            break;
        }

        case IFMT_AB: {
            lifetimes_compute_operand(I.A, block_index, &lifetimes, context);
            lifetimes_compute_operand(I.B, block_index, &lifetimes, context);
            break;
        }

        case IFMT_ABC: {
            lifetimes_compute_operand(I.A, block_index, &lifetimes, context);
            lifetimes_compute_operand(I.B, block_index, &lifetimes, context);
            lifetimes_compute_operand(I.C, block_index, &lifetimes, context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }

    return lifetimes;
}
