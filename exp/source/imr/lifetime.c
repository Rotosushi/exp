

#include "imr/lifetime.h"

Lifetime lifetime_construct(u32 first, u32 last) {
    Lifetime lifetime = {.first_use = first, .last_use = last};
    return lifetime;
}

Lifetime lifetime_create() { return lifetime_construct(0, 0); }
Lifetime lifetime_immortal() { return lifetime_construct(0, u32_MAX); }
