/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include <stdlib.h>

#include "utility/alloc.h"
#include "utility/panic.h"

void *allocate(u64 size) {
    void *result = malloc(size);
    if (result == NULL) { PANIC_ERRNO("malloc failed"); }
    return result;
}

void *callocate(u64 num, u64 size) {
    void *result = calloc(num, size);
    if (result == NULL) { PANIC_ERRNO("calloc failed"); }
    return result;
}

void *reallocate(void *ptr, u64 size) {
    void *result = realloc(ptr, size);
    if (result == NULL) { PANIC_ERRNO("reallocate failed."); }
    return result;
}

void deallocate(void *ptr) { free(ptr); }
