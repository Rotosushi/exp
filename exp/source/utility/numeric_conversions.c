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
#include <ctype.h>

#include "utility/numeric_conversions.h"
#include "utility/panic.h"

u64 u64_safe_strlen(u64 value) {
  if (value < 10) return 1;
  if (value < 100) return 2;
  if (value < 1'000) return 3;
  if (value < 10'000) return 4;
  if (value < 100'000) return 5;
  if (value < 1'000'000) return 6;
  if (value < 10'000'000) return 7;
  if (value < 100'000'000) return 8;
  if (value < 1'000'000'000) return 9;
  if (value < 10'000'000'000) return 10;
  if (value < 100'000'000'000) return 11;
  if (value < 1'000'000'000'000) return 12;
  if (value < 10'000'000'000'000) return 13;
  if (value < 100'000'000'000'000) return 14;
  if (value < 1'000'000'000'000'000) return 15;
  if (value < 10'000'000'000'000'000) return 16;
  if (value < 100'000'000'000'000'000) return 17;
  if (value < 1'000'000'000'000'000'000) return 18;
  if (value < 10'000'000'000'000'000'000UL) return 19;
  return 20;
}

u64 i64_safe_strlen(i64 value) {
  if (value < 0) {
    value = (value == i64_MIN) ? i64_MAX : -value;
    return u64_safe_strlen((u64)value) + 1;
  }
  return u64_safe_strlen((u64)value);
}

char *i64_to_str(i64 value, char *restrict buffer) {
  assert(buffer != NULL);

  i64 tmp_value;
  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    tmp_value = value;
    value /= 10;
    *ptr1++ = mapping[35 + (tmp_value - value * 10)];
  } while (value);

  // append the sign
  if (tmp_value < 0) { *ptr1++ = '-'; }
  // null terminate
  ptr2    = buffer;
  buffer  = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1--  = *ptr2;
    *ptr2++  = tmp;
  }

  return buffer;
}

char *u64_to_str(u64 value, char *restrict buffer) {
  assert(buffer != NULL);

  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    u64 tmp = value;
    value /= 10;
    *ptr1++ = mapping[35 + (tmp - value * 10)];
  } while (value);

  // append the null terminator
  ptr2    = buffer;
  buffer  = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1--  = *ptr2;
    *ptr2++  = tmp;
  }

  return buffer;
}

static u64 base10_stou64(char const *restrict buffer, u64 length) {
  u64 result = 0;
  u64 i      = 0;

  while (i < length && isdigit(buffer[i])) {
    if (__builtin_mul_overflow(result, 10, &result)) {
      PANIC("string out of range of u64");
    }

    if (__builtin_add_overflow(result, (buffer[i] - '0'), &result)) {
      PANIC("string out of range of u64");
    }

    ++i;
  }

  return result;
}

static i64 base10_stoi64(char const *restrict buffer, u64 length) {
  char const *cursor = buffer;
  i64 sign           = 1;
  if (*cursor == '-') {
    sign = -1;
    ++cursor;
  }

  u64 val = base10_stou64(cursor, length);
  if ((val == ((u64)i64_MAX + 1)) && (sign == -1)) {
    return i64_MIN;
  } else if (val > i64_MAX) {
    PANIC("string out of range of i64");
  }

  // since we know val <= i64_MAX it is always safe to
  // cast val to an i64.
  return (i64)val * sign;
}

i64 str_to_i64(char const *restrict buffer, u64 length) {
  assert(buffer != NULL);
  return base10_stoi64(buffer, length);
}

u64 str_to_u64(char const *restrict buffer, u64 length) {
  assert(buffer != NULL);
  return base10_stou64(buffer, length);
}
