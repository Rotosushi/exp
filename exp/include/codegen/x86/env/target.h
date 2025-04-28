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

/**
 * @brief defines x86_Target, providing hooks into x86 codegen
 * and target specific information. This allows specific targets
 * to provide implementation specific information in a uniform way.
 */

#ifndef EXP_CODEGEN_X86_TARGET_H
#define EXP_CODEGEN_X86_TARGET_H

#include "codegen/target.h"

typedef struct x86_Target {
    align_of_fn align_of;
    size_of_fn  size_of;
    codegen_fn  codegen;
    emit_fn     emit;
} x86_Target;

void x86_target_initialize(x86_Target *restrict target);

#endif // !EXP_CODEGEN_X86_TARGET_H
