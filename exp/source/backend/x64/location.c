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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stddef.h>

#include "backend/x64/location.h"
#include "utility/panic.h"

x64_OptionalGPR x64_optional_gpr_empty() {
  x64_OptionalGPR opt = {.present = false};
  return opt;
}
x64_OptionalGPR x64_optional_gpr(x64_GPR gpr) {
  x64_OptionalGPR opt = {.present = true, .gpr = gpr};
  return opt;
}

x64_OptionalU8 x64_optional_u8_empty() {
  x64_OptionalU8 opt = {.present = false};
  return opt;
}

x64_OptionalU8 x64_optional_u8(u8 value) {
  x64_OptionalU8 opt = {.present = true, .value = value};
  return opt;
}

x64_OptionalI64 x64_optional_i64_empty() {
  x64_OptionalI64 opt = {.present = false};
  return opt;
}

x64_OptionalI64 x64_optional_i64(i64 value) {
  x64_OptionalI64 opt = {.present = true, .value = value};
  return opt;
}

x64_Location x64_location_gpr(x64_GPR gpr) {
  x64_Location a = {.kind = LOCATION_GPR, .gpr = gpr};
  return a;
}

void x64_address_increment_offset(x64_Address *restrict address, u64 offset) {
  assert(offset <= i64_MAX);
  i64 increment = (i64)offset;

  if (!address->offset.present) {
    address->offset = x64_optional_i64(increment);
    return;
  }

  if (__builtin_add_overflow(
          address->offset.value, increment, &address->offset.value)) {
    PANIC("address offset overflow");
  }
}

[[maybe_unused]] static void validate_scale(x64_OptionalU8 scale) {
  if (scale.present) {
    assert((scale.value == 1) || (scale.value == 2) || (scale.value == 4) ||
           (scale.value == 8));
  }
}

x64_Location x64_location_address(x64_GPR base,
                                  x64_OptionalGPR optional_index,
                                  x64_OptionalU8 optional_scale,
                                  x64_OptionalI64 optional_offset) {
  validate_scale(optional_scale);
  x64_Location a = {
      .kind    = LOCATION_ADDRESS,
      .address = {.base   = base,
                  .index  = optional_index,
                  .scale  = optional_scale,
                  .offset = optional_offset}
  };
  return a;
}

static bool x64_optional_gpr_equality(x64_OptionalGPR A, x64_OptionalGPR B) {
  if (A.present && B.present) { return A.gpr == B.gpr; }
  if (!A.present && !B.present) { return true; }
  return false;
}

static bool x64_optional_u8_equality(x64_OptionalU8 A, x64_OptionalU8 B) {
  if (A.present && B.present) { return A.value == B.value; }
  if (!A.present && !B.present) { return true; }
  return false;
}

static bool x64_optional_i64_equality(x64_OptionalI64 A, x64_OptionalI64 B) {
  if (A.present && B.present) { return A.value == B.value; }
  if (!A.present && !B.present) { return true; }
  return false;
}

bool x64_location_eq(x64_Location A, x64_Location B) {
  if (A.kind != B.kind) { return 0; }

  switch (A.kind) {
  case LOCATION_GPR:     return A.gpr == B.gpr;
  case LOCATION_ADDRESS: {
    if (A.address.base != B.address.base) { return false; }

    if (!x64_optional_gpr_equality(A.address.index, B.address.index)) {
      return false;
    }

    if (!x64_optional_u8_equality(A.address.scale, B.address.scale)) {
      return false;
    }

    return x64_optional_i64_equality(A.address.offset, B.address.offset);
  }

  default: unreachable();
  }
}
