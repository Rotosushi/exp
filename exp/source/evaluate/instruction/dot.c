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

#include "evaluate/instruction/dot.h"
#include "evaluate/utility/common.h"
#include "support/assert.h"

bool evaluate_dot(Instruction instruction,
                  Frame      *frame,
                  Context *restrict context) {
    exp_assert_debug(instruction.opcode == OPCODE_DOT);
    exp_assert_debug(instruction.A_kind == OPERAND_KIND_SSA);
    Local *A = function_lookup_local(frame->function, instruction.A_data.ssa);
    exp_assert_debug(A != NULL);
    exp_assert_debug(A->type != NULL);

    // retrieve the tuple from operand B
    Value const *tuple_value = NULL;
    if (!evaluate_operand_to_constant(&tuple_value,
                                      instruction.B_kind,
                                      instruction.B_data,
                                      frame,
                                      context)) {
        return false;
    }
    exp_assert_debug(tuple_value->kind == VALUE_KIND_TUPLE);
    Tuple const *tuple = &tuple_value->tuple;

    exp_assert_debug(
        operand_is_index(operand(instruction.C_kind, instruction.C_data)));
    u64 index =
        operand_as_index(operand(instruction.C_kind, instruction.C_data));
    exp_assert_debug(index <= u32_MAX);
    exp_assert_debug(tuple_index_in_bounds(tuple, (u32)index));

    // subscript the tuple to get the element
    Operand element = tuple_at(tuple, (u32)index);

    // turn the element into a value so it can be placed onto the stack
    // (#NOTE #QUESTION #OPTIMIZATION As an optimization, it would be much more
    //  memory efficient to allow the dot operator to return a reference type to
    //  the element. This allows the backend to avoid generating a copy of the
    //  element for each subscript, this composes well with nested subscripts.
    //  However, how would that work in the evaluator?)
    Value const *result = NULL;
    if (!evaluate_operand_to_constant(
            &result, element.kind, element.data, frame, context)) {
        return false;
    }
    exp_assert_debug(result != NULL);

    // push the result onto the stack.
    context_push_local_value(context, frame, A, result);
    return true;
}
