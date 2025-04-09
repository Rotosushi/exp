
#include <assert.h>

#include "codegen/x64/imr/address.h"
#include "codegen/x64/imr/registers.h"
#include "support/assert.h"

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return (scale == 1) || (scale == 2) || (scale == 4) || (scale == 8);
}

x64_Address x64_address_create(x86_64_GPR base, i64 offset) {
    exp_assert(x86_64_gpr_is_sized(base));
    return (x64_Address){.base = base, .offset = offset};
}

x64_Address x64_address_create_indexed(x86_64_GPR base,
                                       x86_64_GPR index,
                                       u8 scale,
                                       i64 offset) {
    exp_assert(x86_64_gpr_is_sized(base));
    exp_assert(x86_64_gpr_is_sized(index));
    exp_assert(validate_scale(scale));
    return (x64_Address){.base      = base,
                         .index     = index,
                         .scale     = scale,
                         .has_index = true,
                         .offset    = offset};
}

bool x64_address_equality(x64_Address A, x64_Address B) {
    if (A.has_index && B.has_index) {
        return (A.base == B.base) && (A.index == B.index) &&
               (A.scale == B.scale) && (A.offset == B.offset);
    } else if (!A.has_index && !B.has_index) {
        return (A.base == B.base) && (A.offset == B.offset);
    } else {
        return false;
    }
}
