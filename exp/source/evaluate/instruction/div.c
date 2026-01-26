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

#include "evaluate/instruction/div.h"
#include "env/context.h"
#include "evaluate/utility/common.h"
#include "support/arithmetic.h"
#include "support/assert.h"
#include "support/constant_string.h"

static bool
div_u8_checked(u8 A, u8 B, u8 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_unsigned_division_by_zero(
            context, context_u16_type(context), A, B);
    }

    *C = A / B;
    return true;
}

static bool
div_u16_checked(u16 A, u16 B, u16 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_unsigned_division_by_zero(
            context, context_u32_type(context), A, B);
    }

    *C = A / B;
    return true;
}

static bool
div_u32_checked(u32 A, u32 B, u32 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_unsigned_division_by_zero(
            context, context_u64_type(context), A, B);
    }

    *C = A / B;
    return true;
}

static bool
div_u64_checked(u64 A, u64 B, u64 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_unsigned_division_by_zero(
            context, context_u64_type(context), A, B);
    }

    *C = A / B;
    return true;
}

static bool
div_i8_checked(i8 A, i8 B, i8 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_signed_division_by_zero(
            context, context_i8_type(context), A, B);
    }

    if (div_i8(A, B, C)) {
        return context_failure_signed_overflow(
            context, SV("/"), context_i8_type(context), A, B);
    }

    return true;
}

static bool
div_i16_checked(i16 A, i16 B, i16 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_signed_division_by_zero(
            context, context_i16_type(context), A, B);
    }

    if (div_i16(A, B, C)) {
        return context_failure_signed_overflow(
            context, SV("/"), context_i16_type(context), A, B);
    }

    return true;
}

static bool
div_i32_checked(i32 A, i32 B, i32 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_signed_division_by_zero(
            context, context_i32_type(context), A, B);
    }

    if (div_i32(A, B, C)) {
        return context_failure_signed_overflow(
            context, SV("/"), context_i32_type(context), A, B);
    }

    return true;
}

static bool
div_i64_checked(i64 A, i64 B, i64 *restrict C, Context *restrict context) {
    if (B == 0) {
        return context_failure_signed_division_by_zero(
            context, context_i64_type(context), A, B);
    }

    if (div_i64(A, B, C)) {
        return context_failure_signed_overflow(
            context, SV("/"), context_i64_type(context), A, B);
    }

    return true;
}

static bool div_value_value(Value const **restrict A,
                            Value const *restrict B,
                            Value const *restrict C,
                            Context *restrict context) {
    // #TODO: Integer Promotion rules
    EXP_ASSERT_ALWAYS(B->kind == C->kind);

    switch (B->kind) {
    case VALUE_KIND_U8: {
        u8 u8_;
        if (!div_u8_checked(B->u8_, C->u8_, &u8_, context)) { return false; }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        u16 u16_;
        if (!div_u16_checked(B->u16_, C->u16_, &u16_, context)) {
            return false;
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        u32 u32_;
        if (!div_u32_checked(B->u32_, C->u32_, &u32_, context)) {
            return false;
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        u64 u64_;
        if (!div_u64_checked(B->u64_, C->u64_, &u64_, context)) {
            return false;
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        i8 i8_;
        if (!div_i8_checked(B->i8_, C->i8_, &i8_, context)) { return false; }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        i16 i16_;
        if (!div_i16_checked(B->i16_, C->i16_, &i16_, context)) {
            return false;
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        i32 i32_;
        if (!div_i32_checked(B->i32_, C->i32_, &i32_, context)) {
            return false;
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        i64 i64_;
        if (!div_i64_checked(B->i64_, C->i64_, &i64_, context)) {
            return false;
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support divition for other types
    default:
        return context_failure_unsupported_operand_value(context, SV("/"), B);
    }

    return true;
}

static bool div_value_operand(Value const **restrict A,
                              Value const *restrict B,
                              Operand C,
                              Context *restrict context) {
    // #TODO: Integer Promotion rules

    switch (B->kind) {
    case VALUE_KIND_U8: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U8);
        u8 u8_;
        if (!div_u8_checked(B->u8_, C.data.u8_, &u8_, context)) {
            return false;
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U16);
        u16 u16_;
        if (!div_u16_checked(B->u16_, C.data.u16_, &u16_, context)) {
            return false;
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U32);
        u32 u32_;
        if (!div_u32_checked(B->u32_, C.data.u32_, &u32_, context)) {
            return false;
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U64);
        u64 u64_;
        if (!div_u64_checked(B->u64_, C.data.u64_, &u64_, context)) {
            return false;
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (!div_i8_checked(B->i8_, C.data.i8_, &i8_, context)) {
            return false;
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (!div_i16_checked(B->i16_, C.data.i16_, &i16_, context)) {
            return false;
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (!div_i32_checked(B->i32_, C.data.i32_, &i32_, context)) {
            return false;
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (!div_i64_checked(B->i64_, C.data.i64_, &i64_, context)) {
            return false;
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support divition for other types
    default:
        return context_failure_unsupported_operand_value(context, SV("/"), B);
    }

    return true;
}

static bool div_operand_value(Value const **restrict A,
                              Operand B,
                              Value const *restrict C,
                              Context *restrict context) {
    // #TODO: Integer Promotion rules

    switch (C->kind) {
    case VALUE_KIND_U8: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U8);
        u8 u8_;
        if (!div_u8_checked(B.data.u8_, C->u8_, &u8_, context)) {
            return false;
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U16);
        u16 u16_;
        if (!div_u16_checked(B.data.u16_, C->u16_, &u16_, context)) {
            return false;
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U32);
        u32 u32_;
        if (!div_u32_checked(B.data.u32_, C->u32_, &u32_, context)) {
            return false;
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U64);
        u64 u64_;
        if (!div_u64_checked(B.data.u64_, C->u64_, &u64_, context)) {
            return false;
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (!div_i8_checked(B.data.i8_, C->i8_, &i8_, context)) {
            return false;
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (!div_i16_checked(B.data.i16_, C->i16_, &i16_, context)) {
            return false;
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (!div_i32_checked(B.data.i32_, C->i32_, &i32_, context)) {
            return false;
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (!div_i64_checked(B.data.i64_, C->i64_, &i64_, context)) {
            return false;
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support divition for other types
    default: return context_failure_unsupported_operand(context, SV("/"), B);
    }

    return true;
}

static bool div_operand_operand(Value const **restrict A,
                                Operand B,
                                Operand C,
                                Frame *restrict frame,
                                Context *restrict context) {
    // #TODO: Integer Promotion Rules
    EXP_ASSERT_ALWAYS(B.kind == C.kind);

    switch (B.kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *B_value =
            context_stack_peek(context, frame->offset, B.data.local);

        Value const *C_value =
            context_stack_peek(context, frame->offset, C.data.local);

        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *B_value = B.data.constant;
        Value const *C_value = C.data.constant;

        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_LABEL: {
        Value const *B_value = NULL;
        if (!evaluate_label_to_constant(&B_value,
                                        constant_string_to_view(B.data.label),
                                        frame,
                                        context)) {
            return false;
        }
        EXP_ASSERT_DEBUG(B_value != NULL);

        Value const *C_value = NULL;
        if (!evaluate_label_to_constant(&C_value,
                                        constant_string_to_view(C.data.label),
                                        frame,
                                        context)) {
            return false;
        }
        EXP_ASSERT_DEBUG(C_value != NULL);

        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_U8: {
        u8 u8_;
        if (!div_u8_checked(B.data.u8_, C.data.u8_, &u8_, context)) {
            return false;
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case OPERAND_KIND_U16: {
        u16 u16_;
        if (!div_u16_checked(B.data.u16_, C.data.u16_, &u16_, context)) {
            return false;
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case OPERAND_KIND_U32: {
        u32 u32_;
        if (!div_u32_checked(B.data.u32_, C.data.u32_, &u32_, context)) {
            return false;
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case OPERAND_KIND_U64: {
        u64 u64_;
        if (!div_u64_checked(B.data.u64_, C.data.u64_, &u64_, context)) {
            return false;
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case OPERAND_KIND_I8: {
        i8 i8_;
        if (!div_i8_checked(B.data.i8_, C.data.i8_, &i8_, context)) {
            return false;
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case OPERAND_KIND_I16: {
        i16 i16_;
        if (!div_i16_checked(B.data.i16_, C.data.i16_, &i16_, context)) {
            return false;
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case OPERAND_KIND_I32: {
        i32 i32_;
        if (!div_i32_checked(B.data.i32_, C.data.i32_, &i32_, context)) {
            return false;
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case OPERAND_KIND_I64: {
        i64 i64_;
        if (!div_i64_checked(B.data.i64_, C.data.i64_, &i64_, context)) {
            return false;
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    default: return context_failure_unsupported_operand(context, SV("/"), B);
    }

    return true;
}

static bool evaluate_div_value(Instruction instruction,
                               Frame *restrict frame,
                               Context *restrict context,
                               Value const *restrict B_value,
                               Value const **restrict A) {
    switch (instruction.C_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *C_value = context_stack_peek(
            context, frame->offset, instruction.C_data.local);
        EXP_ASSERT_DEBUG(C_value != NULL);
        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *C_value = instruction.C_data.constant;
        EXP_ASSERT_DEBUG(C_value != NULL);
        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_LABEL: {
        Value const *C_value = NULL;
        if (!evaluate_label_to_constant(
                &C_value,
                constant_string_to_view(instruction.C_data.label),
                frame,
                context)) {
            return false;
        }
        EXP_ASSERT_DEBUG(C_value != NULL);
        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64:
    case OPERAND_KIND_I8:
    case OPERAND_KIND_I16:
    case OPERAND_KIND_I32:
    case OPERAND_KIND_I64:
        return div_value_operand(
            A,
            B_value,
            operand(instruction.C_kind, instruction.C_data),
            context);

    default:
        return context_failure_unsupported_operand_value(
            context, SV("/"), B_value);
    }
}

static bool evaluate_div_immediate(Instruction instruction,
                                   Frame *restrict frame,
                                   Context *restrict context,
                                   Value const **restrict A) {
    // we know that B is an immediate value
    switch (instruction.C_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *C_value = context_stack_peek(
            context, frame->offset, instruction.C_data.local);

        return div_operand_value(
            A,
            operand(instruction.B_kind, instruction.B_data),
            C_value,
            context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *C_value = instruction.C_data.constant;
        return div_operand_value(
            A,
            operand(instruction.B_kind, instruction.B_data),
            C_value,
            context);
    }

    case OPERAND_KIND_LABEL: {
        Value const *C_value = NULL;
        if (!evaluate_label_to_constant(
                &C_value,
                constant_string_to_view(instruction.C_data.label),
                frame,
                context)) {
            return false;
        }

        return div_operand_value(
            A,
            operand(instruction.B_kind, instruction.B_data),
            C_value,
            context);
    }

    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64:
    case OPERAND_KIND_I8:
    case OPERAND_KIND_I16:
    case OPERAND_KIND_I32:
    case OPERAND_KIND_I64:
        return div_operand_operand(
            A,
            operand(instruction.B_kind, instruction.B_data),
            operand(instruction.C_kind, instruction.C_data),
            frame,
            context);

    default:
        return context_failure_unsupported_operand(
            context, SV("/"), operand(instruction.C_kind, instruction.C_data));
    }
}

bool evaluate_div(Instruction instruction,
                  Frame *restrict frame,
                  Context *restrict context) {
    EXP_ASSERT(frame != NULL);
    EXP_ASSERT(context != NULL);
    EXP_ASSERT_DEBUG(instruction.opcode == OPCODE_DIV);
    EXP_ASSERT_DEBUG(instruction.A_kind == OPERAND_KIND_LOCAL);
    Local *A = function_lookup_local(frame->function, instruction.A_data.local);
    EXP_ASSERT_DEBUG(A != NULL);
    EXP_ASSERT_DEBUG(A->type != NULL);

    Value const *A_value = NULL;
    switch (instruction.B_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *B_value = context_stack_peek(
            context, frame->offset, instruction.B_data.local);

        if (!evaluate_div_value(
                instruction, frame, context, B_value, &A_value)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *B_value = instruction.B_data.constant;

        if (!evaluate_div_value(
                instruction, frame, context, B_value, &A_value)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_LABEL: {
        Value const *B_value = NULL;
        if (!evaluate_label_to_constant(
                &B_value,
                constant_string_to_view(instruction.B_data.label),
                frame,
                context)) {
            return false;
        }

        if (!evaluate_div_value(
                instruction, frame, context, B_value, &A_value)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64:
    case OPERAND_KIND_I8:
    case OPERAND_KIND_I16:
    case OPERAND_KIND_I32:
    case OPERAND_KIND_I64:
        if (!evaluate_div_immediate(instruction, frame, context, &A_value)) {
            return false;
        }
        break;

    default:
        return context_failure_unsupported_operand(
            context, SV("/"), operand(instruction.B_kind, instruction.B_data));
    }

    context_push_local_value(context, frame, A, A_value);
    return true;
}
