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
#ifndef EXP_IMR_OPERAND_H
#define EXP_IMR_OPERAND_H

#include "support/constant_string.h"
#include "support/scalar.h"
#include "support/string.h"

typedef enum OperandKind : u8 {
    OPERAND_KIND_LOCAL,
    OPERAND_KIND_CONSTANT,
    OPERAND_KIND_TYPE,
    OPERAND_KIND_LABEL,
    OPERAND_KIND_NIL,
    OPERAND_KIND_BOOL,
    OPERAND_KIND_U8,
    OPERAND_KIND_U16,
    OPERAND_KIND_U32,
    OPERAND_KIND_U64,
    OPERAND_KIND_I8,
    OPERAND_KIND_I16,
    OPERAND_KIND_I32,
    OPERAND_KIND_I64,
} OperandKind;

struct Value;
struct Type;

typedef union OperandData {
    u32                   local;
    struct Value const   *constant;
    struct Type const    *type;
    ConstantString const *label;
    u8                    nil;
    bool                  bool_;
    u8                    u8_;
    u16                   u16_;
    u32                   u32_;
    u64                   u64_;
    i8                    i8_;
    i16                   i16_;
    i32                   i32_;
    i64                   i64_;
} OperandData;

typedef struct Operand {
    OperandKind kind;
    OperandData data;
} Operand;

struct Context;

inline Operand operand(OperandKind kind, OperandData data) {
    return (Operand){.kind = kind, .data = data};
}

inline Operand operand_local(u32 local) {
    return (Operand){.kind = OPERAND_KIND_LOCAL, .data.local = local};
}

inline Operand operand_constant(struct Value const *constant) {
    return (Operand){.kind = OPERAND_KIND_CONSTANT, .data.constant = constant};
}

inline Operand operand_type(struct Type const *type) {
    return (Operand){.kind = OPERAND_KIND_TYPE, .data.type = type};
}

inline Operand operand_label(ConstantString const *label) {
    return (Operand){.kind = OPERAND_KIND_LABEL, .data.label = label};
}

inline Operand operand_nil() {
    return (Operand){.kind = OPERAND_KIND_NIL, .data.nil = 0};
}

inline Operand operand_bool(bool bool_) {
    return (Operand){.kind = OPERAND_KIND_BOOL, .data.bool_ = bool_};
}

inline Operand operand_u8(u8 u8_) {
    return (Operand){.kind = OPERAND_KIND_U8, .data.u8_ = u8_};
}

inline Operand operand_u16(u16 u16_) {
    return (Operand){.kind = OPERAND_KIND_U16, .data.u16_ = u16_};
}

inline Operand operand_u32(u32 u32_) {
    return (Operand){.kind = OPERAND_KIND_U32, .data.u32_ = u32_};
}

inline Operand operand_u64(u64 u64_) {
    return (Operand){.kind = OPERAND_KIND_U64, .data.u64_ = u64_};
}

inline Operand operand_i8(i8 i8_) {
    return (Operand){.kind = OPERAND_KIND_I8, .data.i8_ = i8_};
}

inline Operand operand_i16(i16 i16_) {
    return (Operand){.kind = OPERAND_KIND_I16, .data.i16_ = i16_};
}

inline Operand operand_i32(i32 i32_) {
    return (Operand){.kind = OPERAND_KIND_I32, .data.i32_ = i32_};
}

inline Operand operand_i64(i64 i64_) {
    return (Operand){.kind = OPERAND_KIND_I64, .data.i64_ = i64_};
}

bool operand_equality(Operand A, Operand B);
bool operand_is_index(Operand A);
u64  operand_as_index(Operand A);

void print_operand(String *restrict string,
                   Operand operand,
                   struct Context *restrict context);

#endif // EXP_IMR_OPERAND_H
