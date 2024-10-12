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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X64_LOCATION_H
#define EXP_BACKEND_X64_LOCATION_H

#include "backend/x64/registers.h"

typedef enum x64_LocationKind : u8 {
  LOCATION_GPR,
  LOCATION_ADDRESS,
} x64_LocationKind;

typedef enum x64_AddressOperandKind {
  X64AOPR_GPR,
  X64AOPR_INDEX,
} x64_AddressOperandKind;

typedef struct x64_AddressOperand {
  x64_AddressOperandKind kind;
  union {
    x64_GPR gpr;
    u64 index;
  };
} x64_AddressOperand;

x64_AddressOperand x64_address_operand_gpr(x64_GPR gpr);
x64_AddressOperand x64_address_operand_index(u64 index);
bool x64_address_operand_equals(x64_AddressOperand one, x64_AddressOperand two);

typedef struct x64_OptionalAddressOperand {
  bool present;
  x64_AddressOperand operand;
} x64_OptionalAddressOperand;

x64_OptionalAddressOperand x64_optional_address_operand_empty();
x64_OptionalAddressOperand
x64_optional_address_operand(x64_AddressOperand operand);

typedef struct x64_OptionalU8 {
  bool present;
  u8 value;
} x64_OptionalU8;

x64_OptionalU8 x64_optional_u8_empty();
x64_OptionalU8 x64_optional_u8(u8 value);

typedef struct x64_OptionalI64 {
  bool present;
  i64 value;
} x64_OptionalI64;

x64_OptionalI64 x64_optional_i64_empty();
x64_OptionalI64 x64_optional_i64(i64 value);

typedef struct x64_Address {
  x64_AddressOperand base;
  x64_OptionalAddressOperand index;
  x64_OptionalU8 scale;
  x64_OptionalI64 offset;
} x64_Address;

x64_Address x64_address_construct(x64_AddressOperand base,
                                  x64_OptionalAddressOperand optional_index,
                                  x64_OptionalU8 optional_scale,
                                  x64_OptionalI64 optional_offset);
x64_Address x64_address_from_label(u64 index);
x64_Address x64_address_from_gpr(x64_GPR gpr);
void x64_address_increment_offset(x64_Address *restrict address, i64 offset);

typedef struct x64_Location {
  x64_LocationKind kind;
  union {
    x64_GPR gpr;
    x64_Address address;
  };
} x64_Location;

x64_Location x64_location_gpr(x64_GPR gpr);
x64_Location x64_location_address(x64_AddressOperand base,
                                  x64_OptionalAddressOperand optional_index,
                                  x64_OptionalU8 optional_scale,
                                  x64_OptionalI64 optional_offset);
bool x64_location_eq(x64_Location A, x64_Location B);

#endif // !EXP_BACKEND_X64_LOCATION_H
