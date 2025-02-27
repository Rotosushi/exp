
#include "utility/int_types.h"

bool i64_in_range_i16(i64 value) {
    return (value >= i16_MIN) && (value <= i16_MAX);
}

bool i64_in_range_i8(i64 value) {
    return (value >= i8_MIN) && (value <= i8_MAX);
}

bool i64_in_range_u8(i64 value) { return (value >= 0) && (value <= u8_MAX); }
