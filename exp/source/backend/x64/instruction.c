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

#include "backend/x64/context.h"
#include "backend/x64/instruction.h"
#include "backend/x64/registers.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/minmax.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

static x64_Instruction x64_instruction(x64_Opcode opcode) {
    x64_Instruction I = {.opcode = opcode};
    return I;
}

static x64_Instruction x64_instruction_A(x64_Opcode opcode, x64_Operand A) {
    x64_Instruction I = {.opcode = opcode, .A_kind = A.kind, .A_data = A.data};
    return I;
}

static x64_Instruction
x64_instruction_AB(x64_Opcode opcode, x64_Operand A, x64_Operand B) {
    x64_Instruction I = {.opcode = opcode,
                         .A_kind = A.kind,
                         .A_data = A.data,
                         .B_kind = B.kind,
                         .B_data = B.data};
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

static u8 x64_operand_size(x64_OperandKind kind,
                           x64_OperandData data,
                           x64_Context *x64_context) {
    switch (kind) {
    case X64_OPERAND_KIND_GPR: return x64_gpr_size(data.gpr);

    case X64_OPERAND_KIND_ADDRESS: {
        return x64_gpr_size(data.address.base);
    }

    case X64_OPERAND_KIND_IMMEDIATE: {
        return (u8)size_of(context_i32_type(x64_context->context));
    }

    case X64_OPERAND_KIND_LABEL: {
        // #NOTE: labels are always functions in this current version of
        //  the compiler, which means they have the same size as a function
        //  pointer.
        return 8;
    }

    case X64_OPERAND_KIND_CONSTANT: {
        Value *constant  = x64_context_constants_at(x64_context, data.constant);
        Type const *type = type_of_value(constant, x64_context->context);
        return (u8)size_of(type);
    }

    default: EXP_UNREACHABLE();
    }
}

static u8 x64_instruction_operand_size(x64_Instruction I,
                                       x64_Context *x64_context) {
    u8 largest_size = x64_operand_size(I.A_kind, I.A_data, x64_context);
    return max_u8(largest_size,
                  x64_operand_size(I.B_kind, I.B_data, x64_context));
}

static void x64_emit_mnemonic(StringView mnemonic,
                              x64_Instruction I,
                              String *buffer,
                              x64_Context *x64_context) {
    string_append(buffer, mnemonic);
    // since we only support i32's we only use 32 bit operands.
    // thus we know that we can always use the 'l' instruction
    // size suffix
    //
    // #TODO: we need to get the size of the type that the operand is
    // representing
    //  and choose the correct mnemonic suffix accordingly.
    //  'b' -> 1 byte
    //  'w' -> 2 byte
    //  'l' -> 4 byte
    //  'q' -> 8 byte
    //  #NOTE:
    //   's' -> f32
    //   'l' -> f64
    //   't' -> f80

    u8 operand_size = x64_instruction_operand_size(I, x64_context);
    if (operand_size == 1) string_append(buffer, SV("b\t"));
    else if (operand_size == 2) string_append(buffer, SV("w\t"));
    else if (operand_size <= 4) string_append(buffer, SV("l\t"));
    else if (operand_size <= 8) string_append(buffer, SV("q\t"));
    else EXP_UNREACHABLE();
}

static void x64_emit_gpr(x64_GPR gpr, String *buffer) {
    string_append(buffer, SV("%"));
    string_append(buffer, x64_gpr_to_sv(gpr));
}

static void x64_emit_address(x64_Address address, String *buffer) {
    string_append_i64(buffer, address.offset);

    string_append(buffer, SV("("));
    x64_emit_gpr(address.base, buffer);

    if (address.has_index) {
        string_append(buffer, SV(", "));
        x64_emit_gpr(address.index, buffer);
        string_append(buffer, SV(", "));
        string_append_u64(buffer, address.scale);
    }

    string_append(buffer, SV(")"));
}

static void x64_emit_operand(x64_OperandKind kind,
                             x64_OperandData data,
                             String *buffer,
                             x64_Context *x64_context) {
    switch (kind) {
    case X64_OPERAND_KIND_GPR: {
        x64_emit_gpr(data.gpr, buffer);
        break;
    }

    case X64_OPERAND_KIND_ADDRESS: {
        x64_emit_address(data.address, buffer);
        break;
    }

    case X64_OPERAND_KIND_IMMEDIATE: {
        string_append(buffer, SV("$"));
        string_append_i64(buffer, data.immediate);
        break;
    }

    case X64_OPERAND_KIND_CONSTANT: {
        // #TODO: this needs to robustly handle all scalar constants.
        //  and it is important to note that only scalar constants
        //  can validly appear here. The only scalar constants that
        //  we have are 64 bit ones. and at the moment we do not
        //  have any 64 bit constants.
        EXP_UNREACHABLE();
        break;
    }

    case X64_OPERAND_KIND_LABEL: {
        StringView name = x64_context_labels_at(x64_context, data.label);
        string_append(buffer, name);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}

void x64_instruction_emit_A(StringView mnemonic,
                            x64_Instruction I,
                            String *buffer,
                            x64_Context *x64_context) {
    x64_emit_mnemonic(mnemonic, I, buffer, x64_context);
    x64_emit_operand(I.A_kind, I.A_data, buffer, x64_context);
}

void x64_instruction_emit_AB(StringView mnemonic,
                             x64_Instruction I,
                             String *buffer,
                             x64_Context *x64_context) {
    x64_emit_mnemonic(mnemonic, I, buffer, x64_context);
    x64_emit_operand(I.B_kind, I.B_data, buffer, x64_context);
    string_append(buffer, SV(", "));
    x64_emit_operand(I.A_kind, I.A_data, buffer, x64_context);
}

void x64_instruction_emit(x64_Instruction I,
                          String *buffer,
                          x64_Context *x64_context) {
    switch (I.opcode) {
    case X64_OPCODE_RETURN: {
        // ret is unique in that it has only one valid operand size,
        // and we are not currently making use of that form of the
        // ret instruction in the compiler.
        string_append(buffer, SV("ret"));
        break;
    }

    case X64_OPCODE_CALL: {
        x64_instruction_emit_A(SV("call\t"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_PUSH: {
        x64_instruction_emit_A(SV("push\t"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_POP: {
        x64_instruction_emit_A(SV("pop\t"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_MOV: {
        x64_instruction_emit_AB(SV("mov"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_LEA: {
        x64_instruction_emit_AB(SV("lea"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_NEG: {
        x64_instruction_emit_A(SV("neg"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_ADD: {
        x64_instruction_emit_AB(SV("add"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_SUB: {
        x64_instruction_emit_AB(SV("sub"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_IMUL: {
        x64_instruction_emit_A(SV("imul"), I, buffer, x64_context);
        break;
    }

    case X64_OPCODE_IDIV: {
        x64_instruction_emit_A(SV("idiv"), I, buffer, x64_context);
        break;
    }

    default: EXP_UNREACHABLE();
    }
}
