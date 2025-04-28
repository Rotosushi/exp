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
#include <stddef.h>

#include "codegen/x86/imr/instruction.h"
#include "codegen/x86/imr/registers.h"
#include "support/unreachable.h"

static x86_Instruction x64_instruction(x86_Opcode opcode) {
    x86_Instruction I = {.opcode = opcode};
    return I;
}

static x86_Instruction x64_instruction_A(x86_Opcode opcode, x86_Operand A) {
    x86_Instruction I = {.opcode = opcode, .A = A};
    return I;
}

static x86_Instruction
x64_instruction_AB(x86_Opcode opcode, x86_Operand A, x86_Operand B) {
    x86_Instruction I = {.opcode = opcode, .A = A, .B = B};
    return I;
}

x86_Instruction x86_ret() { return x64_instruction(X64_OPCODE_RETURN); }

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

static void x64_emit_mnemonic(StringView                       mnemonic,
                              [[maybe_unused]] x86_Instruction I,
                              String *restrict buffer,
                              [[maybe_unused]] Context *restrict context) {
    string_append(buffer, mnemonic);
    // if either operand is a register then it is a 64 bit GPR, so we
    // know we need the 'q' suffix. because, as a simplification, we
    // only support the 64 bit GPRs.
    //
    // as a simplification, we always allocate a 64 bit word for each
    // type we currently support.
    //
    // #TODO:
    // we need to get the size of the type that the operand is representing
    // and choose the correct mnemonic suffix accordingly.
    // 'b' -> u8
    // 'w' -> u16
    // 'l' -> u32
    // 'q' -> u64
    //
    // additionally
    // 's' -> f32
    // 'l' -> f64
    // 't' -> f80
    //
    // however, relying on the above simplifications, we always load/store
    // a quad word, so we can always emit the 'q' suffix.
    string_append(buffer, SV("q\t"));
}

static void x64_emit_operand(x86_Operand operand,
                             String *restrict buffer,
                             Context *restrict context) {
    switch (operand.kind) {
    case X86_OPERAND_KIND_GPR: {
        string_append(buffer, SV("%"));
        string_append(buffer, x86_gpr_mnemonic(operand.data.gpr));
        break;
    }

    case X86_OPERAND_KIND_ADDRESS: {
        x86_Address *address = &operand.data.address;
        string_append_i64(buffer, address->offset);

        string_append(buffer, SV("(%"));
        string_append(buffer, x86_gpr_mnemonic(address->base));

        if (address->has_index) {
            string_append(buffer, SV(", "));
            string_append(buffer, x86_gpr_mnemonic(address->index));
            string_append(buffer, SV(", "));
            string_append_u64(buffer, address->scale);
        }

        string_append(buffer, SV(")"));
        break;
    }

    case X86_OPERAND_KIND_IMMEDIATE: {
        string_append(buffer, SV("$"));
        string_append_i64(buffer, operand.data.immediate);
        break;
    }

    case X86_OPERAND_KIND_CONSTANT: {
        Value *constant = context_constants_at(context, operand.data.constant);
        // #TODO: this needs to robustly handle all scalar constants.
        //  and it is important to note that only scalar constants
        //  can validly appear here.
        assert(constant->kind == VALUE_KIND_I64);
        string_append(buffer, SV("$"));
        string_append_i64(buffer, constant->i64_);
        break;
    }

    case X86_OPERAND_KIND_LABEL: {
        StringView name = constant_string_to_view(operand.data.label);
        string_append(buffer, name);
        break;
    }

    default: unreachable();
    }
}

void x86_instruction_emit(x86_Instruction I,
                          String *restrict buffer,
                          Context *restrict context) {
    switch (I.opcode) {
    case X64_OPCODE_RETURN: {
        string_append(buffer, SV("ret"));
        break;
    }

    case X64_OPCODE_CALL: {
        string_append(buffer, SV("call\t"));
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_PUSH: {
        x64_emit_mnemonic(SV("push"), I, buffer, context);
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_POP: {
        x64_emit_mnemonic(SV("pop"), I, buffer, context);
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_MOV: {
        x64_emit_mnemonic(SV("mov"), I, buffer, context);
        x64_emit_operand(I.B, buffer, context);
        string_append(buffer, SV(", "));
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_LEA: {
        x64_emit_mnemonic(SV("lea"), I, buffer, context);
        x64_emit_operand(I.B, buffer, context);
        string_append(buffer, SV(", "));
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_NEG: {
        x64_emit_mnemonic(SV("neg"), I, buffer, context);
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_ADD: {
        x64_emit_mnemonic(SV("add"), I, buffer, context);
        x64_emit_operand(I.B, buffer, context);
        string_append(buffer, SV(", "));
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_SUB: {
        x64_emit_mnemonic(SV("sub"), I, buffer, context);
        x64_emit_operand(I.B, buffer, context);
        string_append(buffer, SV(", "));
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_IMUL: {
        x64_emit_mnemonic(SV("imul"), I, buffer, context);
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    case X64_OPCODE_IDIV: {
        x64_emit_mnemonic(SV("idiv"), I, buffer, context);
        x64_emit_operand(I.A, buffer, context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}
