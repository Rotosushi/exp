
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

#include "evaluate/instruction/sub.h"
#include "evaluate/utility/common.h"
#include "support/arithmetic.h"
#include "support/assert.h"
#include "support/constant_string.h"

static bool sub_value_value(Value const **restrict A,
                            Value const *restrict B,
                            Value const *restrict C,
                            Context *restrict context) {
    // #TODO: Integer Promotion rules
    EXP_ASSERT_ALWAYS(B->kind == C->kind);

    switch (B->kind) {
    case VALUE_KIND_U8: {
        u8 u8_;
        if (sub_u8(B->u8_, C->u8_, &u8_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u8(context), B->u8_, C->u8_);
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        u16 u16_;
        if (sub_u16(B->u16_, C->u16_, &u16_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u16(context), B->u16_, C->u16_);
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        u32 u32_;
        if (sub_u32(B->u32_, C->u32_, &u32_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u32(context), B->u32_, C->u32_);
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        u64 u64_;
        if (sub_u64(B->u64_, C->u64_, &u64_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u64(context), B->u64_, C->u64_);
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        i8 i8_;
        if (sub_i8(B->i8_, C->i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i8(context), B->i8_, C->i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        i16 i16_;
        if (sub_i16(B->i16_, C->i16_, &i16_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i16(context), B->i16_, C->i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        i32 i32_;
        if (sub_i32(B->i32_, C->i32_, &i32_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i32(context), B->i32_, C->i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        i64 i64_;
        if (sub_i64(B->i64_, C->i64_, &i64_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i64(context), B->i64_, C->i64_);
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support subition for other types
    default:
        return context_failure_unsupported_operand_value(context, SV("-"), B);
    }

    return true;
}

static bool sub_value_operand(Value const **restrict A,
                              Value const *restrict B,
                              Operand C,
                              Context *restrict context) {
    // #TODO: Integer Promotion rules

    switch (B->kind) {
    case VALUE_KIND_U8: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U8);
        u8 u8_;
        if (sub_u8(B->u8_, C.data.u8_, &u8_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u8(context), B->u8_, C.data.u8_);
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U16);
        u16 u16_;
        if (sub_u16(B->u16_, C.data.u16_, &u16_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u16(context),
                                                     B->u16_,
                                                     C.data.u16_);
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U32);
        u32 u32_;
        if (sub_u32(B->u32_, C.data.u32_, &u32_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u32(context),
                                                     B->u32_,
                                                     C.data.u32_);
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_U64);
        u64 u64_;
        if (sub_u64(B->u64_, C.data.u64_, &u64_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u64(context),
                                                     B->u64_,
                                                     C.data.u64_);
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (sub_i8(B->i8_, C.data.i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i8(context), B->i8_, C.data.i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (sub_i16(B->i16_, C.data.i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i16(context),
                                                   B->i16_,
                                                   C.data.i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (sub_i32(B->i32_, C.data.i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i32(context),
                                                   B->i32_,
                                                   C.data.i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        EXP_ASSERT_ALWAYS(C.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (sub_i64(B->i64_, C.data.i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i64(context),
                                                   B->i64_,
                                                   C.data.i64_);
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support subition for other types
    default:
        return context_failure_unsupported_operand_value(context, SV("-"), B);
    }

    return true;
}

static bool sub_operand_value(Value const **restrict A,
                              Operand B,
                              Value const *restrict C,
                              Context *restrict context) {
    // #TODO: Integer Promotion rules

    switch (C->kind) {
    case VALUE_KIND_U8: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U8);
        u8 u8_;
        if (sub_u8(B.data.u8_, C->u8_, &u8_)) {
            return context_failure_unsigned_overflow(
                context, SV("-"), context_type_u8(context), B.data.u8_, C->u8_);
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case VALUE_KIND_U16: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U16);
        u16 u16_;
        if (sub_u16(B.data.u16_, C->u16_, &u16_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u16(context),
                                                     B.data.u16_,
                                                     C->u16_);
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case VALUE_KIND_U32: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U32);
        u32 u32_;
        if (sub_u32(B.data.u32_, C->u32_, &u32_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u32(context),
                                                     B.data.u32_,
                                                     C->u32_);
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case VALUE_KIND_U64: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_U64);
        u64 u64_;
        if (sub_u64(B.data.u64_, C->u64_, &u64_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u64(context),
                                                     B.data.u64_,
                                                     C->u64_);
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case VALUE_KIND_I8: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I8);
        i8 i8_;
        if (sub_i8(B.data.i8_, C->i8_, &i8_)) {
            return context_failure_signed_overflow(
                context, SV("-"), context_type_i8(context), B.data.i8_, C->i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case VALUE_KIND_I16: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I16);
        i16 i16_;
        if (sub_i16(B.data.i16_, C->i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i16(context),
                                                   B.data.i16_,
                                                   C->i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case VALUE_KIND_I32: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I32);
        i32 i32_;
        if (sub_i32(B.data.i32_, C->i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i32(context),
                                                   B.data.i32_,
                                                   C->i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case VALUE_KIND_I64: {
        EXP_ASSERT_ALWAYS(B.kind == OPERAND_KIND_I64);
        i64 i64_;
        if (sub_i64(B.data.i64_, C->i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i64(context),
                                                   B.data.i64_,
                                                   C->i64_);
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    // we don't support subition for other types
    default: return context_failure_unsupported_operand(context, SV("-"), B);
    }

    return true;
}

static bool sub_operand_operand(Value const **restrict A,
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

        return sub_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *B_value = B.data.constant;
        Value const *C_value = C.data.constant;

        return sub_value_value(A, B_value, C_value, context);
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

        return sub_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_U8: {
        u8 u8_;
        if (sub_u8(B.data.u8_, C.data.u8_, &u8_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u8(context),
                                                     B.data.u8_,
                                                     C.data.u8_);
        }
        *A = context_constant_u8(context, u8_);
        break;
    }

    case OPERAND_KIND_U16: {
        u16 u16_;
        if (sub_u16(B.data.u16_, C.data.u16_, &u16_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u16(context),
                                                     B.data.u16_,
                                                     C.data.u16_);
        }
        *A = context_constant_u16(context, u16_);
        break;
    }

    case OPERAND_KIND_U32: {
        u32 u32_;
        if (sub_u32(B.data.u32_, C.data.u32_, &u32_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u32(context),
                                                     B.data.u32_,
                                                     C.data.u32_);
        }
        *A = context_constant_u32(context, u32_);
        break;
    }

    case OPERAND_KIND_U64: {
        u64 u64_;
        if (sub_u64(B.data.u64_, C.data.u64_, &u64_)) {
            return context_failure_unsigned_overflow(context,
                                                     SV("-"),
                                                     context_type_u64(context),
                                                     B.data.u64_,
                                                     C.data.u64_);
        }
        *A = context_constant_u64(context, u64_);
        break;
    }

    case OPERAND_KIND_I8: {
        i8 i8_;
        if (sub_i8(B.data.i8_, C.data.i8_, &i8_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i8(context),
                                                   B.data.i8_,
                                                   C.data.i8_);
        }
        *A = context_constant_i8(context, i8_);
        break;
    }

    case OPERAND_KIND_I16: {
        i16 i16_;
        if (sub_i16(B.data.i16_, C.data.i16_, &i16_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i16(context),
                                                   B.data.i16_,
                                                   C.data.i16_);
        }
        *A = context_constant_i16(context, i16_);
        break;
    }

    case OPERAND_KIND_I32: {
        i32 i32_;
        if (sub_i32(B.data.i32_, C.data.i32_, &i32_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i32(context),
                                                   B.data.i32_,
                                                   C.data.i32_);
        }
        *A = context_constant_i32(context, i32_);
        break;
    }

    case OPERAND_KIND_I64: {
        i64 i64_;
        if (sub_i64(B.data.i64_, C.data.i64_, &i64_)) {
            return context_failure_signed_overflow(context,
                                                   SV("-"),
                                                   context_type_i64(context),
                                                   B.data.i64_,
                                                   C.data.i64_);
        }
        *A = context_constant_i64(context, i64_);
        break;
    }

    default: return context_failure_unsupported_operand(context, SV("-"), B);
    }

    return true;
}

static bool evaluate_sub_value(Instruction instruction,
                               Frame *restrict frame,
                               Context *restrict context,
                               Value const *restrict B_value,
                               Value const **restrict A) {
    switch (instruction.C_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *C_value = context_stack_peek(
            context, frame->offset, instruction.C_data.local);
        EXP_ASSERT_DEBUG(C_value != NULL);
        return sub_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *C_value = instruction.C_data.constant;
        EXP_ASSERT_DEBUG(C_value != NULL);
        return sub_value_value(A, B_value, C_value, context);
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
        return sub_value_value(A, B_value, C_value, context);
    }

    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64:
    case OPERAND_KIND_I8:
    case OPERAND_KIND_I16:
    case OPERAND_KIND_I32:
    case OPERAND_KIND_I64:
        return sub_value_operand(
            A,
            B_value,
            operand(instruction.C_kind, instruction.C_data),
            context);

    default:
        return context_failure_unsupported_operand_value(
            context, SV("-"), B_value);
    }
}

static bool evaluate_sub_immediate(Instruction instruction,
                                   Frame *restrict frame,
                                   Context *restrict context,
                                   Value const **restrict A) {
    // we know that B is an immediate value
    switch (instruction.C_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *C_value = context_stack_peek(
            context, frame->offset, instruction.C_data.local);

        return sub_operand_value(
            A,
            operand(instruction.B_kind, instruction.B_data),
            C_value,
            context);
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *C_value = instruction.C_data.constant;
        return sub_operand_value(
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

        return sub_operand_value(
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
        return sub_operand_operand(
            A,
            operand(instruction.B_kind, instruction.B_data),
            operand(instruction.C_kind, instruction.C_data),
            frame,
            context);

    default:
        return context_failure_unsupported_operand(
            context, SV("-"), operand(instruction.C_kind, instruction.C_data));
    }
}

bool evaluate_sub(Instruction instruction,
                  Frame *restrict frame,
                  Context *restrict context) {
    EXP_ASSERT(frame != NULL);
    EXP_ASSERT(context != NULL);
    EXP_ASSERT_DEBUG(instruction.opcode == OPCODE_SUB);
    EXP_ASSERT_DEBUG(instruction.A_kind == OPERAND_KIND_LOCAL);
    Local *A = function_lookup_local(frame->function, instruction.A_data.local);
    EXP_ASSERT_DEBUG(A != NULL);
    EXP_ASSERT_DEBUG(A->type != NULL);

    Value const *A_value = NULL;
    switch (instruction.B_kind) {
    case OPERAND_KIND_LOCAL: {
        Value const *B_value = context_stack_peek(
            context, frame->offset, instruction.B_data.local);

        if (!evaluate_sub_value(
                instruction, frame, context, B_value, &A_value)) {
            return false;
        }
        break;
    }

    case OPERAND_KIND_CONSTANT: {
        Value const *B_value = instruction.B_data.constant;

        if (!evaluate_sub_value(
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

        if (!evaluate_sub_value(
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
        if (!evaluate_sub_immediate(instruction, frame, context, &A_value)) {
            return false;
        }
        break;

    default:
        return context_failure_unsupported_operand(
            context, SV("-"), operand(instruction.B_kind, instruction.B_data));
    }

    context_push_local_value(context, frame, A, A_value);
    return true;
}
