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
  // OP_STOP
  // [op]
  // stop the interpretation of bytecode
  OP_STOP,

  // OP_PUSH_CONSTANT_U8
  // [op][A]
  // push a constant onto the stack
  //  C[A] is the source constant
  OP_PUSH_CONSTANT_U8,
  // OP_PUSH_CONSTANT_U*
  // [op][A][A][A][A] [A][A][A][A]
  // the same with a larger immediate index.
  OP_PUSH_CONSTANT_U16,
  OP_PUSH_CONSTANT_U32,
  OP_PUSH_CONSTANT_U64,

  // OP_POP
  // [op]
  // pop the top value off the stack
  OP_POP,

  // OP_DEFINE_GLOBAL_CONSTANT
  // [op]
  // define a global symbol
  // S[-1] is the value of the constant
  // S[-2] is the name of the constant
  OP_DEFINE_GLOBAL_CONSTANT,

  // OP_UNOP_MINUS
  // [op]
  // negate the top value of the stack
  OP_UNOP_MINUS,

  // OP_BINOP_PLUS
  // [op]
  // add the top two values of the stack
  OP_BINOP_PLUS,

  // OP_BINOP_MINUS
  // [op]
  // subtract the top value of the stack from
  // the next value on the stack
  OP_BINOP_MINUS,

  // OP_BINOP_STAR
  // [op]
  // mutliply the top two values of the stack
  OP_BINOP_STAR,

  // OP_BINOP_SLASH
  // [op]
  // divide the second from the top value of the stack
  // by the top value of the stack
  OP_BINOP_SLASH,

  // OP_BINOP_PERCENT
  // [op]
  // divide the second from the top value of the stack
  // by the top value of the stack, returing the remainder
  OP_BINOP_PERCENT,

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
} Opcode;

#endif // !EXP_IMR_OPCODE_H