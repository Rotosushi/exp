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

#include "evaluate/instruction/call.h"
#include "evaluate/top.h"
#include "evaluate/utility/common.h"
#include "support/assert.h"

bool evaluate_call(Instruction instruction,
                   Frame      *frame,
                   Context *restrict context) {
    exp_assert_debug(instruction.opcode == OPCODE_CALL);
    exp_assert_debug(instruction.A_kind == OPERAND_KIND_SSA);
    Local *A = function_lookup_local(frame->function, instruction.A_data.ssa);
    exp_assert_debug(A->type != NULL);

    Value const *callee_value = NULL;
    if (!evaluate_operand_to_constant(&callee_value,
                                      instruction.B_kind,
                                      instruction.B_data,
                                      frame,
                                      context)) {
        return false;
    }
    exp_assert_debug(callee_value != NULL);
    exp_assert_debug(callee_value->kind == VALUE_KIND_FUNCTION);
    Function const *callee = &callee_value->function;
    // push all arguments onto the stack.
    // construct a new call frame and push it onto the frame stack
    // evaluate the new frame.

    Frame callee_frame_info;
    callee_frame_info.function = &callee_value->function;
    callee_frame_info.index    = 0;
    callee_frame_info.offset   = context_stack_size(context);
    callee_frame_info.size     = 0;
    context_frames_push(context, callee_frame_info);
    Frame *callee_frame = context_frames_top(context);

    Value const *actual_args_value = NULL;
    if (!evaluate_operand_to_constant(&actual_args_value,
                                      instruction.C_kind,
                                      instruction.C_data,
                                      frame,
                                      context)) {
        return false;
    }
    exp_assert_debug(actual_args_value != NULL);
    exp_assert_debug(actual_args_value->kind == VALUE_KIND_TUPLE);
    Tuple const *actual_args = &actual_args_value->tuple;

    for (u32 index = 0; index < actual_args->size; ++index) {
        // each operand of the tuple represents an incoming argument,
        // so we retrieve their values from the current frame.
        Operand      element    = actual_args->elements[index];
        Value const *actual_arg = NULL;
        if (!evaluate_operand_to_constant(
                &actual_arg, element.kind, element.data, frame, context)) {
            return false;
        }
        exp_assert_debug(actual_arg != NULL);

        // we push their values onto the stack relative to the callee frame
        exp_assert_debug(index <= u8_MAX);
        Local *formal_arg = function_lookup_argument(callee, (u8)index);
        context_push_local_value(context, callee_frame, formal_arg, actual_arg);
    }
    // once all arguments have been pushed onto the stack, I think the
    // callee_frame is set up to pass control to.
    // (This simulates updating the "instruction pointer" in assembly.)
    if (!evaluate_top_frame(context)) { return false; }

    // we expect the callee to leave it's result on the stack after evaluation,
    // and due to the order of evaluation, we expect the call result to be in
    // the stack slot associated with the result local of the call instruction.
    exp_assert_debug(!context_stack_empty(context));
    exp_assert_debug(context_stack_peek(context, frame->offset, A->ssa) !=
                     NULL);
    return true;
}
