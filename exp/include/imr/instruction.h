// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_IMR_INSTRUCTION_H
#define EXP_IMR_INSTRUCTION_H
#include "imr/operand.h"

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode : u16 {
    /*
     * <...> -> side effect
     * ip    -> the instruction pointer
     * R     -> the return value location
     * A|B|C -> an operand
     * SSA[*]           -> indexing the locals array.
     * Values[*]     -> indexing the constants array.
     * GlobalSymbol[*]  -> indexing the global names array followed by
     *          indexing the global symbol table.
     * Calls[*]         -> indexing the actual argument lists array.
     */
    OPCODE_RETURN, // B -- R = B,    <return>
                   // B -- R = Values[B], <return>
                   // B -- R = SSA[B], <return>

    OPCODE_CALL, // ABC -- SSA[A] = GlobalSymbol[B](Calls[C])

    OPCODE_DOT, // ABC -- SSA[A] = SSA[B].C
                // ABC -- SSA[A] = Values[B].C

    OPCODE_LOAD, // AB  -- SSA[A] = B
                 // AB  -- SSA[A] = Values[B]
                 // AB  -- SSA[A] = SSA[B]

    OPCODE_NEGATE, // AB  -- SSA[A] = -(B)
                   // AB  -- SSA[A] = -(SSA[B])

    OPCODE_ADD, // ABC -- SSA[A] = SSA[B] + SSA[C]
                // ABC -- SSA[A] = SSA[B] + C
                // ABC -- SSA[A] = B    + SSA[C]
                // ABC -- SSA[A] = B    + C

    OPCODE_SUBTRACT, // ABC -- SSA[A] = SSA[B] - SSA[C]
                     // ABC -- SSA[A] = SSA[B] - C
                     // ABC -- SSA[A] = B    - SSA[C]
                     // ABC -- SSA[A] = B    - C

    OPCODE_MULTIPLY, // ABC -- SSA[A] = SSA[B] * SSA[C]
                     // ABC -- SSA[A] = SSA[B] * C
                     // ABC -- SSA[A] = B    * SSA[C]
                     // ABC -- SSA[A] = B    * C

    OPCODE_DIVIDE, // ABC -- SSA[A] = SSA[B] / SSA[C]
                   // ABC -- SSA[A] = SSA[B] / C
                   // ABC -- SSA[A] = B    / SSA[C]
                   // ABC -- SSA[A] = B    / C

    OPCODE_MODULUS, // ABC -- SSA[A] = SSA[B] % SSA[C]
                    // ABC -- SSA[A] = SSA[B] % C
                    // ABC -- SSA[A] = B    % SSA[C]
                    // ABC -- SSA[A] = B    % C
} Opcode;

typedef struct Instruction {
    Opcode opcode;
    OperandKind A_kind;
    OperandKind B_kind;
    OperandKind C_kind;
    OperandData A_data;
    OperandData B_data;
    OperandData C_data;
} Instruction;

Instruction instruction_return(Operand result);
Instruction instruction_call(Operand dst, Operand label, Operand args);
Instruction instruction_dot(Operand dst, Operand src, Operand index);
Instruction instruction_load(Operand dst, Operand src);
Instruction instruction_negate(Operand dst, Operand src);
Instruction instruction_add(Operand dst, Operand left, Operand right);
Instruction instruction_subtract(Operand dst, Operand left, Operand right);
Instruction instruction_multiply(Operand dst, Operand left, Operand right);
Instruction instruction_divide(Operand dst, Operand left, Operand right);
Instruction instruction_modulus(Operand dst, Operand left, Operand right);

struct Context;
void print_instruction(String *restrict string,
                       Instruction instruction,
                       struct Context *restrict context);

#endif // !EXP_IMR_INSTRUCTION_H
