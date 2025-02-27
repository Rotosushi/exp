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

// #NOTE: unconditional jump will use this form, so we aren't removing it,
//  even though return is no longer using this form, thus it is unused
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

static Instruction
instruction_ABC(Opcode opcode, Operand A, Operand B, Operand C) {
    Instruction I = {.opcode = opcode,
                     .A_kind = A.kind,
                     .A_data = A.data,
                     .B_kind = B.kind,
                     .B_data = B.data,
                     .C_kind = C.kind,
                     .C_data = C.data};
    return I;
}

Instruction instruction_return(Operand dst, Operand result) {
    return instruction_AB(OPCODE_RETURN, dst, result);
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
    return instruction_ABC(OPCODE_ADD, dst, left, right);
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
