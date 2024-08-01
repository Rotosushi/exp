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
#include "imr/instruction.h"

Instruction instruction_B(Opcode opcode, Operand B) {
  Instruction I = {
      .opcode = opcode, .Ifmt = IFMT_B, .Bfmt = B.format, .B = B.common};
  return I;
}

Instruction instruction_AB(Opcode opcode, Operand A, Operand B) {
  Instruction I = {.opcode = opcode,
                   .Ifmt   = IFMT_AB,
                   .A      = A.common,
                   .Bfmt   = B.format,
                   .B      = B.common};
  return I;
}

Instruction instruction_ABC(Opcode opcode, Operand A, Operand B, Operand C) {
  Instruction I = {.opcode = opcode,
                   .Ifmt   = IFMT_ABC,
                   .A      = A.common,
                   .Bfmt   = B.format,
                   .B      = B.common,
                   .Cfmt   = C.format,
                   .C      = C.common};
  return I;
}