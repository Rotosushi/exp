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
 * @file imr/frame.h
 */

#ifndef EXP_IMR_FRAME_H
#define EXP_IMR_FRAME_H

#include "imr/function.h"
#include "imr/instruction.h"

typedef struct Frame {
    Function *function;
    Instruction *ip;
    u32 base;
    u32 length;
} Frame;

void frame_initialize(Frame *frame, Function *function, u32 base);
void frame_extend(Frame *frame, u32 n);

#endif // !EXP_IMR_FRAME_H
