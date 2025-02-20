// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_INT_TYPES_H
#define EXP_UTILITY_INT_TYPES_H

#include "utility/config.h"

#ifndef EXP_HOST_SYSTEM_LINUX
    #error "unsupported host OS"
#endif

#ifndef EXP_HOST_CPU_X86_64
    #error "unsupported host CPU"
#endif

// x86_64 linux uses the LP64 data model, which means
// char  = 8 bits
// short = 16 bits
// int   = 32 bits
// long  = 64 bits

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

#define i8_MAX  (127)
#define i8_MIN  (-128)
#define i16_MAX (32'767)
#define i16_MIN (-32'768)
#define i32_MAX (2'147'483'647)
#define i32_MIN (-2'147'483'648)
// 9223372036854775808
#define i64_MAX (9'223'372'036'854'775'807L)
#define i64_MIN (-1 + (9'223'372'036'854'775'807L))

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

#define u8_MAX  (255)
#define u16_MAX (65'535)
#define u32_MAX (4'294'967'295)
#define u64_MAX (18'446'744'073'709'551'615UL)

bool i64_in_range_i32(i64 value);
bool i64_in_range_i16(i64 value);
bool i64_in_range_i8(i64 value);

bool i64_in_range_u8(i64 value);
bool i64_in_range_u16(i64 value);
bool i64_in_range_u32(i64 value);
bool i64_in_range_u64(i64 value);

bool u64_in_range_i64(u64 value);
bool u64_in_range_i32(u64 value);
bool u64_in_range_i16(u64 value);
bool u64_in_range_i8(u64 value);

bool u64_in_range_u8(u64 value);
bool u64_in_range_u16(u64 value);
bool u64_in_range_u32(u64 value);

#endif // !EXP_UTILITY_INT_TYPES_H
