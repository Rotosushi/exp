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

#include "evaluate/instruction/neg.h"
#include "evaluate/utility/common.h"
#include "support/assert.h"

bool evaluate_neg_constant(Value const **restrict result,
                           Value const *restrict constant,
                           Context *restrict context) {
    // negation only supports signed integer types.
    // in the future this is also supported by real number types
    exp_assert_debug(result != NULL);
    exp_assert_debug(constant != NULL);
    exp_assert_debug(context != NULL);
    switch (constant->kind) {
    case VALUE_KIND_I8:
        *result = context_constant_i8(context, -(constant->i8_));
        break;
    case VALUE_KIND_I16:
        *result = context_constant_i16(context, -(constant->i16_));
        break;
    case VALUE_KIND_I32:
        *result = context_constant_i32(context, -(constant->i32_));
        break;
    case VALUE_KIND_I64:
        *result = context_constant_i64(context, -(constant->i64_));
        break;
    default:
        return context_failure_unsupported_operand_value(
            context, SV("-"), constant);
    }
    return true;
}

bool evaluate_neg(Instruction instruction,
                  Frame      *frame,
                  Context *restrict context) {
    exp_assert_debug(instruction.opcode == OPCODE_NEG);
    exp_assert_debug(instruction.A_kind == OPERAND_KIND_SSA);
    Local *local =
        function_lookup_local(frame->function, instruction.A_data.ssa);
    exp_assert_debug(local->type != NULL);

    // compute the negation operation and place the result in a new constant.
    // pointed to by result.
    Value const *result = NULL;
    switch (instruction.B_kind) {
    case OPERAND_KIND_SSA: {
        Value const *constant =
            context_stack_peek(context, frame->offset, instruction.B_data.ssa);
        if (!evaluate_neg_constant(&result, constant, context)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *constant = instruction.B_data.constant;
        if (!evaluate_neg_constant(&result, constant, context)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_LABEL: {
        StringView   name = constant_string_to_view(instruction.B_data.label);
        Value const *constant = NULL;
        if (!evaluate_label_to_constant(&constant, name, frame, context)) {
            return false;
        }
        exp_assert_debug(constant != NULL);

        // else we found some constant value
        if (!evaluate_neg_constant(&result, constant, context)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_I8:
        result = context_constant_i8(context, -(instruction.B_data.i8_));
        break;
    case OPERAND_KIND_I16:
        result = context_constant_i16(context, -(instruction.B_data.i16_));
        break;
    case OPERAND_KIND_I32:
        result = context_constant_i32(context, -(instruction.B_data.i32_));
        break;
    case OPERAND_KIND_I64:
        result = context_constant_i64(context, -(instruction.B_data.i64_));
        break;

    default:
        return context_failure_unsupported_operand(
            context, SV("-"), operand(instruction.B_kind, instruction.B_data));
    }

    // push the result onto the current stack frame
    context_push_local_value(context, frame, local, result);
    return true;
}
