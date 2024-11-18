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
typedef enum x64_GPR : u8 {
    X64_GPR_RAX,
    X64_GPR_RBX,
    X64_GPR_RCX,
    X64_GPR_RDX,
    X64_GPR_RBP,
    X64_GPR_RSI,
    X64_GPR_RDI,
    X64_GPR_RSP,
    X64_GPR_R8,
    X64_GPR_R9,
    X64_GPR_R10,
    X64_GPR_R11,
    X64_GPR_R12,
    X64_GPR_R13,
    X64_GPR_R14,
    X64_GPR_R15,
} x64_GPR;

StringView x64_gpr_to_sv(x64_GPR r);
x64_GPR x64_scalar_argument_gpr(u8 index);

#endif // !EXP_BACKEND_X64_GPR_H
