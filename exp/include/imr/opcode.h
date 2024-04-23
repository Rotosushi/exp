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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_IMR_OPCODE_H
#define EXP_IMR_OPCODE_H

/*
  These codes need to allow instructions to represent
  expressions
    - loads
    - stores
    - unops
    - binops
    - calls
*/

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode {
  OPC_LOADI, // A Bx  -- L[A] = Bx
  OPC_NEG,   // A B   -- L[A] = -L[B]
  OPC_ADD,   // A B C -- L[A] = L[B] + L[C]
  OPC_SUB,   // A B C -- L[A] = L[B] - L[C]
  OPC_MUL,   // A B C -- L[A] = L[B] * L[C]
  OPC_DIV,   // A B C -- L[A] = L[B] / L[C]
  OPC_MOD,   // A B C -- L[A] = L[B] % L[C]
} Opcode;

#endif // !EXP_IMR_OPCODE_H