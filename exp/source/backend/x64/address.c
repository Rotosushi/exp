
#include <assert.h>

#include "backend/x64/address.h"

x64_Address x64_address_create(x64_GPR base, i64 offset) {
    x64_Address address = {.base      = base,
                           .index     = 0,
                           .scale     = 1,
                           .has_index = false,
                           .offset    = offset};
    return address;
}

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x64_Address
x64_address_create_indexed(x64_GPR base, x64_GPR index, u8 scale, i64 offset) {
    assert(validate_scale(scale));
    x64_Address address = {.base      = base,
                           .index     = index,
                           .scale     = scale,
                           .has_index = true,
                           .offset    = offset};
    return address;
}

bool x64_address_equality(x64_Address A, x64_Address B) {
    if ((A.base != B.base) || (A.offset != B.offset)) return false;
    if (!A.has_index) return !B.has_index;
    if (!B.has_index) return false;
    return (A.index == B.index) && (A.scale == B.scale);
}
