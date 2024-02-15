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
#include <math.h>

#include "utility/numbers_to_string.h"

size_t intmax_safe_strlen(intmax_t value, unsigned base) {
  assert((base >= 2) || (base <= 32));

  if ((value == 0) || (value == 1)) {
    // "0" or "1" + "\0"
    return 2;
  }

  if (value < 0) {
    // "-" + "<value>" + "\0"
    double absolute = fabs((double)value);
    return (size_t)(ceil(log(absolute) / log((double)base)) + 2);
  } else {
    // "<value>" + "\0"
    return (size_t)(ceil(log((double)value) / log((double)base)) + 1);
  }
}

size_t uintmax_safe_strlen(uintmax_t value, unsigned base) {
  assert((base >= 2) || (base <= 32));

  if ((value == 0) || (value == 1)) {
    // "0" or "1" + "\0"
    return 2;
  }

  // "<value>" + "\0"
  return (size_t)(ceil(log((double)value) / log((double)base)) + 1);
}

char *intmax_to_str(intmax_t value, char *buffer, unsigned base) {
  assert((base >= 2) || (base <= 32));

  intmax_t tmp_value;
  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    tmp_value = value;
    value /= base;
    *ptr1++ = mapping[35 + (tmp_value - value * base)];
  } while (value);

  // append the sign and null terminator
  if (tmp_value < 0) {
    *ptr1++ = '-';
  }

  ptr2 = buffer;
  buffer = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1-- = *ptr2;
    *ptr2++ = tmp;
  }

  return buffer;
}

char *uintmax_to_str(uintmax_t value, char *buffer, unsigned base) {
  assert((base >= 2) || (base <= 32));

  char *ptr1, *ptr2;
  char mapping[] =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

  // convert the number
  ptr1 = buffer;
  do {
    uintmax_t tmp = value;
    value /= base;
    *ptr1++ = mapping[35 + (tmp - value * base)];
  } while (value);

  // append the null terminator
  ptr2 = buffer;
  buffer = ptr1;
  *ptr1-- = '\0';

  // reverse the string
  while (ptr2 < ptr1) {
    char tmp = *ptr1;
    *ptr1-- = *ptr2;
    *ptr2++ = tmp;
  }

  return buffer;
}