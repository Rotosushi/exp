// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_INSTRUCTION_H
#define EXP_IMR_INSTRUCTION_H
#include "imr/operand.h"

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode : u8 {
    /*
     * <...> -> side effect
     * ip    -> the instruction index
     * R     -> the return value location
     * A|B|C -> an operand
     * SSA[*]           -> indexing the locals array.
     * Values[*]     -> indexing the constants array.
     * GlobalSymbol[*]  -> indexing the global names array followed by
     *          indexing the global symbol table.
     * Calls[*]         -> indexing the actual argument lists array.
     */
    // Memory
    OPCODE_LOAD, // AB  -- SSA[A] = B
                 // AB  -- SSA[A] = Values[B]
                 // AB  -- SSA[A] = SSA[B]

    // control flow
    OPCODE_RETURN, // AB -- A = B,    <return>
                   // AB -- A = Values[B], <return>
                   // AB -- A = SSA[B], <return>

    OPCODE_CALL, // ABC -- SSA[A] = GlobalSymbol[B](Calls[C])

    // #TODO:
    OPCODE_JUMP, // A -- <ip = A>

    // #TODO:
    OPCODE_JUMP_IF_EQUAL, // ABC -- <ip = A> if SSA[B] == C
                          // ABC -- <ip = A> if Values[B] == C
                          // ABC -- <ip = A> if SSA[B] == SSA[C]
                          // ABC -- <ip = A> if Values[B] == SSA[C]
                          // ABC -- <ip = A> if SSA[B] == Values[C]
                          // ABC -- <ip = A> if Values[B] == Values[C]

    // Unops
    OPCODE_NEGATE, // AB  -- SSA[A] = -(B)
                   // AB  -- SSA[A] = -(SSA[B])

    // Binops
    OPCODE_DOT, // ABC -- SSA[A] = SSA[B].C
                // ABC -- SSA[A] = Values[B].C

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

/**
 * @brief represents a single instruction
 *
 * sizeof(Opcode) == 1
 * sizeof(OperandKind) == 1
 * sizeof(OperandData) == 4
 * 1 + 1*3 + 4*3 ->
 * sizeof(Instruction) == 16
 */
typedef struct Instruction {
    Opcode opcode;
    OperandKind A_kind;
    OperandKind B_kind;
    OperandKind C_kind;
    OperandData A_data;
    OperandData B_data;
    OperandData C_data;
} Instruction;

Instruction instruction_return(Operand dst, Operand result);
Instruction instruction_call(Operand dst, Operand label, Operand args);
Instruction instruction_dot(Operand dst, Operand src, Operand index);
Instruction instruction_load(Operand dst, Operand src);
Instruction instruction_negate(Operand dst, Operand src);
Instruction instruction_add(Operand dst, Operand left, Operand right);
Instruction instruction_subtract(Operand dst, Operand left, Operand right);
Instruction instruction_multiply(Operand dst, Operand left, Operand right);
Instruction instruction_divide(Operand dst, Operand left, Operand right);
Instruction instruction_modulus(Operand dst, Operand left, Operand right);

#endif // !EXP_IMR_INSTRUCTION_H
