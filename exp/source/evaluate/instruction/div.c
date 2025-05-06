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
#include "evaluate/utility/common.h"
#include "support/assert.h"
#include "support/constant_string.h"

static bool div_i8_overflow(i8 A, i8 B, i8 *C) {
    if (B == 0) { return true; }
    if (A == i8_MIN && B == -1) { return true; }
    *C = A / B;
    return false;
}

static bool div_i16_overflow(i16 A, i16 B, i16 *C) {
    if (B == 0) { return true; }
    if (A == i16_MIN && B == -1) { return true; }
    *C = A / B;
    return false;
}

static bool div_i32_overflow(i32 A, i32 B, i32 *C) {
    if (B == 0) { return true; }
    if (A == i32_MIN && B == -1) { return true; }
    *C = A / B;
    return false;
}

static bool div_i64_overflow(i64 A, i64 B, i64 *C) {
    if (B == 0) { return true; }
    if (A == i64_MIN && B == -1) { return true; }
    *C = A / B;
    return false;
}

static bool div_value_value(Value const **restrict A,
                            Value const *restrict B,
                            Value const *restrict C,
                            Context *restrict context) {
    // #TODO: Integer Promotion rules
    exp_assert_always(B->kind == C->kind);

    switch (B->kind) {
    case VALUE_KIND_U8: {
        *A = context_constant_u8(context, (B->u8_ / C->u8_));
        break;
    }

    case VALUE_KIND_U16: {
        *A = context_constant_u16(context, (B->u16_ / C->u16_));
        break;
    }

    case VALUE_KIND_U32: {
        *A = context_constant_u32(context, (B->u32_ / C->u32_));
        break;
    }

    case VALUE_KIND_U64: {
        *A = context_constant_u64(context, (B->u64_ / C->u64_));
        break;
    }

    case VALUE_KIND_I8: {
        i8 i8_;
        if (div_i8_overflow(B->i8_, C->i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i8_type(context), B->i8_, C->i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        i16 i16_;
        if (div_i16_overflow(B->i16_, C->i16_, &i16_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i16_type(context), B->i16_, C->i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        i32 i32_;
        if (div_i32_overflow(B->i32_, C->i32_, &i32_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i32_type(context), B->i32_, C->i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        i64 i64_;
        if (div_i64_overflow(B->i64_, C->i64_, &i64_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i64_type(context), B->i64_, C->i64_);
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
        exp_assert_always(C.kind == OPERAND_KIND_U8);
        *A = context_constant_u8(context, B->u8_ / C.data.u8_);
        break;
    }

    case VALUE_KIND_U16: {
        exp_assert_always(C.kind == OPERAND_KIND_U16);
        *A = context_constant_u16(context, B->u16_ / C.data.u16_);
        break;
    }

    case VALUE_KIND_U32: {
        exp_assert_always(C.kind == OPERAND_KIND_U32);
        *A = context_constant_u32(context, B->u32_ / C.data.u32_);
        break;
    }

    case VALUE_KIND_U64: {
        exp_assert_always(C.kind == OPERAND_KIND_U64);
        *A = context_constant_u64(context, B->u64_ / C.data.u64_);
        break;
    }

    case VALUE_KIND_I8: {
        exp_assert_always(C.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (div_i8_overflow(B->i8_, C.data.i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i8_type(context), B->i8_, C.data.i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        exp_assert_always(C.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (div_i16_overflow(B->i16_, C.data.i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i16_type(context),
                                                   B->i16_,
                                                   C.data.i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        exp_assert_always(C.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (div_i32_overflow(B->i32_, C.data.i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i32_type(context),
                                                   B->i32_,
                                                   C.data.i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        exp_assert_always(C.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (div_i64_overflow(B->i64_, C.data.i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i64_type(context),
                                                   B->i64_,
                                                   C.data.i64_);
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
        exp_assert_always(B.kind == OPERAND_KIND_U8);
        *A = context_constant_u8(context, B.data.u8_ / C->u8_);
        break;
    }

    case VALUE_KIND_U16: {
        exp_assert_always(B.kind == OPERAND_KIND_U16);
        *A = context_constant_u16(context, B.data.u16_ / C->u16_);
        break;
    }

    case VALUE_KIND_U32: {
        exp_assert_always(B.kind == OPERAND_KIND_U32);
        *A = context_constant_u32(context, B.data.u32_ / C->u32_);
        break;
    }

    case VALUE_KIND_U64: {
        exp_assert_always(B.kind == OPERAND_KIND_U64);
        *A = context_constant_u64(context, B.data.u64_ / C->u64_);
        break;
    }

    case VALUE_KIND_I8: {
        exp_assert_always(B.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (div_i8_overflow(B.data.i8_, C->i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("/"), context_i8_type(context), B.data.i8_, C->i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        exp_assert_always(B.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (div_i16_overflow(B.data.i16_, C->i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i16_type(context),
                                                   B.data.i16_,
                                                   C->i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        exp_assert_always(B.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (div_i32_overflow(B.data.i32_, C->i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i32_type(context),
                                                   B.data.i32_,
                                                   C->i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        exp_assert_always(B.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (div_i64_overflow(B.data.i64_, C->i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i64_type(context),
                                                   B.data.i64_,
                                                   C->i64_);
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
    exp_assert_always(B.kind == C.kind);

    switch (B.kind) {
    case OPERAND_KIND_SSA: {
        Value const *B_value =
            context_stack_peek(context, frame->offset, B.data.ssa);

        Value const *C_value =
            context_stack_peek(context, frame->offset, C.data.ssa);

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
        exp_assert_debug(B_value != NULL);

        Value const *C_value = NULL;
        if (!evaluate_label_to_constant(&C_value,
                                        constant_string_to_view(C.data.label),
                                        frame,
                                        context)) {
            return false;
        }
        exp_assert_debug(C_value != NULL);

        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_U8: {
        *A = context_constant_u8(context, B.data.u8_ / C.data.u8_);
        break;
    }

    case OPERAND_KIND_U16: {
        *A = context_constant_u16(context, B.data.u16_ / C.data.u16_);
        break;
    }

    case OPERAND_KIND_U32: {
        *A = context_constant_u32(context, B.data.u32_ / C.data.u32_);
        break;
    }

    case OPERAND_KIND_U64: {
        *A = context_constant_u64(context, B.data.u64_ / C.data.u64_);
        break;
    }

    case OPERAND_KIND_I8: {
        i8 i8_;
        if (div_i8_overflow(B.data.i8_, C.data.i8_, &i8_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i8_type(context),
                                                   B.data.i8_,
                                                   C.data.i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case OPERAND_KIND_I16: {
        i16 i16_;
        if (div_i16_overflow(B.data.i16_, C.data.i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i16_type(context),
                                                   B.data.i16_,
                                                   C.data.i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case OPERAND_KIND_I32: {
        i32 i32_;
        if (div_i32_overflow(B.data.i32_, C.data.i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i32_type(context),
                                                   B.data.i32_,
                                                   C.data.i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case OPERAND_KIND_I64: {
        i64 i64_;
        if (div_i64_overflow(B.data.i64_, C.data.i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("/"),
                                                   context_i64_type(context),
                                                   B.data.i64_,
                                                   C.data.i64_);
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
    case OPERAND_KIND_SSA: {
        Value const *C_value =
            context_stack_peek(context, frame->offset, instruction.C_data.ssa);
        exp_assert_debug(C_value != NULL);
        return div_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *C_value = instruction.C_data.constant;
        exp_assert_debug(C_value != NULL);
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
        exp_assert_debug(C_value != NULL);
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
    case OPERAND_KIND_SSA: {
        Value const *C_value =
            context_stack_peek(context, frame->offset, instruction.C_data.ssa);

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
    exp_assert(frame != NULL);
    exp_assert(context != NULL);
    exp_assert_debug(instruction.opcode == OPCODE_DIV);
    exp_assert_debug(instruction.A_kind == OPERAND_KIND_SSA);
    Local *A = function_lookup_local(frame->function, instruction.A_data.ssa);
    exp_assert_debug(A != NULL);
    exp_assert_debug(A->type != NULL);

    Value const *A_value = NULL;
    switch (instruction.B_kind) {
    case OPERAND_KIND_SSA: {
        Value const *B_value =
            context_stack_peek(context, frame->offset, instruction.B_data.ssa);

        return evaluate_div_value(
            instruction, frame, context, B_value, &A_value);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *B_value = instruction.B_data.constant;

        return evaluate_div_value(
            instruction, frame, context, B_value, &A_value);
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

        return evaluate_div_value(
            instruction, frame, context, B_value, &A_value);
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
