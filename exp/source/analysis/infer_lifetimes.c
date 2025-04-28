/**
 * Copyright (C) 2025 cade-weinberg
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

#include "analysis/infer_lifetimes.h"
#include "env/symbol_table.h"
#include "imr/instruction.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void
infer_lifetime_operand_A(OperandKind kind, OperandData data, u32 block_index) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local          = data.ssa;
        local->lifetime.start = block_index;
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

static void
infer_lifetime_operand(OperandKind kind, OperandData data, u32 block_index) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        Local *local = data.ssa;
        if (block_index > local->lifetime.end) {
            local->lifetime.end = block_index;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *constant = data.constant;
        if (constant->kind == VALUE_KIND_TUPLE) {
            Tuple const *tuple = &constant->tuple;

            for (u64 i = 0; i < tuple->size; ++i) {
                Operand element = tuple->elements[i];
                infer_lifetime_operand(element.kind, element.data, block_index);
            }
        }
        break;
    }

    default: break;
    }
}

static void infer_lifetime_B(Instruction I, u32 block_index) {
    infer_lifetime_operand(I.B_kind, I.B_data, block_index);
}

static void infer_lifetime_AB(Instruction I, u32 block_index) {
    infer_lifetime_operand_A(I.A_kind, I.A_data, block_index);
    infer_lifetime_operand(I.B_kind, I.B_data, block_index);
}

static void infer_lifetime_ABC(Instruction I, u32 block_index) {
    infer_lifetime_operand_A(I.A_kind, I.A_data, block_index);
    infer_lifetime_operand(I.B_kind, I.B_data, block_index);
    infer_lifetime_operand(I.C_kind, I.C_data, block_index);
}

static void infer_lifetime_function(Function *restrict body) {
    for (u8 i = 0; i < body->arguments.size; ++i) {
        Local *arg          = body->arguments.list[i];
        arg->lifetime.start = 0;
        arg->lifetime.end   = u32_MAX;
    }

    Bytecode *bc = &body->bc;

    for (u32 i = bc->length; i > 0; --i) {
        u32         block_index = i - 1;
        Instruction I           = bc->buffer[block_index];
        switch (I.opcode) {
        case OPCODE_RET: {
            infer_lifetime_B(I, block_index);
            break;
        }

        case OPCODE_CALL: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_DOT: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_LET: {
            infer_lifetime_AB(I, block_index);
            break;
        }

        case OPCODE_NEG: {
            infer_lifetime_AB(I, block_index);
            break;
        }

        case OPCODE_ADD: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_SUB: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_MUL: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_DIV: {
            infer_lifetime_ABC(I, block_index);
            break;
        }

        case OPCODE_MOD: {
            infer_lifetime_ABC(I, block_index);
            break;
        }
        default: EXP_UNREACHABLE();
        }
    }
}

static void infer_lifetime_global(Symbol *restrict element,
                                  Context *restrict context) {
    if (element->kind == SYMBOL_KIND_FUNCTION) {
        Function *body = context_enter_function(context, element->name);
        infer_lifetime_function(body);
        context_leave_function(context);
    }
}

i32 infer_lifetimes(Context *restrict context) {
    exp_assert(context != NULL);
    SymbolTable *table = &context->global_symbol_table;
    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *element = table->elements[index];
        if (element == NULL) { continue; }
        infer_lifetime_global(element, context);
    }
    return 0;
}
