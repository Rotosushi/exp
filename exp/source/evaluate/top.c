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

#include "evaluate/top.h"
#include "evaluate/instruction/add.h"
#include "evaluate/instruction/call.h"
#include "evaluate/instruction/div.h"
#include "evaluate/instruction/dot.h"
#include "evaluate/instruction/let.h"
#include "evaluate/instruction/mod.h"
#include "evaluate/instruction/mul.h"
#include "evaluate/instruction/neg.h"
#include "evaluate/instruction/ret.h"
#include "evaluate/instruction/sub.h"
#include "support/unreachable.h"

static bool evaluate_instruction(Instruction instruction,
                                 Frame      *frame,
                                 Context *restrict context) {
    switch (instruction.opcode) {
    case OPCODE_RET:  return evaluate_ret(instruction, frame, context);
    case OPCODE_CALL: return evaluate_call(instruction, frame, context);
    case OPCODE_LET:  return evaluate_let(instruction, frame, context);
    case OPCODE_NEG:  return evaluate_neg(instruction, frame, context);
    case OPCODE_DOT:  return evaluate_dot(instruction, frame, context);
    case OPCODE_ADD:  return evaluate_add(instruction, frame, context);
    case OPCODE_SUB:  return evaluate_sub(instruction, frame, context);
    case OPCODE_MUL:  return evaluate_mul(instruction, frame, context);
    case OPCODE_DIV:  return evaluate_div(instruction, frame, context);
    case OPCODE_MOD:  return evaluate_mod(instruction, frame, context);
    default:          EXP_UNREACHABLE();
    }
}

bool evaluate_top_frame(Context *restrict context) {
    bool            result = true;
    Frame          *frame  = context_frames_top(context);
    Bytecode const *body   = &frame->function->body;
    for (; frame->index < body->length; ++frame->index) {
        if (!evaluate_instruction(body->buffer[frame->index], frame, context)) {
            result = false;
            break;
        }
    }
    context_frames_pop(context);
    return result;
}
