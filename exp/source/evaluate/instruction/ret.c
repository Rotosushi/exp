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

#include "evaluate/instruction/ret.h"
#include "evaluate/utility/common.h"
#include "support/assert.h"

bool evaluate_ret(Instruction instruction,
                  Frame      *frame,
                  Context *restrict context) {
    Value const *constant = NULL;
    if (!evaluate_operand_to_constant(&constant,
                                      instruction.B_kind,
                                      instruction.B_data,
                                      frame,
                                      context)) {
        return false;
    }
    exp_assert(constant != NULL);

    // pop the locals associated with the current frame off the stack.
    context_stack_pop_n(context, frame->size);

    // push the return value onto the stack
    context_stack_push(context, constant);

    // return to the caller.
    return true;
}
