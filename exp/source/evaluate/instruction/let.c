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

#include "evaluate/instruction/let.h"
#include "evaluate/utility/common.h"
#include "support/assert.h"

bool evaluate_let(Instruction instruction,
                  Frame      *frame,
                  Context *restrict context) {
    EXP_ASSERT(frame != NULL);
    EXP_ASSERT(context != NULL);
    EXP_ASSERT_DEBUG(instruction.opcode == OPCODE_LET);
    EXP_ASSERT_DEBUG(instruction.A_kind == OPERAND_KIND_LOCAL);
    Local *local =
        function_lookup_local(frame->function, instruction.A_data.local);
    EXP_ASSERT_DEBUG(local != NULL);
    EXP_ASSERT_DEBUG(!string_view_empty(local->name));
    EXP_ASSERT_DEBUG(local->type != NULL);

    // retrieve the value we are going to associate with the local name.
    // note that we expect any instructions which create/update/modify
    // this value to have already occured, by the preceding instructions.
    Value const *constant = NULL;
    if (!evaluate_operand_to_constant(&constant,
                                      instruction.B_kind,
                                      instruction.B_data,
                                      frame,
                                      context)) {
        return false;
    }
    EXP_ASSERT_DEBUG(constant != NULL);

    // Are we defining a global name?
    if (context_at_top_level(context)) {
        Symbol *global = context_global_symbol_lookup(context, local->name);
        EXP_ASSERT_DEBUG(string_view_equal(global->name, local->name));
        EXP_ASSERT_DEBUG(global->type == NULL);
        EXP_ASSERT_DEBUG(global->value == NULL);
        global->name  = local->name;
        global->type  = local->type;
        global->value = constant;
    }
    // else define a local name

    // push the value onto the current stack frame
    context_push_local_value(context, frame, local, constant);
    return true;
}
