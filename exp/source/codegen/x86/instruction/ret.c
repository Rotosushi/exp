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

#include "codegen/x86/instruction/ret.h"
#include "support/unreachable.h"

void x86_codegen_ret(Instruction          instruction,
                     [[maybe_unused]] u32 block_index,
                     x86_Function *restrict x86_function,
                     [[maybe_unused]] Context *restrict context) {
    switch (instruction.B_kind) {
    case OPERAND_KIND_SSA:
    case OPERAND_KIND_CONSTANT:
    case OPERAND_KIND_LABEL:    EXP_UNREACHABLE(); break;

    // #NOTE: I am still considering this. Is it fine to not return anything
    // here, or is it safer to always return zero? If we do not emit any
    // statement for return nil, becuase it is in my opinion practically
    // unecessary, and if the rax register can be unused by the entire body of
    // the procedure, then we can avoid the (albeit miniscule) overhead of
    // saving/restoring the rax register in this body. Which, given the way our
    // register allocator works, is not going to happen. it simply chooses the
    // first available register starting from 0 and going to 15, and rax is
    // register 0.
    case OPERAND_KIND_NIL:
        x86_Operand operand =
            x86_operand_location(x86_function->result->location);
        x86_function_append(x86_function, x86_xor(operand, operand));
        break;

    case OPERAND_KIND_BOOL:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_bool(instruction.B_data.bool_)));
        break;

    case OPERAND_KIND_U8:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_u8(instruction.B_data.u8_)));
        break;

    case OPERAND_KIND_U16:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_u16(instruction.B_data.u16_)));
        break;

    case OPERAND_KIND_U32:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_u32(instruction.B_data.u32_)));
        break;

    case OPERAND_KIND_U64:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_u64(instruction.B_data.u64_)));
        break;

    case OPERAND_KIND_I8:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_i8(instruction.B_data.i8_)));
        break;

    case OPERAND_KIND_I16:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_i16(instruction.B_data.i16_)));
        break;

    case OPERAND_KIND_I32:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_i32(instruction.B_data.i32_)));
        break;

    case OPERAND_KIND_I64:
        x86_function_append(
            x86_function,
            x86_mov(x86_operand_location(x86_function->result->location),
                    x86_operand_i64(instruction.B_data.i64_)));
        break;
    }

    x86_function_footer(x86_function);
    x86_function_append(x86_function, x86_ret());
}
