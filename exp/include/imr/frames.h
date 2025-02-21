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
 * @file imr/frames.h
 */

#ifndef EXP_IMR_FRAMES_H
#define EXP_IMR_FRAMES_H

#include "imr/frame.h"

typedef struct Frames {
    u32 length;
    u32 capacity;
    Frame *buffer;
} Frames;

void frames_initialize(Frames *frames);
void frames_terminate(Frames *frames);

Frame *frames_top(Frames *frames);
Frame *frames_push(Frames *frames, Function *function, u32 base);
void frames_pop(Frames *frames);

#endif // !EXP_IMR_FRAMES_H
