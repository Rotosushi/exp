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

/**
 * @brief represents a bytecode instruction
 *
 */
typedef u64 Instruction;

/*
 * These codes need to allow instructions to represent
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
 * ABC -> [opcode(u8)][format(u8)][A(u16)][B(u16)][C(u16)]
 * AB  -> [opcode(u8)][format(u8)][A(u16)][B(u16)][reserved(u16)]
 * ABx -> [opcode(u8)][format(u8)][A(u16)][Bx(u32)]
 * Ax  -> [opcode(u8)][format(u8)][Ax(u32)][reserved(u16)]
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
 *      7        6 5  4 3  2 1
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
   *
   *  ip    -> the instruction pointer
   * Ax|B|C -> an immediate
   * L[*]   -> indexing the locals array
   * C[*]   -> indexing the constants array
   */

  OPC_MOVE, // AB  -- L[A] = B
            // AB  -- L[A] = C[B]
            // AB  -- L[A] = L[B]
            // ABx -- L[A] = Bx
            // ABx -- L[A] = C[Bx]
            // ABx -- L[A] = L[Bx]

  OPC_NEG, // AB  -- L[A] = -(B)
           // AB  -- L[A] = -(C[B])
           // AB  -- L[A] = -(L[B])
           // ABx -- L[A] = -(Bx)
           // ABx -- L[A] = -(C[Bx])
           // ABx -- L[A] = -(L[Bx])

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
  FORMAT_ABC = 0x0,
  FORMAT_AB  = 0x1,
  FORMAT_ABx = 0x2,
  FORMAT_Ax  = 0x3,
} InstructionFormat;

typedef enum OperandFormat {
  FORMAT_LOCAL     = 0x0,
  FORMAT_CONSTANT  = 0x1,
  FORMAT_IMMEDIATE = 0x2,
} OperandFormat;

#define INST_OP(I) ((u8)((I) & u8_MAX))

#define INST_FORMAT_BYTE(I) ((u8)(((I) >> 8) & u8_MAX))
#define INST_FORMAT(I)      (INST_FORMAT_BYTE(I) & (u8)0x3)
#define INST_B_FORMAT(I)    ((INST_FORMAT_BYTE(I) >> 2) & (u8)0x3)
#define INST_C_FORMAT(I)    ((INST_FORMAT_BYTE(I) >> 4) & (u8)0x3)

#define INST_A(I)  ((u16)(((I) >> 16) & u16_MAX))
#define INST_B(I)  ((u16)(((I) >> 32) & u16_MAX))
#define INST_C(I)  ((u16)(((I) >> 48) & u16_MAX))
#define INST_Ax(I) ((u32)(((I) >> 16) & u32_MAX))
#define INST_Bx(I) ((u32)(((I) >> 32) & u32_MAX))

/*
  #NOTE:
  INST_SET_* works by first creating a temporary equal to the
  original I with the target bits cleared (bitwise and (&) with 0
  in the target position), then setting those bits equal to the
  value of the argument (bitwise or (|) with target bits set to the value).
  then setting the original value equal to the result.
*/

#define INST_SET_OP(I, op)                                                     \
  ((I) = (((I) & ~(u64)(u8_MAX)) | ((u64)(op) & (u64)(u8_MAX))))

#define INST_SET_FORMAT(I, f)                                                  \
  ((I) = (((I) & ~((u64)(u8_MAX) << 8)) |                                      \
          (((u64)(f) << 8) & ((u64)(u8_MAX) << 8))))

#define INST_SET_A(I, n)                                                       \
  ((I) = (((I) & ~((u64)(u16_MAX) << 16)) |                                    \
          (((u64)(n) << 16) & ((u64)(u16_MAX) << 16))))

#define INST_SET_B_FORMAT(I, f)                                                \
  ((I) = (((I) & ~((u64)(0x3) << 10)) |                                        \
          (((u64)(f) << 10) & ((u64)(0x3) << 10))))

#define INST_SET_C_FORMAT(I, f)                                                \
  ((I) = (((I) & ~((u64)(0x3) << 12)) |                                        \
          (((u64)(f) << 12) & ((u64)(0x3) << 12))))

#define INST_SET_B(I, n)                                                       \
  ((I) = (((I) & ~(((u64)(u16_MAX)) << 32)) |                                  \
          (((u64)(n) << 32) & (((u64)(u16_MAX)) << 32))))

#define INST_SET_C(I, n)                                                       \
  ((I) = (((I) & ~(((u64)(u16_MAX)) << 48)) |                                  \
          (((u64)(n) << 48) & (((u64)(u16_MAX)) << 48))))

#define INST_SET_Ax(I, n)                                                      \
  ((I) = (((I) & ~(((u64)(u32_MAX)) << 16)) |                                  \
          (((u64)(n) << 16) & (((u64)(u32_MAX)) << 16))))

#define INST_SET_Bx(I, n)                                                      \
  ((I) = (((I) & ~(((u64)(u32_MAX)) << 32)) |                                  \
          (((u64)(n) << 32) & (((u64)(u32_MAX)) << 32))))

#endif // !EXP_IMR_INSTRUCTION_H