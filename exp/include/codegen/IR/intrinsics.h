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

#ifndef EXP_CODEGEN_IR_INTRINSICS_H
#define EXP_CODEGEN_IR_INTRINSICS_H

#include "imr/type.h"

u64 ir_size_of(Type const *restrict type);
u64 ir_align_of(Type const *restrict type);

#endif // !EXP_CODEGEN_IR_INTRINSICS_H
