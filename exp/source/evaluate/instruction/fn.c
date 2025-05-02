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

#include "evaluate/instruction/fn.h"
#include "support/assert.h"

bool evaluate_fn(Instruction instruction,
                 Frame      *frame,
                 Context *restrict context) {
    exp_assert(frame != NULL);
    exp_assert(context != NULL);
    exp_assert_debug(instruction.opcode == OPCODE_FN);
    exp_assert_debug(instruction.A_kind == OPERAND_KIND_SSA);
    Local *A = function_lookup_local(frame->function, instruction.A_data.ssa);
    exp_assert_debug(A != NULL);
    exp_assert_debug(!string_view_empty(A->name));
    exp_assert_debug(A->type != NULL);
    StringView  name = A->name;
    Type const *type = A->type;

    // The "fn" instruction is always constructed with a pointer
    // to the function constant. So we never have another kind of
    // operand in B. At least, that is how this works as of now.
    exp_assert_debug(instruction.B_kind == OPERAND_KIND_CONSTANT);
    Value const *constant = instruction.B_data.constant;
    exp_assert_debug(constant->kind == VALUE_KIND_FUNCTION);

    // We are defining a
    Symbol *global = context_global_symbol_lookup(context, name);
    exp_assert_debug(global->type == NULL);
    exp_assert_debug(global->value == NULL);
    global->name  = name;
    global->type  = type;
    global->value = constant;
    return true;
}
