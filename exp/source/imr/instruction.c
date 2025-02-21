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
 * @file imr/instruction.c
 */

#include "imr/instruction.h"
#include "utility/assert.h"

[[maybe_unused]] static Instruction instruction_A(Opcode opcode, Operand A) {
    Instruction I = {.opcode = opcode, .A_kind = A.kind, .A_data = A.data};
    return I;
}

static Instruction instruction_AB(Opcode opcode, Operand A, Operand B) {
    Instruction I = {.opcode = opcode,
                     .A_kind = A.kind,
                     .A_data = A.data,
                     .B_kind = B.kind,
                     .B_data = B.data};
    return I;
}

static Instruction instruction_ABC(Opcode opcode, Operand A, Operand B,
                                   Operand C) {
    Instruction I = {.opcode = opcode,
                     .A_kind = A.kind,
                     .A_data = A.data,
                     .B_kind = B.kind,
                     .B_data = B.data,
                     .C_kind = C.kind,
                     .C_data = C.data};
    return I;
}

Instruction instruction_return(Operand result) {
    return instruction_A(OPCODE_RET, result);
}

Instruction instruction_call(Operand dst, Operand label, Operand args) {
    return instruction_ABC(OPCODE_CALL, dst, label, args);
}

Instruction instruction_dot(Operand dst, Operand src, Operand index) {
    return instruction_ABC(OPCODE_DOT, dst, src, index);
}

Instruction instruction_load(Operand dst, Operand src) {
    return instruction_AB(OPCODE_LOAD, dst, src);
}

Instruction instruction_neg(Operand dst, Operand src) {
    return instruction_AB(OPCODE_NEG, dst, src);
}

Instruction instruction_add(Operand dst, Operand left, Operand right) {
    return instruction_ABC(OPCODE_ADD, dst, left, right);
}

Instruction instruction_sub(Operand dst, Operand left, Operand right) {
    return instruction_ABC(OPCODE_SUB, dst, left, right);
}

Instruction instruction_mul(Operand dst, Operand left, Operand right) {
    return instruction_ABC(OPCODE_MUL, dst, left, right);
}

Instruction instruction_div(Operand dst, Operand left, Operand right) {
    return instruction_ABC(OPCODE_DIV, dst, left, right);
}

Instruction instruction_mod(Operand dst, Operand left, Operand right) {
    return instruction_ABC(OPCODE_MOD, dst, left, right);
}
