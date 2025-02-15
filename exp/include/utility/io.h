// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_IO_H
#define EXP_UTILITY_IO_H

// #include <stdio.h>

#include "utility/string_view.h"

typedef struct File {
    i32 descriptor;
} File;

extern struct File *program_input;
extern struct File *program_output;
extern struct File *program_error;

typedef enum FileModes {
    FILEMODE_READ,
    FILEMODE_WRITE,
    FILEMODE_READWRITE,
} FileModes;

File file_open(StringView path, FileModes modes);
void file_close(File *file);

void file_remove(StringView path);

/**
 * @brief write <buffer> to <stream>
 *
 * @note if the write fails, an error is printed to stderr
 * and exit is called
 *
 * @param buffer the buffer to write
 * @param length the length of the buffer
 * @param stream the stream to write to
 * @return u64 the number of chars written
 */
void file_write(StringView string, File *file);
void file_write_i64(i64 value, File *file);
void file_write_u64(u64 value, File *file);

/**
 * @brief read <length> chars into <buffer> from <stream>
 *
 * @param buffer the buffer to store chars into
 * @param length the number of chars to read
 * @param stream the stream to read from.
 * @return u64 the number of chars actually read.
 */
u64 file_read(char *buffer, u64 length, File *file);

u64 file_length(File *file);

#endif // !EXP_UTILITY_IO_H
