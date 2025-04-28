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
 * @file imr/subject.h
 * @brief defines a struct which is simple a function, context
 * pointer pair. Useful whenever we need a context pointer and a
 * function pointer togther, such as when we are inferring types,
 * or validating bytecode.
 */

#ifndef EXP_IMR_SUBJECT_H
#define EXP_IMR_SUBJECT_H

#include "env/context.h"
#include "imr/function.h"

typedef struct Subject {
    Function *function;
    Context  *context;
} Subject;

#endif // EXP_IMR_SUBJECT_H
