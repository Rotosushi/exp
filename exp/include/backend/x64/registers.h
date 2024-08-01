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
#ifndef EXP_BACKEND_X64_GPR_H
#define EXP_BACKEND_X64_GPR_H

#include "utility/string_view.h"

/**
 * @brief General Purpose Register
 *
 */
typedef enum x64_GPR {
  X64GPR_RAX,
  X64GPR_RBX,
  X64GPR_RCX,
  X64GPR_RDX,
  X64GPR_RBP,
  X64GPR_RSI,
  X64GPR_RDI,
  X64GPR_RSP,
  X64GPR_R8,
  X64GPR_R9,
  X64GPR_R10,
  X64GPR_R11,
  X64GPR_R12,
  X64GPR_R13,
  X64GPR_R14,
  X64GPR_R15,
  X64GPR_R16,
  X64GPR_R17,
  X64GPR_R18,
  X64GPR_R19,
  X64GPR_R20,
  X64GPR_R21,
  X64GPR_R22,
  X64GPR_R23,
  X64GPR_R24,
  X64GPR_R25,
  X64GPR_R26,
  X64GPR_R27,
  X64GPR_R28,
  X64GPR_R29,
  X64GPR_R30,
  X64GPR_R31,
} x64_GPR;

StringView x64_gpr_to_sv(x64_GPR r);

#endif // !EXP_BACKEND_X64_GPR_H