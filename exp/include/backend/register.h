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
#ifndef EXP_BACKEND_REGISTER_H
#define EXP_BACKEND_REGISTER_H
#include "utility/int_types.h"
#include "utility/string_view.h"

/**
 * @brief enumerates the general purpose registers in x64
 *
 * @note we still need to take into account the overlapping
 * 8-bit, 16-bit, and 32-bit registers;
 * and the SSE floating point registers XMM[0..15] and YMM[0..15].
 * of particular note are the SSE registers which can also be used
 * to efficiently process large amounts of data.
 */
typedef enum Register {
  REG_RAX,
  REG_RBX,
  REG_RCX,
  REG_RDX,
  REG_RBP,
  REG_RSI,
  REG_RDI,
  REG_RSP,
  REG_R8,
  REG_R9,
  REG_R10,
  REG_R11,
  REG_R12,
  REG_R13,
  REG_R14,
  REG_R15,
  REG_NONE,
} Register;

StringView register_to_sv(Register r);

#endif // !EXP_BACKEND_REGISTER_H