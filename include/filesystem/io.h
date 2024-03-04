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
#ifndef EXP_FILESYSTEM_IO_H
#define EXP_FILESYSTEM_IO_H

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
void file_write(const char *restrict buffer, FILE *restrict stream);

/**
 * @brief read <length> chars into <buffer> from <stream>
 *
 * @param buffer the buffer to store chars into
 * @param length the number of chars to read
 * @param stream the stream to read from.
 * @return size_t the number of chars actually read.
 */
size_t file_read(char *buffer, size_t length, FILE *restrict stream);

#endif // !EXP_FILESYSTEM_IO_H