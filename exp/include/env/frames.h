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
 * @file env/frames.h
 */

#ifndef EXP_ENV_FRAMES_H
#define EXP_ENV_FRAMES_H

#include "env/frame.h"

typedef struct Frames {
    u32    size;
    u32    capacity;
    Frame *buffer;
} Frames;

void frames_create(Frames *restrict frames);
void frames_destroy(Frames *restrict frames);

bool frames_empty(Frames const *restrict frames);

void   frames_push(Frames *restrict frames, Frame frame);
Frame *frames_top(Frames const *restrict frames);
void   frames_pop(Frames *restrict frames);

#endif // !EXP_ENV_FRAMES_H
