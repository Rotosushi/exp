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

#include "codegen/x64/imr/instruction.h"
#include "codegen/x64/imr/registers.h"
#include "support/unreachable.h"

static x64_Instruction x64_instruction(x64_Opcode opcode) {
    x64_Instruction I = {.opcode = opcode};
    return I;
}

static x64_Instruction x64_instruction_A(x64_Opcode opcode, x64_Operand A) {
    x64_Instruction I = {.opcode = opcode, .A = A};
    return I;
}

static x64_Instruction
x64_instruction_AB(x64_Opcode opcode, x64_Operand A, x64_Operand B) {
    x64_Instruction I = {.opcode = opcode, .A = A, .B = B};
    return I;
}

x64_Instruction x64_ret() { return x64_instruction(X64_OPCODE_RETURN); }

x64_Instruction x64_call(x64_Operand label) {
    return x64_instruction_A(X64_OPCODE_CALL, label);
}

x64_Instruction x64_push(x64_Operand src) {
    return x64_instruction_A(X64_OPCODE_PUSH, src);
}

x64_Instruction x64_pop(x64_Operand dst) {
    return x64_instruction_A(X64_OPCODE_POP, dst);
}

x64_Instruction x64_mov(x64_Operand dst, x64_Operand src) {
    return x64_instruction_AB(X64_OPCODE_MOV, dst, src);
}

x64_Instruction x64_lea(x64_Operand dst, x64_Operand src) {
    return x64_instruction_AB(X64_OPCODE_LEA, dst, src);
}

x64_Instruction x64_neg(x64_Operand dst) {
    return x64_instruction_A(X64_OPCODE_NEG, dst);
}

x64_Instruction x64_add(x64_Operand dst, x64_Operand src) {
    return x64_instruction_AB(X64_OPCODE_ADD, dst, src);
}

x64_Instruction x64_sub(x64_Operand dst, x64_Operand src) {
    return x64_instruction_AB(X64_OPCODE_SUB, dst, src);
}

x64_Instruction x64_imul(x64_Operand src) {
    return x64_instruction_A(X64_OPCODE_IMUL, src);
}

x64_Instruction x64_idiv(x64_Operand src) {
    return x64_instruction_A(X64_OPCODE_IDIV, src);
}

static void x64_emit_mnemonic(StringView mnemonic,
                              [[maybe_unused]] x64_Instruction I,
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

static void x64_emit_operand(x64_Operand operand,
                             String *restrict buffer,
                             Context *restrict context) {
    switch (operand.kind) {
    case X64_OPERAND_KIND_GPR: {
        string_append(buffer, SV("%"));
        string_append(buffer, x64_gpr_to_sv(operand.gpr));
        break;
    }

    case X64_OPERAND_KIND_ADDRESS: {
        x64_Address *address = &operand.address;
        string_append_i64(buffer, address->offset);

        string_append(buffer, SV("(%"));
        string_append(buffer, x64_gpr_to_sv(address->base));

        if (address->index != X64_GPR_NONE) {
            string_append(buffer, SV(", "));
            string_append(buffer, x64_gpr_to_sv(address->index));
            string_append(buffer, SV(", "));
            string_append_u64(buffer, address->scale);
        }

        string_append(buffer, SV(")"));
        break;
    }

    case X64_OPERAND_KIND_IMMEDIATE: {
        string_append(buffer, SV("$"));
        string_append_i64(buffer, operand.immediate);
        break;
    }

    case X64_OPERAND_KIND_CONSTANT: {
        Value *constant = context_constants_at(context, operand.constant);
        // #TODO: this needs to robustly handle all scalar constants.
        //  and it is important to note that only scalar constants
        //  can validly appear here.
        assert(constant->kind == VALUE_KIND_I64);
        string_append(buffer, SV("$"));
        string_append_i64(buffer, constant->i64_);
        break;
    }

    case X64_OPERAND_KIND_LABEL: {
        assert(operand.label <= u16_MAX);
        StringView name = context_labels_at(context, operand.label);
        string_append(buffer, name);
        break;
    }

    default: unreachable();
    }
}

void x64_instruction_emit(x64_Instruction I,
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
