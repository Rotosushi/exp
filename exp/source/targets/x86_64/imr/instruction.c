/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "targets/x86_64/imr/instruction.h"
#include "env/context.h"
#include "targets/x86_64/print/mnemonic.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

static x86_64_Instruction x86_64_instruction(x86_64_Opcode opcode) {
    return (x86_64_Instruction){.opcode = opcode};
}

static x86_64_Instruction x86_64_instruction_A(x86_64_Opcode opcode,
                                               x86_64_Operand A) {
    return (x86_64_Instruction){
        .opcode = opcode, .A_kind = A.kind, .A_data = A.data};
}

static x86_64_Instruction x86_64_instruction_AB(x86_64_Opcode opcode,
                                                x86_64_Operand A,
                                                x86_64_Operand B) {
    return (x86_64_Instruction){.opcode = opcode,
                                .A_kind = A.kind,
                                .B_kind = B.kind,
                                .A_data = A.data,
                                .B_data = B.data};
}

x86_64_Instruction x86_64_ret() {
    return x86_64_instruction(X86_64_OPCODE_RET);
}

x86_64_Instruction x86_64_call(x86_64_Operand target) {
    return x86_64_instruction_A(X86_64_OPCODE_CALL, target);
}

x86_64_Instruction x86_64_push(x86_64_Operand source) {
    return x86_64_instruction_A(X86_64_OPCODE_PUSH, source);
}

x86_64_Instruction x86_64_pop(x86_64_Operand target) {
    return x86_64_instruction_A(X86_64_OPCODE_POP, target);
}

x86_64_Instruction x86_64_mov(x86_64_Operand target, x86_64_Operand source) {
    return x86_64_instruction_AB(X86_64_OPCODE_MOV, target, source);
}

x86_64_Instruction x86_64_lea(x86_64_Operand target, x86_64_Operand source) {
    return x86_64_instruction_AB(X86_64_OPCODE_LEA, target, source);
}

x86_64_Instruction x86_64_neg(x86_64_Operand target) {
    return x86_64_instruction_A(X86_64_OPCODE_NEG, target);
}

x86_64_Instruction x86_64_add(x86_64_Operand target, x86_64_Operand source) {
    return x86_64_instruction_AB(X86_64_OPCODE_ADD, target, source);
}

x86_64_Instruction x86_64_sub(x86_64_Operand target, x86_64_Operand source) {
    return x86_64_instruction_AB(X86_64_OPCODE_SUB, target, source);
}

x86_64_Instruction x86_64_imul(x86_64_Operand source) {
    return x86_64_instruction_A(X86_64_OPCODE_IMUL, source);
}

x86_64_Instruction x86_64_idiv(x86_64_Operand source) {
    return x86_64_instruction_A(X86_64_OPCODE_IDIV, source);
}

static void print_x86_64_instruction_A(String *buffer, StringView mnemonic,
                                       x86_64_Instruction instruction,
                                       TranslationUnit *context) {
    u8 size = x86_64_operand_size(instruction.A_kind, instruction.A_data);
    print_x86_64_mnemonic(buffer, mnemonic, size);
    print_x86_64_operand(buffer, instruction.A_kind, instruction.A_data,
                         context);
}

static void print_x86_64_instruction_AB(String *buffer, StringView mnemonic,
                                        x86_64_Instruction instruction,
                                        TranslationUnit *context) {
    u8 size = x86_64_operand_size(instruction.A_kind, instruction.A_data);
    print_x86_64_mnemonic(buffer, mnemonic, size);
    print_x86_64_operand(buffer, instruction.B_kind, instruction.B_data,
                         context);
    string_append(buffer, SV(", "));
    print_x86_64_operand(buffer, instruction.A_kind, instruction.A_data,
                         context);
}

void print_x86_64_instruction(String *buffer, x86_64_Instruction instruction,
                              TranslationUnit *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    switch (instruction.opcode) {
    case X86_64_OPCODE_RET: {
        // #NOTE: x86-64 ret mnemonic has no arguments and thus
        //  needs no size mnemonic.
        string_append(buffer, SV("  ret"));
        break;
    }

    case X86_64_OPCODE_CALL: {
        print_x86_64_instruction_A(buffer, SV("call"), instruction, context);
        break;
    }

    case X86_64_OPCODE_PUSH: {
        print_x86_64_instruction_A(buffer, SV("push"), instruction, context);
        break;
    }

    case X86_64_OPCODE_POP: {
        print_x86_64_instruction_A(buffer, SV("pop"), instruction, context);
        break;
    }

    case X86_64_OPCODE_MOV: {
        print_x86_64_instruction_AB(buffer, SV("mov"), instruction, context);
        break;
    }

    case X86_64_OPCODE_LEA: {
        print_x86_64_instruction_AB(buffer, SV("lea"), instruction, context);
        break;
    }

    case X86_64_OPCODE_NEG: {
        print_x86_64_instruction_A(buffer, SV("neg"), instruction, context);
        break;
    }

    case X86_64_OPCODE_ADD: {
        print_x86_64_instruction_AB(buffer, SV("add"), instruction, context);
        break;
    }

    case X86_64_OPCODE_SUB: {
        print_x86_64_instruction_AB(buffer, SV("sub"), instruction, context);
        break;
    }

    case X86_64_OPCODE_IMUL: {
        print_x86_64_instruction_A(buffer, SV("imul"), instruction, context);
        break;
    }

    case X86_64_OPCODE_IDIV: {
        print_x86_64_instruction_A(buffer, SV("idiv"), instruction, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}
