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
typedef enum Opcode : u8 {
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
  OPC_RET, // B -- R = B,    <return>
           // B -- R = Values[B], <return>
           // B -- R = SSA[B], <return>
           // B -- R = GlobalSymbol[B], <return>

  OPC_CALL, // ABC -- SSA[A] = GlobalSymbol[B](Calls[C])

  // why not
  // OPC_GEA, // ABC -- SSA[A] = get_element_address(SSA[B], C)
  //          // ABC -- SSA[A] = get_element_address(Values[B], C)
  //          // ABC -- SSA[A] = get_element_address(GlobalSymbols[B], C)
  // plus
  // OPC_DEREF, // AB -- SSA[A] = deref(SSA[B])
  //            // AB -- SSA[A] = deref(GlobalSymbols[B])
  // which can be combined to create both "A.B" and "A[B]"
  OPC_DOT, // ABC -- SSA[A] = SSA[B].C
           // ABC -- SSA[A] = Values[B].C
           // ABC -- SSA[A] = GlobalSymbol[B].C

  OPC_LEA, // AB -- SSA[A] = address_of(SSA[B])
           // AB -- SSA[A] = address_of(GlobalSymbol[B])

  OPC_MOVE, // AB -- SSA[A] = B
            // AB -- SSA[A] = Values[B]
            // AB -- SSA[A] = SSA[B]
            // AB -- SSA[A] = GlobalSymbol[B]
            // AB -- GlobalSymbol[A] = B
            // AB -- GlobalSymbol[A] = Values[B]
            // AB -- GlobalSymbol[A] = SSA[B]
            // AB -- GlobalSymbol[A] = GlobalSymbol[B]

  OPC_NEG, // AB -- SSA[A] = -(B)
           // AB -- SSA[A] = -(SSA[B])
           // AB -- SSA[A] = GlobalSymbol[B]

  OPC_ADD, // ABC -- SSA[A] = SSA[B] + SSA[C]
           // ABC -- SSA[A] = SSA[B] + C
           // ABC -- SSA[A] = B    + SSA[C]
           // ABC -- SSA[A] = B    + C

  OPC_SUB, // ABC -- SSA[A] = SSA[B] - SSA[C]
           // ABC -- SSA[A] = SSA[B] - C
           // ABC -- SSA[A] = B    - SSA[C]
           // ABC -- SSA[A] = B    - C

  OPC_MUL, // ABC -- SSA[A] = SSA[B] * SSA[C]
           // ABC -- SSA[A] = SSA[B] * C
           // ABC -- SSA[A] = B    * SSA[C]
           // ABC -- SSA[A] = B    * C

  OPC_DIV, // ABC -- SSA[A] = SSA[B] / SSA[C]
           // ABC -- SSA[A] = SSA[B] / C
           // ABC -- SSA[A] = B    / SSA[C]
           // ABC -- SSA[A] = B    / C

  OPC_MOD, // ABC -- SSA[A] = SSA[B] % SSA[C]
           // ABC -- SSA[A] = SSA[B] % C
           // ABC -- SSA[A] = B    % SSA[C]
           // ABC -- SSA[A] = B    % C
} Opcode;

typedef enum InstructionFormat : u8 {
  IFMT_B,
  IFMT_AB,
  IFMT_ABC,
} InstructionFormat;

/**
 * @brief represents a bytecode instruction
 */
typedef struct Instruction {
  Opcode opcode;
  InstructionFormat format;
  Operand A;
  Operand B;
  Operand C;
} Instruction;

Instruction instruction_ret(Operand result);
Instruction instruction_call(Operand dst, Operand label, Operand args);
Instruction instruction_dot(Operand dst, Operand src, Operand index);
Instruction instruction_move(Operand dst, Operand src);
Instruction instruction_neg(Operand dst, Operand src);
Instruction instruction_add(Operand dst, Operand left, Operand right);
Instruction instruction_sub(Operand dst, Operand left, Operand right);
Instruction instruction_mul(Operand dst, Operand left, Operand right);
Instruction instruction_div(Operand dst, Operand left, Operand right);
Instruction instruction_mod(Operand dst, Operand left, Operand right);

#endif // !EXP_IMR_INSTRUCTION_H
