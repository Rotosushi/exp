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

#include "utility/config.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

#define TO_BUF(buf, sv)                                                        \
  char buf[sv.length + 1];                                                     \
  memcpy(buf, sv.ptr, sv.length);                                              \
  buf[sv.length] = '\0'

FILE *file_open(StringView path, StringView modes) {
  assert(!string_view_empty(path));
  assert(!string_view_empty(modes));
  TO_BUF(pbuf, path);
  TO_BUF(mbuf, modes);
  FILE *file = fopen(pbuf, mbuf);
  if (file == NULL) { PANIC_ERRNO("fopen failed"); }
  return file;
}

void file_close(FILE *restrict file) {
  assert(file != NULL);
  if (fclose(file) == EOF) { PANIC_ERRNO("fclose failed"); }
}

void file_remove(StringView path) {
  TO_BUF(buf, path);
  if (remove(buf)) { PANIC_ERRNO("remove failed"); }
}

void file_write(FILE *restrict out, StringView buffer) {
  for (u64 i = 0; i < buffer.length; ++i) {
    int c = fputc(buffer.ptr[i], out);
    if (c == EOF) {
      panic(
          string_view_from_cstring(strerror(ferror(out))), __FILE__, __LINE__);
    }
  }
}

void file_write_i64(FILE *restrict out, i64 value) {
  u64 len           = i64_safe_strlen(value);
  char buf[len + 1] = {};
  if (i64_to_str(value, buf) == NULL) { PANIC("i64_to_str failed"); }
  file_write(out, string_view(len, buf));
}

void file_write_u64(FILE *restrict out, u64 value) {
  u64 len                              = u64_safe_strlen(value);
  char buf[u64_safe_strlen(value) + 1] = {};
  if (u64_to_str(value, buf) == NULL) { PANIC("u64_to_str failed"); }
  file_write(out, string_view(len, buf));
}

u64 file_read(FILE *restrict stream, char *buffer, u64 length) {
  u64 count = fread(buffer, sizeof(*buffer), length, stream);
  int error = ferror(stream);
  if (error != 0) { PANIC(strerror(error)); }

  return count;
}

#if defined(EXP_HOST_OS_LINUX)
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
