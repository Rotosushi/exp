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
#include "utility/int_types.h"

/*
 * opcodes need to allow instructions to represent
 * expressions, which are composed of:
 *  - loads
 *  - stores
 *  - unops
 *  - binops
 *  - calls
 *  - jumps
 *  - phi
 *
 * all instructions are defined to have one of the
 * following formats:
 *
 *
 * B   -> [opcode(u8)][format(u8)][reserved(u16)][B(u16)][reserved(u16)]
 * AB  -> [opcode(u8)][format(u8)][A(u16)][B(u16)][reserved(u16)]
 * ABC -> [opcode(u8)][format(u8)][A(u16)][B(u16)][C(u16)]
 *
 * // #TODO: add extended instruction formats
 * ABx -> [opcode(u8)][format(u8)][A(u16)][Bx(u32)]
 * Bx  -> [opcode(u8)][format(u8)][reserved(u16)][Bx(u32)]
 *
 *
 * opcode selects which operation the instruction represents.
 *   each operation is defined to be one of the above layouts.
 *
 * format states what format the instruction is in, and
 *  what format the operands are in.
 *
 * A  is operand 1
 * Ax is operand 1, this is A e[x]tended
 * B  is operand 2
 * Bx is operand 2, this is B e[x]tended
 * C  is operand 3
 *
 *
 * format:
 *    8 7        6 5  4 3  2 1
 *  [reserved]   [C]  [B]  [I]
 *
 * I is the Instruction Format
 * B is how to interpret operand 2
 * C is how to interpret operand 3
 *
 */

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode {
  /*
   * <...> -> side effect
   * ip    -> the instruction pointer
   * R     -> the return value index
   * A|B|C -> an operand
   * L[*]  -> indexing the locals array
   * C[*]  -> indexing the constants array
   */
  OPC_RET, // B -- L[R] = B,    <return>
           // B -- L[R] = C[B], <return>
           // B -- L[R] = L[B], <return>

  OPC_MOVE, // AB  -- L[A] = B
            // AB  -- L[A] = C[B]
            // AB  -- L[A] = L[B]

  OPC_NEG, // AB  -- L[A] = -(B)
           // AB  -- L[A] = -(C[B])
           // AB  -- L[A] = -(L[B])

  OPC_ADD, // ABC -- L[A] = L[B] + L[C]
           // ABC -- L[A] = L[B] + C[C]
           // ABC -- L[A] = L[B] + C
           // ABC -- L[A] = C[B] + L[C]
           // ABC -- L[A] = C[B] + C[C]
           // ABC -- L[A] = C[B] + C
           // ABC -- L[A] = B    + L[C]
           // ABC -- L[A] = B    + C[C]
           // ABC -- L[A] = B    + C

  OPC_SUB, // ABC -- L[A] = L[B] - L[C]
           // ABC -- L[A] = L[B] - C[C]
           // ABC -- L[A] = L[B] - C
           // ABC -- L[A] = C[B] - L[C]
           // ABC -- L[A] = C[B] - C[C]
           // ABC -- L[A] = C[B] - C
           // ABC -- L[A] = B    - L[C]
           // ABC -- L[A] = B    - C[C]
           // ABC -- L[A] = B    - C

  OPC_MUL, // ABC -- L[A] = L[B] * L[C]
           // ABC -- L[A] = L[B] * C[C]
           // ABC -- L[A] = L[B] * C
           // ABC -- L[A] = C[B] * L[C]
           // ABC -- L[A] = C[B] * C[C]
           // ABC -- L[A] = C[B] * C
           // ABC -- L[A] = B    * L[C]
           // ABC -- L[A] = B    * C[C]
           // ABC -- L[A] = B    * C

  OPC_DIV, // ABC -- L[A] = L[B] / L[C]
           // ABC -- L[A] = L[B] / C[C]
           // ABC -- L[A] = L[B] / C
           // ABC -- L[A] = C[B] / L[C]
           // ABC -- L[A] = C[B] / C[C]
           // ABC -- L[A] = C[B] / C
           // ABC -- L[A] = B    / L[C]
           // ABC -- L[A] = B    / C[C]
           // ABC -- L[A] = B    / C

  OPC_MOD, // ABC -- L[A] = L[B] % L[C]
           // ABC -- L[A] = L[B] % C[C]
           // ABC -- L[A] = L[B] % C
           // ABC -- L[A] = C[B] % L[C]
           // ABC -- L[A] = C[B] % C[C]
           // ABC -- L[A] = C[B] % C
           // ABC -- L[A] = B    % L[C]
           // ABC -- L[A] = B    % C[C]
           // ABC -- L[A] = B    % C
} Opcode;

typedef enum InstructionFormat {
  IFMT_B   = 0x0,
  IFMT_AB  = 0x1,
  IFMT_ABC = 0x2,
} InstructionFormat;

typedef enum OperandFormat {
  OPRFMT_SSA       = 0x0,
  OPRFMT_CONSTANT  = 0x1,
  OPRFMT_IMMEDIATE = 0x2,
} OperandFormat;

typedef struct Operand {
  unsigned format : 2;
  unsigned common : 16;
} Operand;

Operand opr_constant(u16 index);
Operand opr_immediate(u16 imm);
Operand opr_ssa(u16 ssa);

/**
 * @brief represents a bytecode instruction
 *
 */
typedef struct Instruction {
  unsigned opcode   : 8;
  unsigned I_format : 2;
  unsigned Bfmt     : 2;
  unsigned Cfmt     : 2;
  unsigned          : 2;
  unsigned A        : 16;
  unsigned B        : 16;
  unsigned C        : 16;
} Instruction;

Instruction inst_B(Opcode opcode, Operand B);
Instruction inst_AB(Opcode opcode, Operand A, Operand B);
Instruction inst_ABC(Opcode opcode, Operand A, Operand B, Operand C);

#endif // !EXP_IMR_INSTRUCTION_H