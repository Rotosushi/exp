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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "support/config.h"
#include "support/io.h"
#include "support/numeric_conversions.h"
#include "support/panic.h"

FILE *file_open(char const *restrict path, char const *restrict modes) {
    assert(path != NULL);
    assert(modes != NULL);
    FILE *file = fopen(path, modes);
    if (file == NULL) { PANIC_ERRNO("fopen failed"); }
    return file;
}

void file_close(FILE *restrict file) {
    assert(file != NULL);
    if (fclose(file) == EOF) { PANIC_ERRNO("fclose failed"); }
}

void file_remove(char const *restrict path) {
    if (remove(path)) { PANIC_ERRNO("remove failed"); }
}

void file_write(StringView view, FILE *restrict stream) {
    fwrite(view.ptr, sizeof(*view.ptr), view.length, stream);
    if (ferror(stream)) { PANIC_ERRNO("fputs failed"); }
}

void file_write_i64(i64 value, FILE *restrict stream) {
    u64  buflen          = i64_safe_strlen(value);
    char buf[buflen + 1] = {};
    if (i64_to_str(value, buf) == NULL) { PANIC("i64_to_str failed"); }
    file_write(string_view(buf, buflen), stream);
}

void file_write_u64(u64 value, FILE *restrict stream) {
    u64  buflen                          = u64_safe_strlen(value);
    char buf[u64_safe_strlen(value) + 1] = {};
    if (u64_to_str(value, buf) == NULL) { PANIC("u64_to_str failed"); }
    file_write(string_view(buf, buflen), stream);
}

u64 file_read(char *restrict buffer, u64 length, FILE *restrict stream) {
    u64 count = fread(buffer, sizeof(*buffer), length, stream);
    int error = ferror(stream);
    if (error != 0) { PANIC(strerror(error)); }

    return count;
}

void file_read_all(String *restrict string, StringView path) {
    FILE *file   = file_open(path.ptr, "r");
    u64   length = file_length(file);
    string_resize(string, length + 1);
    char *data       = string_data(string);
    u64   bytes_read = file_read(data, length, file);
    string->length   = bytes_read;
    file_close(file);
}

void file_write_all(String *restrict string, StringView path) {
    FILE *file = file_open(path.ptr, "w");
    file_write(string_to_view(string), file);
    file_close(file);
}

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

u64 file_length(FILE *restrict file) {
    i32 fd = fileno(file);

    struct stat info;
    if (fstat(fd, &info) != 0) { PANIC_ERRNO("fstat failed"); }

    return (u64)info.st_size;
}

#else
u64 file_length(FILE *restrict file) {
    if (fseek(file, 0L, SEEK_END) != 0) { PANIC_ERRNO("fseek failed"); }

    i64 size = ftell(file);
    if (size == -1L) { PANIC_ERRNO("ftell failed"); }

    rewind(file);
    return (u64)size;
}
#endif
