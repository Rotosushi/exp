
#include "support/scalar.h"

bool i64_in_range_i8(i64 value) {
    return (value >= i8_MIN) && (value <= i8_MAX);
}

bool i64_in_range_i16(i64 value) {
    return (value >= i16_MIN) && (value <= i16_MAX);
}

bool i64_in_range_i32(i64 value) {
    return (value >= i32_MIN) && (value <= i32_MAX);
}

bool i64_in_range_u8(i64 value) { return (value >= 0) && (value <= u8_MAX); }

bool i64_in_range_u16(i64 value) { return (value >= 0) && (value <= u16_MAX); }

bool i64_in_range_u32(i64 value) { return (value >= 0) && (value <= u32_MAX); }

bool i64_in_range_u64(i64 value) { return (value >= 0); }

bool u64_in_range_i8(u64 value) { return (value <= i8_MAX); }

bool u64_in_range_i16(u64 value) { return (value <= i16_MAX); }

bool u64_in_range_i32(u64 value) { return (value <= i32_MAX); }

bool u64_in_range_i64(u64 value) { return (value <= i64_MAX); }

bool u64_in_range_u8(u64 value) { return (value <= u8_MAX); }

bool u64_in_range_u16(u64 value) { return (value <= u16_MAX); }

bool u64_in_range_u32(u64 value) { return (value <= u32_MAX); }

i8 min_i8(i8 x, i8 y) { return (x < y) ? x : y; }

i16 min_i16(i16 x, i16 y) { return (x < y) ? x : y; }

i32 min_i32(i32 x, i32 y) { return (x < y) ? x : y; }

i64 min_i64(i64 x, i64 y) { return (x < y) ? x : y; }

u8 min_u8(u8 x, u8 y) { return (x < y) ? x : y; }

u16 min_u16(u16 x, u16 y) { return (x < y) ? x : y; }

u32 min_u32(u32 x, u32 y) { return (x < y) ? x : y; }

u64 min_u64(u64 x, u64 y) { return (x < y) ? x : y; }

i8 max_i8(i8 x, i8 y) { return (x > y) ? x : y; }

i16 max_i16(i16 x, i16 y) { return (x > y) ? x : y; }

i32 max_i32(i32 x, i32 y) { return (x > y) ? x : y; }

i64 max_i64(i64 x, i64 y) { return (x > y) ? x : y; }

u8 max_u8(u8 x, u8 y) { return (x > y) ? x : y; }

u16 max_u16(u16 x, u16 y) { return (x > y) ? x : y; }

u32 max_u32(u32 x, u32 y) { return (x > y) ? x : y; }

u64 max_u64(u64 x, u64 y) { return (x > y) ? x : y; }

// #NOTE: when compiling under GCC "-value" undergoes integer promotion
// to int. Then when we return into a smaller type, this generates a warning.
// Ideally I would not cast, but it is necessary to suppress the warning.
i8 abs_i8(i8 value) { return (value < 0) ? (i8)(-value) : value; }

i16 abs_i16(i16 value) { return (value < 0) ? (i8)(-value) : value; }

i32 abs_i32(i32 value) { return (value < 0) ? -value : value; }

i64 abs_i64(i64 value) { return (value < 0) ? -value : value; }
