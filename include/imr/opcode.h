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

  definitions
    - variables
    - constants
    - functions

  expressions
    - constants
    - references
    - calls
    - unary operators
    - binary operators

  if we assume that instructions are an opcode
  followed by their arguments, layed out according
  to the opcode.
  and we have a array of constants C[N]
  and we have a array of types T[N]
  and we have a array of registers R[N]
  and we have a stack of values S[N]
*/

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode {
  // for the sake of brevity, '[]' is referring to one byte

  // OP_PUSH_CONSTANT
  // [op][A]
  // push a constant onto the stack
  //  C[A] is the source constant
  OP_PUSH_CONSTANT,

  // OP_PUSH_REGISTER
  // [op][A]
  // push a register onto the stack
  // R[A] is the source register
  OP_PUSH_REGISTER,

  // OP_POP
  // [op]
  // pop the top value off the stack
  OP_POP,

  // OP_POP_REGISTER
  // [op][A]
  // pop the top value off the stack,
  // and store it into a register
  // R[A] is the target register
  OP_POP_REGISTER,

  // OP_MOVE_CONSTANT_TO_REGISTER
  // [op][A][B]
  // Move a constant into a register
  // R[A] is the target register
  // C[B] is the source constant
  OP_MOVE_CONSTANT_TO_REGISTER,

  // OP_DEFINE_GLOBAL_CONSTANT
  // [op][A][B][C]
  // define a global symbol
  //  R[A] is the name of the global
  //  R[B] is the type of the global
  //  R[C] is the value of the global
  OP_DEFINE_GLOBAL_CONSTANT,

  // #TODO
  // OP_DEFINE_FN

  // #TODO:
  // OP_LOAD,
  // OP_LOADI,
  // OP_LOADC,

  // #TODO:
  //  OP_STORE,
  //  OP_STOREI,

  // #TODO:
  // OP_CALL,
  // OP_RETURN,

  // #TODO:
  // OP_UOP_NEG,

  // #TODO:
  // OP_BOP_SUB,
  // OP_BOP_ADD,
  // OP_BOP_MUL,
  // OP_BOP_DIV,
  // OP_BOP_MOD,
} Opcode;

#endif // !EXP_IMR_OPCODE_H