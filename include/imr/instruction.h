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
  all instructions are defined to have one of the
  following formats:

  1 -> [opcode(u8)][M(u8)][Ax(u32)][reserved(u16)]
  2 -> [opcode(u8)][M(u8)][A(u16)][Bx(u32)]
  3 -> [opcode(u8)][M(u8)][A(u16)][B(u16)][reserved(u16)]
  4 -> [opcode(u8)][M(u8)][A(u16)][B(u16)][C(u16)]

  opcode selects which operation the instruction represents.
    each operation is defined to be one of the above formats.
  M (for [M]etadata) says which operands are immediate or register.
  A is operand 1, usually this will be the destination operand.
  B is operand 2, usually this will be a source operand.
  C is operand 3, usually this will be a source operand.
  Ax is operand 1, this is A e[x]tended
  Bx is operand 2, this is B e[x]tended
*/

#define INST_OP(I) ((u8)((I) & u8_MAX))
#define INST_M(I)  ((u8)(((I) >> 8) & u8_MAX))

#define INST_A_IS_IMMEDIATE(I) (INST_M(I) & (u8)1)
#define INST_B_IS_IMMEDIATE(I) ((INST_M(I) >> 1) & (u8)1)
#define INST_C_IS_IMMEDIATE(I) ((INST_M(I) >> 2) & (u8)1)

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
*/

#define INST_SET_OP(I, op)                                                     \
  ((I) = (((I) & ~((u64)(u8_MAX))) | (((u64)(op)) & ((u64)(u8_MAX)))))

#define INST_SET_A_IS_IMMEDIATE(I) ((I) = ((I) | ((u64)1 << 9)))
#define INST_SET_B_IS_IMMEDIATE(I) ((I) = ((I) | ((u64)1 << 10)))
#define INST_SET_C_IS_IMMEDIATE(I) ((I) = ((I) | ((u64)1 << 11)))

#define INST_SET_A(I, n)                                                       \
  ((I) = (((I) & ~(((u64)(u16_MAX)) << 16)) |                                  \
          (((u64)(n) << 16) & (((u64)(u16_MAX)) << 16))))

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