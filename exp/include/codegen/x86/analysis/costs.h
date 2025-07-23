// Copyright (C) 2025 Cade Weinberg
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
#ifndef EXP_CODEGEN_X86_ANALYSIS_COSTS_H
#define EXP_CODEGEN_X86_ANALYSIS_COSTS_H

/**
 * each instruction in x86-64 has costs associated with it.
 * and each variant of that instruction has a different set of
 * costs. and each specific processor has different resources
 * available to  handle each particular instruction.
 * - CPU execution cycles for the opcode + operands
 * - memory access vs register access
 * - cached vs uncached instruction
 * - cached vs uncached data
 * - pipeline slots used (pipeline pressure)
 * - registers used (register pressure)
 * - data dependencies between instructions
 *
 * - What is better for comparing the cost between two operations?
 *   floats, or integers?
 *
 * - CPU execution cycles can potentially be modeled as a single integer
 *   in the cases where the opcode + operands are the most
 * standard|basic|primitive. such as `add <reg>, <reg>` however, in order to
 * account for the potential for an uncached instruction, the model must be
 * given as a range, [low, high]. where the low end is when the instruction is
 * already present within the cache, and the high end is when the instruction
 * needs to be fetched from main memory. And when one of the operands is a
 * memory reference, we must account for that data being present in the cache or
 * not.
 *
 */

#endif // !EXP_CODEGEN_X86_ANALYSIS_COSTS_H
