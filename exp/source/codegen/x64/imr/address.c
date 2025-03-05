
#include <assert.h>

#include "codegen/x64/imr/address.h"

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x64_Address
x64_address_create(x64_GPR base, x64_GPR index, u8 scale, i64 offset) {
    assert(base != X64_GPR_NONE);
    assert(validate_scale(scale));
    x64_Address address = {
        .base = base, .index = index, .scale = scale, .offset = offset};
    return address;
}

bool x64_address_equality(x64_Address A, x64_Address B) {
    return (A.base == B.base) && (A.index == B.index) && (A.scale == B.scale) &&
           (A.offset == B.offset);
}
