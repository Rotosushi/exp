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

#include "imr/instruction.h"
#include "env/context.h"
#include "support/assert.h"
#include "support/unreachable.h"

extern Operand operand_A(Instruction instruction);
extern Operand operand_B(Instruction instruction);
extern Operand operand_C(Instruction instruction);

extern Instruction instruction_B(Opcode opcode, Operand B);
extern Instruction instruction_AB(Opcode opcode, Operand A, Operand B);
extern Instruction
instruction_ABC(Opcode opcode, Operand A, Operand B, Operand C);

extern Instruction instruction_return(Operand result);
extern Instruction instruction_call(Operand dst, Operand label, Operand args);
extern Instruction instruction_let(Operand dst, Operand type, Operand src);
extern Instruction instruction_as(Operand dst, Operand type, Operand src);
extern Instruction instruction_dot(Operand dst, Operand src, Operand index);
extern Instruction instruction_neg(Operand dst, Operand src);
extern Instruction instruction_add(Operand dst, Operand left, Operand right);
extern Instruction instruction_sub(Operand dst, Operand left, Operand right);
extern Instruction instruction_mul(Operand dst, Operand left, Operand right);
extern Instruction instruction_div(Operand dst, Operand left, Operand right);
extern Instruction instruction_mod(Operand dst, Operand left, Operand right);

static void print_B(String *restrict string,
                    StringView  mnemonic,
                    Instruction I,
                    Context *restrict context) {
    string_append(string, mnemonic);
    string_append(string, SV(" "));
    print_operand(string, operand(I.B_kind, I.B_data), context);
}

static void print_AB(String *restrict string,
                     StringView  mnemonic,
                     Instruction I,
                     Context *restrict context) {
    string_append(string, mnemonic);
    string_append(string, SV(" "));
    print_operand(string, operand(I.A_kind, I.A_data), context);
    string_append(string, SV(", "));
    print_operand(string, operand(I.B_kind, I.B_data), context);
}

static void print_ABC(String *restrict string,
                      StringView  mnemonic,
                      Instruction I,
                      Context *restrict context) {
    string_append(string, mnemonic);
    string_append(string, SV(" "));
    print_operand(string, operand(I.A_kind, I.A_data), context);
    string_append(string, SV(", "));
    print_operand(string, operand(I.B_kind, I.B_data), context);
    string_append(string, SV(", "));
    print_operand(string, operand(I.C_kind, I.C_data), context);
}

void print_instruction(String *restrict string,
                       Instruction I,
                       struct Context *restrict context) {
    EXP_ASSERT(string != NULL);
    EXP_ASSERT(context != NULL);
    switch (I.opcode) {
    case OPCODE_RET:  print_B(string, SV("ret"), I, context); break;
    case OPCODE_LET:  print_ABC(string, SV("let"), I, context); break;
    case OPCODE_AS:   print_ABC(string, SV("as"), I, context); break;
    case OPCODE_CALL: print_ABC(string, SV("call"), I, context); break;
    case OPCODE_DOT:  print_ABC(string, SV("dot"), I, context); break;
    case OPCODE_NEG:  print_AB(string, SV("neg"), I, context); break;
    case OPCODE_ADD:  print_ABC(string, SV("add"), I, context); break;
    case OPCODE_SUB:  print_ABC(string, SV("sub"), I, context); break;
    case OPCODE_MUL:  print_ABC(string, SV("mul"), I, context); break;
    case OPCODE_DIV:  print_ABC(string, SV("div"), I, context); break;
    case OPCODE_MOD:  print_ABC(string, SV("mod"), I, context); break;

    default: EXP_UNREACHABLE();
    }
}
