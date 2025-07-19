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
#ifndef EXP_CODEGEN_X86_INTRINSICS_PTR_KIND_OF_H
#define EXP_CODEGEN_X86_INTRINSICS_PTR_KIND_OF_H

#include "codegen/x86/imr/allocation/location.h"
#include "imr/layout.h"

x86_PtrKind x86_ptr_kind_of(Layout const *restrict layout);

#endif // !EXP_CODEGEN_X86_INTRINSICS_PTR_KIND_OF_H
