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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <stdio.h>

/**
 * @brief write <buffer> to <stream>
 *
 * @note if the write fails, an error is printed to stderr
 * and exit is called
 *
 * @param buffer the buffer to write
 * @param length the length of the buffer
 * @param stream the stream to write to
 * @return size_t the number of chars written
 */
size_t file_write(const char *restrict buffer, size_t length,
                  FILE *restrict stream);