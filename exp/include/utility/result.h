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
#ifndef EXP_UTILITY_RESULT_H
#define EXP_UTILITY_RESULT_H

#include "utility/int_types.h"

typedef enum ExpResult : u32 {
    EXP_SUCCESS,
    EXP_FAILURE,
} ExpResult;

#endif // EXP_UTILITY_RESULT_H
