/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file imr/frames.c
 */

#include "imr/frames.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"

void frames_initialize(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    frames->length   = 0;
    frames->capacity = 0;
    frames->buffer   = nullptr;
}
void frames_terminate(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    deallocate(frames->buffer);
    frames_initialize(frames);
}

static bool frames_full(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    return (frames->length + 1) >= frames->capacity;
}

static void frames_grow(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    Growth32 g       = array_growth_u32(frames->capacity, sizeof(Frame));
    frames->buffer   = reallocate(frames->buffer, g.alloc_size);
    frames->capacity = g.new_capacity;
}

Frame *frames_top(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    EXP_ASSERT(frames->length > 0);
    return &frames->buffer[frames->length - 1];
}

Frame *frames_push(Frames *frames, Function *function, u32 base) {
    EXP_ASSERT(frames != nullptr);
    if (frames_full(frames)) { frames_grow(frames); }
    Frame *frame = &frames->buffer[frames->length++];
    frame_initialize(frame, function, base);
    return frame;
}

void frames_pop(Frames *frames) {
    EXP_ASSERT(frames != nullptr);
    EXP_ASSERT(frames->length > 0);
    --frames->length;
}
