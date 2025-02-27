
#include "utility/int_types.h"

bool i64_in_range_i16(i64 value) {
    return (value >= i16_MIN) && (value <= i16_MAX);
}
