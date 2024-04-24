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
#ifndef EXP_BACKEND_REGISTER_ALLOCATOR_H
#define EXP_BACKEND_REGISTER_ALLOCATOR_H

#include "backend/register.h"

/*
The target algorithm is Linear Scan Register Allocation (LSRA)
The goal of the algorithm is to allocate the local variables
and temporary variables within a function to specific registers,
such that the function body can be mapped to target assembly
instructions.

in pseudo-code:

LinearScanRegisterAllocation
    active ← {}
    for each live interval i, in order of increasing start point do
        ExpireOldIntervals(i)
        if length(active) = R then
            SpillAtInterval(i)
        else
            register[i] ← a register removed from pool of free registers
            add i to active, sorted by increasing end point

ExpireOldIntervals(i)
    for each interval j in active, in order of increasing end point do
        if endpoint[j] ≥ startpoint[i] then
            return
        remove j from active
        add register[j] to pool of free registers

SpillAtInterval(i)
    spill ← last interval in active
    if endpoint[spill] > endpoint[i] then
        register[i] ← register[spill]
        location[spill] ← new stack location
        remove spill from active
        add i to active, sorted by increasing end point
    else
        location[i] ← new stack location

-----

  So we need three major pieces:
    - a pool of registers which can be marked active/inactive
    - a pool of stack space, where SSA locals can be spilled/filled
    - a set of liveness ranges for each SSA local

  The LSRA needs to take into account a few more things:
    - function arguments have defined registers which they take up,
      which affect the location a SSA local must be in, in the future.
    - certain instructions on the target architecture have defined
      registers which are their operands, which affects the location
      of the corresponding SSA locals
  This can be done by a pre-allocation step, which runs before
  the main allocation algorithm.
*/

#endif // !EXP_BACKEND_REGISTER_ALLOCATOR_H