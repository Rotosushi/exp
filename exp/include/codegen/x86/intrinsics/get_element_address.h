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
#ifndef EXP_BACKEND_X64_INTRINSICS_GET_ELEMENT_ADDRESS_H
#define EXP_BACKEND_X64_INTRINSICS_GET_ELEMENT_ADDRESS_H

#include "codegen/x86/imr/address.h"
#include "imr/type.h"

/**
 * @brief Get the address of an element in a tuple
 *
 * @note This function computes the address at comptime. When considering
 * addresses of elements in memory more generally we have to consider the
 * full capabilities of the LEA instruction.
 *
 * @param src The address of the tuple.
 * @param type The type of the tuple.
 * @param index The index of the element to get the address of.
 * @return The address of the element.
 */
x86_Address
x86_get_element_address(x86_Address *src, Type const *type, u64 index);

#endif // !EXP_BACKEND_X64_INTRINSICS_GET_ELEMENT_ADDRESS_H
