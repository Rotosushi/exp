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
#ifndef EXP_TARGETS_X86_64_INSTRUCTIONS_PUSH_H
#define EXP_TARGETS_X86_64_INSTRUCTIONS_PUSH_H

#include "targets/x86_64/operand.h"

void x86_64_push(String *buffer, x86_64_Operand target);

#endif // EXP_TARGETS_X86_64_ISNTRUCTIONS_PUSH_H
