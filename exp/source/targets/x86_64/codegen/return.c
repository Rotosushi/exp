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

/**
 * @file targets/x86_64/codegen/return.c
 */

#include "targets/x86_64/codegen/return.h"
#include "intrinsics/type_of.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

/*
 * #NOTE: the return instruction could be refactored to AB type,
 *  where we use A to allocate the return slot. However, this immediately
 *  opens up the fact that multiple return expressions must utilize a
 *  phi node to resolve the assignment. strictly speaking. as of now,
 *  the return slot is implicit to the function itself.
 */

ExpResult x86_64_codegen_return(String *buffer, Instruction *instruction,
                                Function *function, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(instruction != nullptr);
    EXP_ASSERT(function != nullptr);
    EXP_ASSERT(context != nullptr);

    switch (instruction->B_kind) {
    case OPERAND_KIND_SSA: {

        break;
    }

    case OPERAND_KIND_I32: {

        break;
    }

    case OPERAND_KIND_CONSTANT: {

        break;
    }

    case OPERAND_KIND_LABEL: {
        EXP_UNREACHABLE();
        break;
    }

    default: EXP_UNREACHABLE();
    }

    return EXP_SUCCESS;
}
