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

#include "env/frames.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"

void frames_create(Frames *restrict frames) {
    exp_assert(frames != NULL);
    frames->size     = 0;
    frames->capacity = 0;
    frames->buffer   = NULL;
}

void frames_destroy(Frames *restrict frames) {
    exp_assert(frames != NULL);

    deallocate(frames->buffer);
    frames_create(frames);
}

bool frames_empty(Frames const *restrict frames) {
    exp_assert(frames != NULL);
    return frames->size == 0;
}

static bool frames_full(Frames const *restrict frames) {
    return (frames->size + 1) > frames->capacity;
}

static void frames_grow(Frames *restrict frames) {
    Growth_u32 g = array_growth_u32(frames->capacity, sizeof(*frames->buffer));
    frames->buffer   = reallocate(frames->buffer, g.alloc_size);
    frames->capacity = g.new_capacity;
}

void frames_push(Frames *restrict frames, Frame frame) {
    exp_assert(frames != NULL);
    exp_assert(frame.function != NULL);
    if (frames_full(frames)) { frames_grow(frames); }
    frames->buffer[frames->size++] = frame;
}

Frame *frames_top(Frames const *restrict frames) {
    exp_assert(frames != NULL);
    exp_assert(!frames_empty(frames));
    return frames->buffer + (frames->size - 1);
}

void frames_pop(Frames *restrict frames) {
    exp_assert(frames != NULL);
    frames->size -= 1;
}
