/**
 * Copyright (C) 2024 Cade Weinberg
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
#include <stddef.h>

#include "env/context.h"
#include "imr/operand.h"
#include "support/assert.h"
#include "support/string.h"
#include "support/unreachable.h"

Operand operand(OperandKind kind, OperandData data) {
    return (Operand){.kind = kind, .data = data};
}

Operand operand_ssa(u32 ssa) {
    return (Operand){.kind = OPERAND_KIND_SSA, .data.ssa = ssa};
}

Operand operand_constant(u32 index) {
    return (Operand){.kind = OPERAND_KIND_CONSTANT, .data.constant = index};
}

Operand operand_label(ConstantString *cs) {
    return (Operand){.kind = OPERAND_KIND_LABEL, .data.label = cs};
}

Operand operand_u8(u8 u8_) {
    return (Operand){.kind = OPERAND_KIND_U8, .data.u8_ = u8_};
}

Operand operand_u16(u16 u16_) {
    return (Operand){.kind = OPERAND_KIND_U16, .data.u16_ = u16_};
}

Operand operand_u32(u32 u32_) {
    return (Operand){.kind = OPERAND_KIND_U32, .data.u32_ = u32_};
}

Operand operand_u64(u64 u64_) {
    return (Operand){.kind = OPERAND_KIND_U64, .data.u64_ = u64_};
}

Operand operand_i8(i8 i8_) {
    return (Operand){.kind = OPERAND_KIND_I8, .data.i8_ = i8_};
}

Operand operand_i16(i16 i16_) {
    return (Operand){.kind = OPERAND_KIND_I16, .data.i16_ = i16_};
}

Operand operand_i32(i32 i32_) {
    return (Operand){.kind = OPERAND_KIND_I32, .data.i32_ = i32_};
}

Operand operand_i64(i64 i64_) {
    return (Operand){.kind = OPERAND_KIND_I64, .data.i64_ = i64_};
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_KIND_SSA:      return A.data.ssa == B.data.ssa;
    case OPERAND_KIND_CONSTANT: return A.data.constant == B.data.constant;
    case OPERAND_KIND_U8:       return A.data.u8_ == B.data.u8_;
    case OPERAND_KIND_U16:      return A.data.u16_ == B.data.u16_;
    case OPERAND_KIND_U32:      return A.data.u32_ == B.data.u32_;
    case OPERAND_KIND_U64:      return A.data.u64_ == B.data.u64_;
    case OPERAND_KIND_I8:       return A.data.i8_ == B.data.i8_;
    case OPERAND_KIND_I16:      return A.data.i16_ == B.data.i16_;
    case OPERAND_KIND_I32:      return A.data.i32_ == B.data.i32_;
    case OPERAND_KIND_I64:      return A.data.i64_ == B.data.i64_;
    case OPERAND_KIND_LABEL:    return A.data.label == B.data.label;
    default:                    EXP_UNREACHABLE();
    }
}

bool operand_is_index(Operand A) {
    switch (A.kind) {
    case OPERAND_KIND_U8:
    case OPERAND_KIND_U16:
    case OPERAND_KIND_U32:
    case OPERAND_KIND_U64: return true;
    case OPERAND_KIND_I8:  return A.data.i8_ >= 0;
    case OPERAND_KIND_I16: return A.data.i16_ >= 0;
    case OPERAND_KIND_I32: return A.data.i32_ >= 0;
    case OPERAND_KIND_I64: return A.data.i64_ >= 0;
    default:               return false;
    }
}

u64 operand_as_index(Operand A) {
    exp_assert(operand_is_index(A));
    switch (A.kind) {
    case OPERAND_KIND_U8:  return A.data.u8_;
    case OPERAND_KIND_U16: return A.data.u16_;
    case OPERAND_KIND_U32: return A.data.u32_;
    case OPERAND_KIND_U64: return A.data.u64_;
    case OPERAND_KIND_I8:  return (u64)A.data.i8_;
    case OPERAND_KIND_I16: return (u64)A.data.i16_;
    case OPERAND_KIND_I32: return (u64)A.data.i32_;
    case OPERAND_KIND_I64: return (u64)A.data.i64_;

    default: EXP_UNREACHABLE();
    }
}

static void print_operand_ssa(String *restrict string, u32 ssa) {
    string_append(string, SV("%"));
    string_append_u64(string, ssa);
}

static void print_operand_value(String *restrict string,
                                u32 index,
                                Context *restrict context) {
    Value *value = context_constants_at(context, index);
    print_value(string, value, context);
}

static void print_operand_label(String *restrict string,
                                ConstantString const *restrict cs) {
    string_append(string, SV("%"));
    string_append(string, constant_string_to_view(cs));
}

void print_operand(String *restrict string,
                   Operand operand,
                   Context *restrict context) {
    switch (operand.kind) {
    case OPERAND_KIND_SSA: print_operand_ssa(string, operand.data.ssa); break;
    case OPERAND_KIND_CONSTANT:
        print_operand_value(string, operand.data.constant, context);
        break;
    case OPERAND_KIND_LABEL:
        print_operand_label(string, operand.data.label);
        break;
    case OPERAND_KIND_U8:  string_append_u64(string, operand.data.u8_); break;
    case OPERAND_KIND_U16: string_append_u64(string, operand.data.u16_); break;
    case OPERAND_KIND_U32: string_append_u64(string, operand.data.u32_); break;
    case OPERAND_KIND_U64: string_append_u64(string, operand.data.u64_); break;
    case OPERAND_KIND_I8:  string_append_i64(string, operand.data.i8_); break;
    case OPERAND_KIND_I16: string_append_i64(string, operand.data.i16_); break;
    case OPERAND_KIND_I32: string_append_i64(string, operand.data.i32_); break;
    case OPERAND_KIND_I64: string_append_i64(string, operand.data.i64_); break;

    default: EXP_UNREACHABLE();
    }
}
