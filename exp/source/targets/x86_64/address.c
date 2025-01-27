
#include <assert.h>

#include "targets/x86_64/address.h"

[[maybe_unused]] static bool validate_scale(u8 scale) {
    return ((scale == 1) || (scale == 2) || (scale == 4) || (scale == 8));
}

x86_64_Address x86_64_address_create(x86_64_GPR base, i64 offset) {
    x86_64_Address address = {
        .base = base, .scale = 1, .has_index = false, .offset = offset};
    return address;
}

x86_64_Address x86_64_address_create_indexed(x86_64_GPR base,
                                             x86_64_GPR index,
                                             u8 scale,
                                             i64 offset) {
    assert(validate_scale(scale));
    x86_64_Address address = {.base      = base,
                              .index     = index,
                              .scale     = scale,
                              .has_index = true,
                              .offset    = offset};
    return address;
}

void print_x86_64_address(String *buffer, x86_64_Address address) {
    string_append_i64(buffer, address.offset);

    string_append(buffer, SV("("));
    print_x86_64_gpr(buffer, address.base);

    if (address.has_index) {
        string_append(buffer, SV(", "));
        print_x86_64_gpr(buffer, address.index);
        string_append(buffer, SV(", "));
        string_append_u64(buffer, address.scale);
    }

    string_append(buffer, SV(")"));
}
