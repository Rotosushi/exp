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
#include <stdlib.h>
#include <time.h>

#include "imr/instruction.h"
#include "utility/debug.h"

bool test_ABC() {
  u8 op = (u8)(rand() & u8_MAX);
  u8 If = (u8)(rand() & 0x3);
  u16 A = (u16)(rand() & u16_MAX);
  u16 B = (u16)(rand() & u16_MAX);
  u8 Bf = (u8)(rand() & 0x3);
  u16 C = (u16)(rand() & u16_MAX);
  u8 Cf = (u8)(rand() & 0x3);

  Instruction I = 0;

  INST_SET_OP(I, op);
  INST_SET_FORMAT(I, If);
  INST_SET_B_FORMAT(I, Bf);
  INST_SET_C_FORMAT(I, Cf);
  INST_SET_A(I, A);
  INST_SET_B(I, B);
  INST_SET_C(I, C);

  u8 op_ = INST_OP(I);
  u8 If_ = INST_FORMAT(I);
  u8 Bf_ = INST_B_FORMAT(I);
  u8 Cf_ = INST_C_FORMAT(I);
  u16 A_ = INST_A(I);
  u16 B_ = INST_B(I);
  u16 C_ = INST_C(I);

  bool failed = 0;
  failed |= op != op_;
  failed |= If != If_;
  failed |= Bf != Bf_;
  failed |= Cf != Cf_;
  failed |= A != A_;
  failed |= B != B_;
  failed |= C != C_;
  return failed;
}

bool test_AB() {
  u8 op = (u8)(rand() % u8_MAX);
  u16 A = (u16)(rand() % u16_MAX);
  u16 B = (u16)(rand() % u16_MAX);

  Instruction I = 0;

  INST_SET_OP(I, op);
  INST_SET_A(I, A);
  INST_SET_B(I, B);

  u8 op_ = INST_OP(I);
  u16 A_ = INST_A(I);
  u16 B_ = INST_B(I);

  bool failed = 0;
  failed |= op != op_;
  failed |= A != A_;
  failed |= B != B_;
  return failed;
}

bool test_ABx() {
  u8 op  = (u8)(rand() % u8_MAX);
  u16 A  = (u16)(rand() % u16_MAX);
  u32 Bx = (u32)rand();

  Instruction I = 0;

  INST_SET_OP(I, op);
  INST_SET_A(I, A);
  INST_SET_Bx(I, Bx);

  u8 op_  = INST_OP(I);
  u16 A_  = INST_A(I);
  u32 Bx_ = INST_Bx(I);

  bool failed = 0;
  failed |= op != op_;
  failed |= A != A_;
  failed |= Bx != Bx_;
  return failed;
}

bool test_Ax() {
  u8 op  = (u8)(rand() % u8_MAX);
  u32 Ax = (u32)rand();

  Instruction I = 0;

  INST_SET_OP(I, op);
  INST_SET_Ax(I, Ax);

  u8 op_  = INST_OP(I);
  u32 Ax_ = INST_Ax(I);

  bool failed = 0;
  failed |= op != op_;
  failed |= Ax != Ax_;
  return failed;
}

int instruction_tests([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  srand((u32)time(NULL));
  bool failed = 0;

  failed |= test_ABC();
  failed |= test_AB();
  failed |= test_ABx();
  failed |= test_Ax();

  if (failed) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}