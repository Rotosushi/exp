
#include <assert.h>

#include "imr/local.h"

void local_initialize(Local *local) {
    assert(local != nullptr);
    local->type     = nullptr;
    local->label    = string_view_create();
    local->location = location_register(0);
    local->lifetime = lifetime_create();
}

void local_update_type(Local *local, Type const *type) {
    assert(local != nullptr);
    local->type = type;
}

void local_update_label(Local *local, StringView label) {
    assert(local != nullptr);
    local->label = label;
}

void local_update_location(Local *local, Location location) {
    assert(local != nullptr);
    local->location = location;
}

void local_update_first_use(Local *local, u64 first_use) {
    assert(local != nullptr);
    local->lifetime.first_use = first_use;
}

void local_update_last_use(Local *local, u64 last_use) {
    assert(local != nullptr);
    local->lifetime.last_use = last_use;
}
