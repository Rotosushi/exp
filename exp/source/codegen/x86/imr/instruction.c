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

#include "codegen/x86/imr/instruction.h"
#include "support/assert.h"
#include "support/unreachable.h"

static x86_Instruction x64_instruction(x86_Opcode opcode) {
    return (x86_Instruction){.opcode = opcode};
}

static x86_Instruction x64_instruction_A(x86_Opcode opcode, x86_Operand A) {
    return (x86_Instruction){
        .opcode = opcode, .A_kind = A.kind, .A_data = A.data};
}

static x86_Instruction
x64_instruction_AB(x86_Opcode opcode, x86_Operand A, x86_Operand B) {
    return (x86_Instruction){.opcode = opcode,
                             .A_kind = A.kind,
                             .A_data = A.data,
                             .B_kind = B.kind,
                             .B_data = B.data};
}

x86_Instruction x86_ret() { return x64_instruction(X64_OPCODE_RET); }

x86_Instruction x86_call(x86_Operand label) {
    return x64_instruction_A(X64_OPCODE_CALL, label);
}

x86_Instruction x86_push(x86_Operand src) {
    return x64_instruction_A(X64_OPCODE_PUSH, src);
}

x86_Instruction x86_pop(x86_Operand dst) {
    return x64_instruction_A(X64_OPCODE_POP, dst);
}

x86_Instruction x86_mov(x86_Operand dst, x86_Operand src) {
    return x64_instruction_AB(X64_OPCODE_MOV, dst, src);
}

x86_Instruction x86_lea(x86_Operand dst, x86_Operand src) {
    return x64_instruction_AB(X64_OPCODE_LEA, dst, src);
}

x86_Instruction x86_neg(x86_Operand dst) {
    return x64_instruction_A(X64_OPCODE_NEG, dst);
}

x86_Instruction x86_add(x86_Operand dst, x86_Operand src) {
    return x64_instruction_AB(X64_OPCODE_ADD, dst, src);
}

x86_Instruction x86_sub(x86_Operand dst, x86_Operand src) {
    return x64_instruction_AB(X64_OPCODE_SUB, dst, src);
}

x86_Instruction x86_imul(x86_Operand src) {
    return x64_instruction_A(X64_OPCODE_IMUL, src);
}

x86_Instruction x86_idiv(x86_Operand src) {
    return x64_instruction_A(X64_OPCODE_IDIV, src);
}

static void print_x86_instruction_A(String *restrict buffer,
                                    x86_Instruction instruction,
                                    StringView      mnemonic) {
    string_append(buffer, mnemonic);
    string_append(buffer, SV("\t"));
    print_x86_operand(buffer,
                      x86_operand(instruction.A_kind, instruction.A_data));
}

static void print_x86_instruction_AB(String *restrict buffer,
                                     x86_Instruction instruction,
                                     StringView      mnemonic) {
    string_append(buffer, mnemonic);
    string_append(buffer, SV("\t"));
    print_x86_operand(buffer,
                      x86_operand(instruction.A_kind, instruction.A_data));
    string_append(buffer, SV(", "));
    print_x86_operand(buffer,
                      x86_operand(instruction.B_kind, instruction.B_data));
}

void print_x86_instruction(String *restrict buffer,
                           x86_Instruction instruction) {
    exp_assert(buffer != NULL);
    switch (instruction.opcode) {
    case X64_OPCODE_RET: {
        string_append(buffer, SV("ret"));
        break;
    }

    case X64_OPCODE_CALL: {
        print_x86_instruction_A(buffer, instruction, SV("call"));
        break;
    }

    case X64_OPCODE_PUSH: {
        print_x86_instruction_A(buffer, instruction, SV("push"));
        break;
    }

    case X64_OPCODE_POP: {
        print_x86_instruction_A(buffer, instruction, SV("pop"));
        break;
    }

    case X64_OPCODE_MOV: {
        print_x86_instruction_AB(buffer, instruction, SV("mov"));
        break;
    }

    case X64_OPCODE_LEA: {
        print_x86_instruction_AB(buffer, instruction, SV("lea"));
        break;
    }

    case X64_OPCODE_NEG: {
        print_x86_instruction_A(buffer, instruction, SV("neg"));
        break;
    }

    case X64_OPCODE_ADD: {
        print_x86_instruction_AB(buffer, instruction, SV("add"));
        break;
    }

    case X64_OPCODE_SUB: {
        print_x86_instruction_AB(buffer, instruction, SV("sub"));
        break;
    }

    case X64_OPCODE_IMUL: {
        print_x86_instruction_A(buffer, instruction, SV("imul"));
        break;
    }

    case X64_OPCODE_IDIV: {
        print_x86_instruction_A(buffer, instruction, SV("idiv"));
        break;
    }

    default: EXP_UNREACHABLE();
    }
}
