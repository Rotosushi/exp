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
#ifndef EXP_IMR_LOCALS_H
#define EXP_IMR_LOCALS_H

#include "imr/value.h"
#include "utility/io.h"

typedef Value *Frame;

/**
 * @brief represents the local variables of functions
 *
 */
typedef struct Locals {
  u64 size;
  u64 capacity;
  Value *locals;
} Locals;

Locals locals_create();
void locals_destroy(Locals *restrict l);

/**
 * @brief add a new local to the given frame
 *
 * @param l
 * @param frame
 * @return u32 the index of the new local relative to <frame>
 */
u16 locals_new_local(Locals *restrict l, Frame frame);

/**
 * @brief return the i'th local of the given frame
 *
 * @warning it is undefined behavior to access locals
 * of frames which are not the top of the call stack
 *
 * @param l
 * @param frame
 * @param i
 * @return Value*
 */
Value *locals_at(Locals *restrict l, Frame frame, u16 i);

/**
 * @brief push a new function frame pointer
 *
 * @warning This function returns a new frame pointer
 * which is the new top of the call stack; after this call
 * it is undefined behavior to add new locals to call frames
 * below this one.
 *
 * @param l
 * @return Value*
 */
Frame locals_push_frame(Locals *restrict l);

/**
 * @brief pop all locals past the begin frame pointer
 *
 * @warning This function assumes that <frame> is the
 * top frame of the call stack. Thus every local allocated
 * between the passed frame and the end is "poped" from
 * the call stack. if you pass a frame below the top,
 * every frame above it will be poped as well.
 *
 * @param l
 * @param frame
 */
void locals_pop_frame(Locals *restrict l, Frame frame);

void print_locals(Locals const *restrict l, FILE *restrict file);

#endif // !EXP_IMR_LOCALS_H