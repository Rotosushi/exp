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

/*
  So, the big question for a call instruction is how to we elaborate
  what the arguments to the call instruction are within the space provided
  by the instruction format?

  we only have 3 operands to the instruction A, B, C, (each being 16 bits)
  and each argument to the function is as complex as an entire operand.
  so how do we support function calls with [n] potential arguments?

  my first guess is to specify a range of locals using the operands B, C.
  and specify the function to call using operand A.
  this leaves out how to specify the returned value.
  so we need at least 4 operands? and if we are specifying a range of ssa
  locals, we need to at least be able to start and stop the range anywhere.
  so doesn't that mean we need at least two u16's? which means that with this
  approach we need 4 u16's to properly encode. (and this leaves out the problem
  of getting the arguments into a consecutive range of ssa locals.)

  the call instruction needs to layout like:
  [f][R][B][C] -> SSA[R] = f(SSA[B], ..., SSA[C])

  what if we specify the argument list in another table somehow? and we
  then just need to store a key in the instruction which can be used to
  retrieve the argument list. just as we did with the global variable names.
  then it can be
  ABC -- SSA[A] = <B>(<C>)
*/

/**
 * @brief the valid opcodes for instructions
 *
 */
typedef enum Opcode {
  /*
   * <...> -> side effect
   * ip    -> the instruction pointer
   * R     -> the return value location
   * A|B|C -> an operand
   * SSA[*]           -> indexing the locals array.
   * Constants[*]     -> indexing the constants array.
   * GlobalSymbol[*]  -> indexing the global names array followed by
   *          indexing the global symbol table.
   * Calls[*]         -> indexing the actual argument lists array.
   */
  OPC_RET, // B -- R = B,    <return>
           // B -- R = Constants[B], <return>
           // B -- R = SSA[B], <return>

  OPC_CALL, // ABC -- SSA[A] = GlobalSymbol[B](Calls[C])

  OPC_LOAD, // AB  -- SSA[A] = B
            // AB  -- SSA[A] = Constants[B]
            // AB  -- SSA[A] = SSA[B]

  OPC_NEG, // AB  -- SSA[A] = -(B)
           // AB  -- SSA[A] = -(Constants[B])
           // AB  -- SSA[A] = -(SSA[B])

  OPC_ADD, // ABC -- SSA[A] = SSA[B] + SSA[C]
           // ABC -- SSA[A] = SSA[B] + Constants[C]
           // ABC -- SSA[A] = SSA[B] + C
           // ABC -- SSA[A] = Constants[B] + SSA[C]
           // ABC -- SSA[A] = Constants[B] + Constants[C]
           // ABC -- SSA[A] = Constants[B] + C
           // ABC -- SSA[A] = B    + SSA[C]
           // ABC -- SSA[A] = B    + Constants[C]
           // ABC -- SSA[A] = B    + C

  OPC_SUB, // ABC -- SSA[A] = SSA[B] - SSA[C]
           // ABC -- SSA[A] = SSA[B] - Constants[C]
           // ABC -- SSA[A] = SSA[B] - C
           // ABC -- SSA[A] = Constants[B] - SSA[C]
           // ABC -- SSA[A] = Constants[B] - Constants[C]
           // ABC -- SSA[A] = Constants[B] - C
           // ABC -- SSA[A] = B    - SSA[C]
           // ABC -- SSA[A] = B    - Constants[C]
           // ABC -- SSA[A] = B    - C

  OPC_MUL, // ABC -- SSA[A] = SSA[B] * SSA[C]
           // ABC -- SSA[A] = SSA[B] * Constants[C]
           // ABC -- SSA[A] = SSA[B] * C
           // ABC -- SSA[A] = Constants[B] * SSA[C]
           // ABC -- SSA[A] = Constants[B] * Constants[C]
           // ABC -- SSA[A] = Constants[B] * C
           // ABC -- SSA[A] = B    * SSA[C]
           // ABC -- SSA[A] = B    * Constants[C]
           // ABC -- SSA[A] = B    * C

  OPC_DIV, // ABC -- SSA[A] = SSA[B] / SSA[C]
           // ABC -- SSA[A] = SSA[B] / Constants[C]
           // ABC -- SSA[A] = SSA[B] / C
           // ABC -- SSA[A] = Constants[B] / SSA[C]
           // ABC -- SSA[A] = Constants[B] / Constants[C]
           // ABC -- SSA[A] = Constants[B] / C
           // ABC -- SSA[A] = B    / SSA[C]
           // ABC -- SSA[A] = B    / Constants[C]
           // ABC -- SSA[A] = B    / C

  OPC_MOD, // ABC -- SSA[A] = SSA[B] % SSA[C]
           // ABC -- SSA[A] = SSA[B] % Constants[C]
           // ABC -- SSA[A] = SSA[B] % C
           // ABC -- SSA[A] = Constants[B] % SSA[C]
           // ABC -- SSA[A] = Constants[B] % Constants[C]
           // ABC -- SSA[A] = Constants[B] % C
           // ABC -- SSA[A] = B    % SSA[C]
           // ABC -- SSA[A] = B    % Constants[C]
           // ABC -- SSA[A] = B    % C
} Opcode;

typedef enum InstructionFormat {
  IFMT_B,
  IFMT_AB,
  IFMT_ABC,
} InstructionFormat;

typedef enum OperandFormat {
  OPRFMT_SSA,
  OPRFMT_CONSTANT,
  OPRFMT_IMMEDIATE,
  OPRFMT_GLOBAL,
} OperandFormat;

typedef struct Operand {
  unsigned format : 3;
  unsigned common : 16;
} Operand;

Operand opr_constant(u16 index);
Operand opr_immediate(u16 imm);
Operand opr_ssa(u16 ssa);
Operand opr_global(u16 idx);

/*
  #TODO if we ever need to store more than
  a u16's worth of constants, or ssa's, or
  global variable names, we will need extended
  versions of instructions
*/

/**
 * @brief represents a bytecode instruction
 */
typedef struct Instruction {
  unsigned opcode   : 7;
  unsigned I_format : 3;
  unsigned Bfmt     : 3;
  unsigned Cfmt     : 3;
  unsigned A        : 16;
  unsigned B        : 16;
  unsigned C        : 16;
} Instruction;

Instruction inst_B(Opcode opcode, Operand B);
Instruction inst_AB(Opcode opcode, Operand A, Operand B);
Instruction inst_ABC(Opcode opcode, Operand A, Operand B, Operand C);

#endif // !EXP_IMR_INSTRUCTION_H