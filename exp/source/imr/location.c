

#include "imr/location.h"

static Location location_construct(LocationKind kind, LocationData data) {
    Location location = {.kind = kind, .data = data};
    return location;
}

Location location_create() {
    Location location = {};
    return location;
}

Location location_register(u32 general_purpose_register) {
    return location_construct(
        LOCATION_GENERAL_PURPOSE_REGISTER,
        (LocationData){.general_purpose_register = general_purpose_register});
}

Location location_stack_slot(u32 stack_slot) {
    return location_construct(LOCATION_STACK_SLOT,
                              (LocationData){.stack_slot = stack_slot});
}
