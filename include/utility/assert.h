// Copyright (C) 2024 cade
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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include "utility/config.h"

#if defined(__clang__) || defined(__GNUC__)
#define EXP_BREAK() __builtin_trap()
#else
#error "Unsupported Host OS"
#endif

#if defined(EXP_DEBUG)
#include