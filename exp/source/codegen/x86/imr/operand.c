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
#include <stddef.h>

#include "codegen/x86/imr/operand.h"
#include "support/assert.h"
#include "support/unreachable.h"

x86_Operand x86_operand(x86_OperandKind kind, x86_OperandData data) {
    return (x86_Operand){.kind = kind, .data = data};
}

x86_Operand x86_operand_location(x86_Location location) {
    return (x86_Operand){.kind          = X86_OPERAND_KIND_LOCATION,
                         .data.location = location};
}

x86_Operand x86_operand_location_gpr(x86_GPR gpr) {
    return (x86_Operand){.kind          = X86_OPERAND_KIND_LOCATION,
                         .data.location = x86_location_gpr(gpr)};
}

x86_Operand
x86_operand_location_address(x86_GPR base, x86_PtrKind ptr_kind, i32 offset) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_LOCATION,
                         .data.location =
                             x86_location_address(base, ptr_kind, offset)};
}

x86_Operand x86_operand_location_address_indexed(
    x86_GPR base, x86_PtrKind ptr_kind, x86_GPR index, u8 scale, i32 offset) {
    return (x86_Operand){.kind          = X86_OPERAND_KIND_LOCATION,
                         .data.location = x86_location_address_indexed(
                             base, ptr_kind, index, scale, offset)};
}

x86_Operand x86_operand_label(ConstantString const *label) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_LABEL, .data.label = label};
}

x86_Operand x86_operand_nil() {
    return (x86_Operand){.kind = X86_OPERAND_KIND_NIL, .data.nil = 0};
}

x86_Operand x86_operand_bool(bool bool_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_BOOL, .data.bool_ = bool_};
}

x86_Operand x86_operand_u8(u8 u8_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_U8, .data.u8_ = u8_};
}

x86_Operand x86_operand_u16(u16 u16_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_U16, .data.u16_ = u16_};
}

x86_Operand x86_operand_u32(u32 u32_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_U32, .data.u32_ = u32_};
}

x86_Operand x86_operand_u64(u64 u64_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_U64, .data.u64_ = u64_};
}

x86_Operand x86_operand_i8(i8 i8_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_I8, .data.i8_ = i8_};
}

x86_Operand x86_operand_i16(i16 i16_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_I16, .data.i16_ = i16_};
}

x86_Operand x86_operand_i32(i32 i32_) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_I32, .data.i32_ = i32_};
}

x86_Operand x86_operand_i64(i64 value) {
    return (x86_Operand){.kind = X86_OPERAND_KIND_I64, .data.i64_ = value};
}

static void print_x86_rip_label(String *restrict buffer, StringView label) {
    string_append(buffer, SV("[rip +"));
    string_append(buffer, label);
    string_append(buffer, SV("]"));
}

void print_x86_operand(String *restrict buffer, x86_Operand operand) {
    exp_assert(buffer != NULL);
    switch (operand.kind) {
    case X86_OPERAND_KIND_LOCATION:
        print_x86_location(buffer, operand.data.location);
        break;
    case X86_OPERAND_KIND_LABEL:
        // #NOTE: assume RIP addressing of global symbols
        print_x86_rip_label(buffer,
                            constant_string_to_view(operand.data.label));
        break;
    case X86_OPERAND_KIND_NIL: string_append_u64(buffer, 0); break;
    case X86_OPERAND_KIND_BOOL:
        string_append_u64(buffer, operand.data.bool_ ? 1 : 0);
        break;
    case X86_OPERAND_KIND_U8:
        string_append_u64(buffer, operand.data.u8_);
        break;
    case X86_OPERAND_KIND_U16:
        string_append_u64(buffer, operand.data.u16_);
        break;
    case X86_OPERAND_KIND_U32:
        string_append_u64(buffer, operand.data.u32_);
        break;
    case X86_OPERAND_KIND_U64:
        string_append_u64(buffer, operand.data.u64_);
        break;
    case X86_OPERAND_KIND_I8:
        string_append_i64(buffer, operand.data.i8_);
        break;
    case X86_OPERAND_KIND_I16:
        string_append_i64(buffer, operand.data.i16_);
        break;
    case X86_OPERAND_KIND_I32:
        string_append_i64(buffer, operand.data.i32_);
        break;
    case X86_OPERAND_KIND_I64:
        string_append_i64(buffer, operand.data.i64_);
        break;
    default: EXP_UNREACHABLE();
    }
}
