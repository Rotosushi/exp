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
#ifndef EXP_BACKEND_X64_ALLOCATION_H
#define EXP_BACKEND_X64_ALLOCATION_H

#include "backend/x64_gpr.h"

typedef enum X64AllocationKind {
  ALLOC_GPR,
  ALLOC_STACK,
} X64AllocationKind;

typedef struct X64Allocation {
  X64AllocationKind kind;
  union {
    X64GPR gpr;
    u16 offset;
  };
} X64Allocation;

X64Allocation x64allocation_reg(X64GPR gpr);
X64Allocation x64allocation_stack(u16 offset);

#endif // !EXP_BACKEND_X64_ALLOCATION_H