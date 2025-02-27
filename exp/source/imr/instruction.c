/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "imr/instruction.h"
#include "utility/assert.h"

// #NOTE: unconditional jump will use this form, so we aren't removing it,
//  even though return is no longer using this form, thus it is unused
[[maybe_unused]] static Instruction instruction_A(Opcode opcode, Operand A) {
    Instruction I = {.opcode = opcode, .A = A};
    return I;
}

static Instruction instruction_AB(Opcode opcode, Operand A, Operand B) {
    Instruction I = {.opcode = opcode, .A = A, .B = B};
    return I;
}

static Instruction instruction_ABC(Opcode opcode, Operand A, Operand B,
                                   Operand C) {
    Instruction I = {.opcode = opcode, .A = A, .B = B, .C = C};
    return I;
}

Instruction instruction_return(Operand dst, Operand result) {
    return instruction_AB(OPCODE_RET, dst, result);
}

Instruction instruction_call(Operand dst, Operand label, Operand args) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_CALL, dst, label, args);
}

Instruction instruction_dot(Operand dst, Operand src, Operand index) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_DOT, dst, src, index);
}

Instruction instruction_load(Operand dst, Operand src) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_AB(OPCODE_LOAD, dst, src);
}

Instruction instruction_negate(Operand dst, Operand src) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_AB(OPCODE_NEG, dst, src);
}

Instruction instruction_add(Operand dst, Operand left, Operand right) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_ADD, dst, left, right);
}

Instruction instruction_subtract(Operand dst, Operand left, Operand right) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_SUB, dst, left, right);
}

Instruction instruction_multiply(Operand dst, Operand left, Operand right) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_MUL, dst, left, right);
}

Instruction instruction_divide(Operand dst, Operand left, Operand right) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_DIV, dst, left, right);
}

Instruction instruction_modulus(Operand dst, Operand left, Operand right) {
    EXP_ASSERT(dst.kind == OPERAND_KIND_SSA);
    return instruction_ABC(OPCODE_MOD, dst, left, right);
}
