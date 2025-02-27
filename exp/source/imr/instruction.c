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
#include <assert.h>

#include "imr/instruction.h"

static Instruction instruction_B(Opcode opcode, Operand B) {
    Instruction I = {.opcode = opcode, .format = IFMT_B, .B = B};
    return I;
}

static Instruction instruction_AB(Opcode opcode, Operand A, Operand B) {
    Instruction I = {.opcode = opcode, .format = IFMT_AB, .A = A, .B = B};
    return I;
}

static Instruction
instruction_ABC(Opcode opcode, Operand A, Operand B, Operand C) {
    Instruction I = {
        .opcode = opcode, .format = IFMT_ABC, .A = A, .B = B, .C = C};
    return I;
}

Instruction instruction_return(Operand result) {
    return instruction_B(OPCODE_RETURN, result);
}

Instruction instruction_call(Operand dst, Operand label, Operand args) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_CALL, dst, label, args);
}

Instruction instruction_dot(Operand dst, Operand src, Operand index) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_DOT, dst, src, index);
}

Instruction instruction_load(Operand dst, Operand src) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_AB(OPCODE_LOAD, dst, src);
}

Instruction instruction_negate(Operand dst, Operand src) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_AB(OPCODE_NEGATE, dst, src);
}

Instruction instruction_add(Operand dst, Operand left, Operand right) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_ADDITION, dst, left, right);
}

Instruction instruction_subtract(Operand dst, Operand left, Operand right) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_SUBTRACT, dst, left, right);
}

Instruction instruction_multiply(Operand dst, Operand left, Operand right) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_MULTIPLY, dst, left, right);
}

Instruction instruction_divide(Operand dst, Operand left, Operand right) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_DIVIDE, dst, left, right);
}

Instruction instruction_modulus(Operand dst, Operand left, Operand right) {
    assert(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_MODULUS, dst, left, right);
}
