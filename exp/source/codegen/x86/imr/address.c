
#include <assert.h>

#include "codegen/x86/imr/address.h"
#include "codegen/x86/imr/registers.h"
#include "support/assert.h"

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x86_Address x86_address_create(x86_GPR base, i64 offset) {
    exp_assert(x86_gpr_is_sized(base));
    return (x86_Address){.base = base, .offset = offset};
}

x86_Address
x86_address_create_indexed(x86_GPR base, x86_GPR index, u8 scale, i64 offset) {
    exp_assert(x86_gpr_is_sized(base));
    exp_assert(x86_gpr_is_sized(index));
    exp_assert(validate_scale(scale));
    return (x86_Address){.base      = base,
                         .index     = index,
                         .scale     = scale,
                         .has_index = true,
                         .offset    = offset};
}

bool x86_address_equality(x86_Address A, x86_Address B) {
    if (A.has_index && B.has_index) {
        return (A.base == B.base) && (A.index == B.index) &&
               (A.scale == B.scale) && (A.offset == B.offset);
    } else if (!A.has_index && !B.has_index) {
        return (A.base == B.base) && (A.offset == B.offset);
    } else {
        return false;
    }
}
