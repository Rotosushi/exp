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
#include "backend/x64/context.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
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

static void lifetimes_grow(Lifetimes *lifetimes) {
    assert(lifetimes != nullptr);
    Growth64 g        = array_growth_u64(lifetimes->capacity, sizeof(Lifetime));
    lifetimes->buffer = reallocate(lifetimes->buffer, g.alloc_size);
    lifetimes->capacity = g.new_capacity;
}

void lifetimes_update(Lifetimes *lifetimes, u64 ssa, Lifetime lifetime) {
    assert(lifetimes != nullptr);
    while (lifetimes->capacity <= ssa) {
        lifetimes_grow(lifetimes);
    }
    lifetimes->buffer[ssa] = lifetime;
    if (lifetimes->count < ssa) { lifetimes->count = ssa; }
}

Lifetime *lifetimes_at(Lifetimes *restrict lifetiems, u64 ssa) {
    assert(ssa < lifetiems->count);
    return lifetiems->buffer + ssa;
}

static void lifetimes_compute_operand(OperandKind kind,
                                      OperandData data,
                                      u64 block_index,
                                      Lifetimes *lifetimes,
                                      x64_Context *x64_context) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Lifetime *lifetime = lifetimes_at(lifetimes, data.ssa);
        if (block_index > lifetime->last_use) {
            lifetime->last_use = block_index;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value *constant = x64_context_constants_at(x64_context, data.constant);
        if (constant->kind == VALUE_KIND_TUPLE) {
            Tuple *tuple = &constant->tuple;

            for (u64 i = 0; i < tuple->size; ++i) {
                Operand element = tuple->elements[i];
                lifetimes_compute_operand(element.kind,
                                          element.data,
                                          block_index,
                                          lifetimes,
                                          x64_context);
            }
        }
        break;
    }

    // #NOTE: a lifetime only makes sense w.r.t. local variables.
    //  %ssa can only be present within Operands directly, or
    //  within a Tuple. becuase Tuples are composed of Operands.
    //  thus nothing needs to be done for any other kind of Operand.
    // #TODO: if we allow labels to be how we address local declarations.
    //  then we will need to handle that here. Because lifetimes will need
    //  to be associated with labels just as they are with %ssa; as of now
    //  named locals are also given %ssa assignments, and they are treat the
    //  same. which sames on implementation complexity.
    default: break;
    }
}

static void lifetimes_compute_B(Instruction I,
                                u64 block_index,
                                Lifetimes *lifetimes,
                                x64_Context *x64_context) {

    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, x64_context);
}

static void lifetimes_compute_AB(Instruction I,
                                 u64 block_index,
                                 Lifetimes *lifetimes,
                                 x64_Context *x64_context) {
    lifetimes_compute_operand(
        I.A_kind, I.A_data, block_index, lifetimes, x64_context);
    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, x64_context);
}

static void lifetimes_compute_ABC(Instruction I,
                                  u64 block_index,
                                  Lifetimes *lifetimes,
                                  x64_Context *x64_context) {
    lifetimes_compute_operand(
        I.A_kind, I.A_data, block_index, lifetimes, x64_context);
    lifetimes_compute_operand(
        I.B_kind, I.B_data, block_index, lifetimes, x64_context);
    lifetimes_compute_operand(
        I.C_kind, I.C_data, block_index, lifetimes, x64_context);
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
void lifetimes_initialize(Lifetimes *lifetimes,
                          FunctionBody *restrict body,
                          x64_Context *restrict x64_context) {
    lifetimes->count    = body->ssa_count;
    lifetimes->capacity = body->ssa_count;
    lifetimes->buffer   = callocate(body->ssa_count, sizeof(Lifetime));

    Bytecode *bc = &body->bc;

    for (u64 i = bc->length; i > 0; --i) {
        u64 block_index = i - 1;
        Instruction I   = bc->buffer[block_index];
        switch (I.opcode) {
        case OPCODE_RETURN: {
            lifetimes_compute_B(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_CALL: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_DOT: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_LOAD: {
            lifetimes_compute_AB(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_NEGATE: {
            lifetimes_compute_AB(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_ADD: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_SUBTRACT: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_MULTIPLY: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_DIVIDE: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        case OPCODE_MODULUS: {
            lifetimes_compute_ABC(I, block_index, lifetimes, x64_context);
            break;
        }

        default: EXP_UNREACHABLE();
        }
    }
}
