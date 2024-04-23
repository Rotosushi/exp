/**
 * Copyright (C) 2024 Cade Weinberg
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
#include <assert.h>

#include "env/locals.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"

Locals locals_create() {
  Locals l;
  l.size     = 0;
  l.capacity = 0;
  l.locals   = NULL;
  return l;
}

void locals_destroy(Locals *restrict l) {
  assert(l != NULL);
  l->size     = 0;
  l->capacity = 0;
  free(l->locals);
  l->locals = NULL;
}

static bool locals_full(Locals *restrict l) {
  return (l->size + 1) >= l->capacity;
}

static void locals_grow(Locals *restrict l) {
  Growth g    = array_growth(l->capacity, sizeof(Value));
  l->locals   = reallocate(l->locals, g.alloc_size);
  l->capacity = g.new_capacity;
}

/**
 * @brief compute the offset into the locals buffer equal to
 * the passed frame pointer
 *
 * @param l
 * @param frame
 * @return u64
 */
static u64 frame_offset(Locals *restrict l, Frame frame) {
  // frame points somewhere >= the beginning of the buffer
  return (u64)(frame - l->locals);
}

/**
 * @brief compute the number of active locals within the
 * current frame.
 *
 * @param l
 * @param frame
 * @return u64
 */
static u64 frame_size(Locals *restrict l, Frame frame) {
  // the size of the current frame is the number of
  // locals pushed within the given frame.

  // the frame_offset is equal to the number of active
  // locals which are not part of the frame.
  u64 foff = frame_offset(l, frame);
  // l->size is the total number of locals active in all frames.
  // which must be >= frame_offset.
  // so the difference between the total number of locals
  // and the number of locals not part of the frame must
  // be equal to the number of locals within the given frame.
  return (u64)(l->size - foff);
}

/**
 * @brief The frame is inbounds of the locals buffer if it is >= the beginning
 * of the buffer and < the size of the locals buffer
 *
 * @note this is [[maybe_unused]] because it is only called in
 * assert statements, meaning that during a release build this
 * function will not be called.
 *
 * @param l
 * @param frame
 * @return true
 * @return false
 */
[[maybe_unused]] static bool frame_inbounds(Locals *restrict l, Frame frame) {
  return (frame >= l->locals) && (frame < (l->locals + l->capacity));
}

/**
 * @brief the given index is within the given frame if the index is < the size
 * of the given frame.
 *
 * @note this is [[maybe_unused]] because it is only called in
 * assert statements, meaning that during a release build this
 * function will not be called.
 *
 * @param l
 * @param frame
 * @param index
 * @return true
 * @return false
 */
[[maybe_unused]] static bool local_in_frame(Locals *restrict l, Frame frame,
                                            u16 index) {
  return index < frame_size(l, frame);
}

// we increase the total size of the locals buffer by 1.
// This "allocates" a new local. We then return the
// offset of that new local relative to the given frame pointer.
u16 locals_new_local(Locals *restrict l, Frame f) {
  assert(l != NULL);
  assert(frame_inbounds(l, f));
  // if there is no space for one more local,
  // grow the locals buffer.
  if (locals_full(l)) {
    locals_grow(l);
  }

  // (f + frame_size) is one past the end of the local frame
  // and because we are operating on locals in a call stack,
  // it must be equivalent to (l->locals + l->size).
  // so in order to "allocate" that new local, all we need to
  // do is increment l->size;
  u64 absolute_offset = l->size++;

  // the frame relative offset is equal to the absolute
  // offset minus the frame offset
  u64 relative_offset = absolute_offset - frame_offset(l, f);

  // we can only reference locals in instructions with
  // a maximum offset of a single u16. Since we access
  // locals relative to a frame pointer, we can still
  // allocate a full u64's worth of local variables
  // accross all active frames. a single frame cannot be
  // larger than a u16 however.
  if (relative_offset > u16_MAX) {
    PANIC("local offset out of bounds");
  }

  return (u16)relative_offset;
}

Value *locals_at(Locals *restrict l, Frame f, u16 i) {
  assert(l != NULL);
  assert(frame_inbounds(l, f));
  assert(local_in_frame(l, f, i));

  // since the frame is inbounds, and the index of the local
  // is within the frame given, we can be sure that f[i] is the
  // value of the local at i. since we return a pointer, we
  // just do a little pointer addition.
  return f + i;
}

// the new frame of locals starts at the end of the
// current local buffer.
// This is consistent
// with a call stack because any call frame below
// this one is untouched (it must end at the end of
// the current size, because every "active" frame
// must start somewhere within the buffer and continue
// until the end of the buffer) and there cannot be
// any frames above this one on the stack, because we
// are adding a new frame to the top of the stack.
Frame locals_push_frame(Locals *restrict l) {
  assert(l != NULL);
  if (locals_full(l)) {
    locals_grow(l);
  }

  return l->locals + l->size;
}

// to 'pop' the current frame we subtract
// the current frame size from the size of the
// locals array.
// This is consistent with a call stack,
// because any frames below this one
// remain untouched (they must end right before
// the beginning of this frame), and there
// cannot be any frames above this one, because
// we are poping the top.
void locals_pop_frame(Locals *restrict l, Frame f) {
  assert(l != NULL);
  assert(frame_inbounds(l, f));

  u64 fsz = frame_size(l, f);
  l->size -= fsz;
}