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
#ifndef EXP_BACKEND_IR_X64_H
#define EXP_BACKEND_IR_X64_H

/*
  We need to model the problem of:
  -> instruction selection -> instruction scheduling -> register allocation ->

  we need to model the individual instructions and their semantics.

  that is:
  what are their valid inputs,
  what are their valid outputs,
  how long do the take to run,
  and anything else that is relevant.

  To accomplish this, we need something in memory to query against
  programatically.

  such that given a set of bytecode instructions we can construct
  a semantically equivalent set of x64 instructions (ISA).

  This is in essence "lowering" bytecode to x64 ISA.

  We can model the semantics of an instruction set as a directed acyclic graph
  (DAG).

  The problem of selecting instructions is represented in code as a DAG covering
  problem.

  which is, broadly, mapping a pattern of bytecode instructions to a pattern of
  x64 ISA DAG nodes.

  which is in general equivalent to subgraph isomorphism.
*/

/*
  specifically, we want to enumerate the x64 ISA.

  this is a large problem, so instead we will only enumerate the
  subset of the x64 ISA which is used to implement the programming
  language.

  and by enumerate I mean specify the inputs, outputs, costs, and
  constraints of each instruction considered by the instruction
  selector.


*/

#endif // !EXP_BACKEND_IR_X64_H