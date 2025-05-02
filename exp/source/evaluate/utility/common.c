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

#include "evaluate/utility/common.h"
#include "support/assert.h"
#include "support/unreachable.h"

bool evaluate_label_to_constant(Value const **restrict result,
                                StringView label,
                                Frame *restrict frame,
                                Context *restrict context) {
    LookupResult lookup = context_lookup_label(context, frame->function, label);
    switch (lookup.kind) {
    case LOOKUP_RESULT_NONE:
        return context_failure_undefined_symbol(context, label);

    case LOOKUP_RESULT_LOCAL:
        *result = context_stack_peek(context, frame->offset, lookup.local->ssa);
        break;

    case LOOKUP_RESULT_GLOBAL:
        exp_assert_debug(lookup.global->value != NULL);
        *result = lookup.global->value;
        break;

    default: EXP_UNREACHABLE();
    }
    return true;
}

bool evaluate_operand_to_constant(Value const **restrict result,
                                  OperandKind kind,
                                  OperandData data,
                                  Frame      *frame,
                                  Context *restrict context) {
    switch (kind) {
    case OPERAND_KIND_SSA: {
        *result = context_stack_peek(context, frame->offset, data.ssa);
        break;
    }

    case OPERAND_KIND_CONSTANT: *result = data.constant; break;

    case OPERAND_KIND_LABEL: {
        StringView label = constant_string_to_view(data.label);
        return evaluate_label_to_constant(result, label, frame, context);
    }

    case OPERAND_KIND_NIL: *result = context_constant_nil(context); break;
    case OPERAND_KIND_BOOL:
        *result = data.bool_ ? context_constant_true(context)
                             : context_constant_false(context);
        break;

    case OPERAND_KIND_U8:
        *result = context_constant_u8(context, data.u8_);
        break;
    case OPERAND_KIND_U16:
        *result = context_constant_u16(context, data.u16_);
        break;
    case OPERAND_KIND_U32:
        *result = context_constant_u32(context, data.u32_);
        break;
    case OPERAND_KIND_U64:
        *result = context_constant_u64(context, data.u64_);
        break;
    case OPERAND_KIND_I8:
        *result = context_constant_i8(context, data.i8_);
        break;
    case OPERAND_KIND_I16:
        *result = context_constant_i16(context, data.i16_);
        break;
    case OPERAND_KIND_I32:
        *result = context_constant_i32(context, data.i32_);
        break;
    case OPERAND_KIND_I64:
        *result = context_constant_i64(context, data.i64_);
        break;

    default: EXP_UNREACHABLE();
    }
    return true;
}
