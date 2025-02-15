// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_INTRINSICS_SIZEOF_H
#define EXP_INTRINSICS_SIZEOF_H
#include <stddef.h>

#include "imr/type.h"

/**
 * @brief returns the size in bytes of the given type.
 *
 * @param type
 * @return u64
 */
u64 size_of(Type const *type);

#endif // !EXP_INTRINSICS_SIZEOF_H
