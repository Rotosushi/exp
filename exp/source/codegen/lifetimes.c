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

#include "codegen/lifetimes.h"
#include "support/allocation.h"
#include "support/unreachable.h"

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

static void lifetimes_compute_A(OperandKind A_kind,
                                OperandData A_data,
                                u64         block_index,
                                Lifetimes  *lifetimes) {
    switch (A_kind) {
    case OPERAND_KIND_SSA: {
        Lifetime *lifetime = lifetimes_at(lifetimes, A_data.ssa);
        // #NOTE that since operand A declares and defines any SSA local,
        // it is always the first use of that SSA local. And since we
        // only every have one instruction that defines a SSA local, we can
        // unconditionally set the first use to the current block index.
        // This is true if we walk the bytecode forward or backward.
        lifetime->first_use = block_index;
        break;
    }

    // Operand A is always an SSA local, as of right now.
    default: EXP_UNREACHABLE();
    }
}

static void lifetimes_compute_operand(OperandKind kind,
                                      OperandData data,
                                      u64         block_index,
                                      Lifetimes  *lifetimes,
                                      Context    *context) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Lifetime *lifetime = lifetimes_at(lifetimes, data.ssa);
        if (block_index > lifetime->last_use) {
            lifetime->last_use = block_index;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant = context_constants_at(context, data.constant);
        if (constant->kind == VALUE_KIND_TUPLE) {
            Tuple *tuple = &constant->tuple;

            for (u64 i = 0; i < tuple->size; ++i) {
                Operand element = tuple->elements[i];
                lifetimes_compute_operand(element.kind,
                                          element.data,
                                          block_index,
                                          lifetimes,
                                          context);
            }
        }
        break;
    }

    default: break;
    }
}

static void lifetimes_compute_B(Instruction I,
                                u64         block_index,
                                Lifetimes  *lifetimes,
                                Context    *context) {

    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, context);
}

static void lifetimes_compute_AB(Instruction I,
                                 u64         block_index,
                                 Lifetimes  *lifetimes,
                                 Context    *context) {
    lifetimes_compute_A(I.A_kind, I.A_data, block_index, lifetimes);
    lifetimes_compute_operand(
        I.A_kind, I.A_data, block_index, lifetimes, context);
    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, context);
}

static void lifetimes_compute_ABC(Instruction I,
                                  u64         block_index,
                                  Lifetimes  *lifetimes,
                                  Context    *context) {
    lifetimes_compute_A(I.A_kind, I.A_data, block_index, lifetimes);
    lifetimes_compute_operand(
        I.A_kind, I.A_data, block_index, lifetimes, context);
    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, context);
    lifetimes_compute_operand(
        I.C_kind, I.C_data, block_index, lifetimes, context);
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
Lifetimes lifetimes_compute(Function *restrict body,
                            Context *restrict context) {
    Bytecode *bc        = &body->bc;
    Lifetimes lifetimes = lifetimes_create(body->ssa_count);

    for (u64 i = bc->length; i > 0; --i) {
        u64         block_index = i - 1;
        Instruction I           = bc->buffer[block_index];
        switch (I.opcode) {
        case OPCODE_RET: {
            lifetimes_compute_B(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_CALL: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_DOT: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_LOAD: {
            lifetimes_compute_AB(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_NEG: {
            lifetimes_compute_AB(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_ADD: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_SUB: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_MUL: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_DIV: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }

        case OPCODE_MOD: {
            lifetimes_compute_ABC(I, block_index, &lifetimes, context);
            break;
        }
        default: EXP_UNREACHABLE();
        }
    }

    return lifetimes;
}
