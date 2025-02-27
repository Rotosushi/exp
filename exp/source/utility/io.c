/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <string.h>

#include "utility/assert.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

#ifndef EXP_HOST_SYSTEM_LINUX
    #error "unsupported host OS"
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

File program_input_impl  = {.descriptor = 0};
File program_output_impl = {.descriptor = 1};
File program_error_impl  = {.descriptor = 2};

File *program_input  = &program_input_impl;
File *program_output = &program_output_impl;
File *program_error  = &program_output_impl;

File file_open(StringView path, FileModes modes) {
    EXP_ASSERT(!string_view_empty(path));

    i32 open_flags = 0;
    switch (modes) {
    case FILEMODE_READ:      open_flags = O_RDONLY; break;
    case FILEMODE_WRITE:     open_flags = O_WRONLY; break;
    case FILEMODE_READWRITE: open_flags = O_RDWR; break;
    default:                 EXP_UNREACHABLE();
    }

    i32 file_descriptor = open(path.ptr, open_flags);
    if (file_descriptor < 0) { PANIC_ERRNO("open failed"); }
    return (File){file_descriptor};
}

void file_close(File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    if (close(file->descriptor) < 0) { PANIC_ERRNO("close failed"); }
    file->descriptor = -1;
}

void file_remove(StringView path) {
    EXP_ASSERT(!string_view_empty(path));
    if (unlink(path.ptr) < 0) { PANIC_ERRNO("unlink failed"); }
}

void file_write(StringView buffer, File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    ssize_t code = write(file->descriptor, buffer.ptr, buffer.length);
    if (code < 0) { PANIC_ERRNO("write failed"); }
}

void file_write_i64(i64 value, File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    u64 length           = i64_safe_strlen(value);
    char buf[length + 1] = {};
    if (i64_to_str(value, buf) == NULL) { PANIC("i64_to_str failed"); }
    file_write(string_view_from_str(buf, length), file);
}

void file_write_u64(u64 value, File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    u64 length           = u64_safe_strlen(value);
    char buf[length + 1] = {};
    if (u64_to_str(value, buf) == NULL) { PANIC("u64_to_str failed"); }
    file_write(string_view_from_str(buf, length), file);
}

u64 file_read(char *buffer, u64 length, File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    ssize_t count = read(file->descriptor, buffer, length);
    if (count < 0) { PANIC_ERRNO("read failed"); }
    return (u64)count;
}

u64 file_length(File *file) {
    EXP_ASSERT(file != nullptr);
    EXP_ASSERT(file->descriptor >= 0);
    struct stat info;
    if (fstat(file->descriptor, &info) != 0) { PANIC_ERRNO("fstat failed"); }

    return (u64)info.st_size;
}
