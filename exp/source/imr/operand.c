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
 * @file imr/operand.c
 */

#include "imr/operand.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

Operand operand_construct(OperandKind kind, OperandData data) {
    Operand operand_ = {.kind = kind, .data = data};
    return operand_;
}

Operand operand_uninitialized() {
    return operand_construct(OPERAND_UNINITIALIZED, (OperandData){});
}

Operand operand_register(u8 register_) {
    return operand_construct(OPERAND_REGISTER,
                             (OperandData){.register_ = register_});
}

Operand operand_stack(u16 stack) {
    return operand_construct(OPERAND_STACK, (OperandData){.stack = stack});
}

Operand operand_scalar(Scalar scalar) {
    switch (scalar.kind) {
    case SCALAR_NIL:
        return operand_construct(OPERAND_SCALAR_NIL,
                                 (OperandData){.nil = false});
    case SCALAR_BOOL:
        return operand_construct(OPERAND_SCALAR_BOOL,
                                 (OperandData){.bool_ = scalar.data.bool_});
    case SCALAR_U8:
        return operand_construct(OPERAND_SCALAR_U8,
                                 (OperandData){.u8_ = scalar.data.u8_});
    case SCALAR_U16:
        return operand_construct(OPERAND_SCALAR_U16,
                                 (OperandData){.u16_ = scalar.data.u16_});
    case SCALAR_U32:
        return operand_construct(OPERAND_SCALAR_U32,
                                 (OperandData){.u32_ = scalar.data.u32_});
    case SCALAR_U64:
        return operand_construct(OPERAND_SCALAR_U64,
                                 (OperandData){.u64_ = scalar.data.u64_});
    case SCALAR_I8:
        return operand_construct(OPERAND_SCALAR_I8,
                                 (OperandData){.i8_ = scalar.data.i8_});
    case SCALAR_I16:
        return operand_construct(OPERAND_SCALAR_I16,
                                 (OperandData){.i16_ = scalar.data.i16_});
    case SCALAR_I32:
        return operand_construct(OPERAND_SCALAR_I32,
                                 (OperandData){.i32_ = scalar.data.i32_});
    case SCALAR_I64:
        return operand_construct(OPERAND_SCALAR_I64,
                                 (OperandData){.i64_ = scalar.data.i64_});
    default: EXP_UNREACHABLE();
    }
}

Scalar operand_as_scalar(Operand operand) {
    EXP_ASSERT(operand.kind >= OPERAND_SCALAR_NIL);
    EXP_ASSERT(operand.kind <= OPERAND_SCALAR_I64);
    switch (operand.kind) {
    case OPERAND_SCALAR_NIL:  return scalar_nil();
    case OPERAND_SCALAR_BOOL: return scalar_bool(operand.data.bool_);
    case OPERAND_SCALAR_U8:   return scalar_u8(operand.data.u8_);
    case OPERAND_SCALAR_U16:  return scalar_u16(operand.data.u16_);
    case OPERAND_SCALAR_U32:  return scalar_u32(operand.data.u32_);
    case OPERAND_SCALAR_U64:  return scalar_u64(operand.data.u64_);
    case OPERAND_SCALAR_I8:   return scalar_i8(operand.data.i8_);
    case OPERAND_SCALAR_I16:  return scalar_i16(operand.data.i16_);
    case OPERAND_SCALAR_I32:  return scalar_i32(operand.data.i32_);
    case OPERAND_SCALAR_I64:  return scalar_i64(operand.data.i64_);
    default:                  EXP_UNREACHABLE();
    }
}

bool operand_equality(Operand A, Operand B) {
    if (A.kind != B.kind) { return false; }

    switch (A.kind) {
    case OPERAND_REGISTER: return A.data.register_ == B.data.register_;
    case OPERAND_STACK:    return A.data.stack == B.data.stack;
    default:               EXP_UNREACHABLE();
    }
}

void print_operand(String *buffer, Operand operand) {
    switch (operand.kind) {
    case OPERAND_REGISTER:
        u8 register_ = operand.data.register_;
        string_append(buffer, SV("r"));
        string_append_u64(buffer, register_);
        break;
    case OPERAND_STACK:
        u16 stack = operand.data.stack;
        string_append(buffer, SV("s"));
        string_append_u64(buffer, stack);
        break;
    default: EXP_UNREACHABLE();
    }
}
