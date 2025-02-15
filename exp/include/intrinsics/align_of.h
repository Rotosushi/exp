// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_INTRINSICS_ALIGNOF_H
#define EXP_INTRINSICS_ALIGNOF_H
#include <stddef.h>

#include "imr/type.h"

/**
 * @brief returns the native alignment of the <type>
 *
 * @note this is the number of bytes to align a <value> with <type> to.
 *
 * @param type
 * @return u64
 */
u64 align_of(Type const *type);

#endif // !EXP_INTRINSICS_ALIGNOF_H
