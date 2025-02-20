// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_CONFIG_H
#define EXP_UTILITY_CONFIG_H
// NOLINTBEGIN
// clang-format off
#define EXP_VERSION_MAJOR 0
#define EXP_VERSION_MINOR 2 
#define EXP_VERSION_PATCH 0
#define EXP_GIT_REVISION "4a46535"
#define EXP_TEST_DIR "/home/elise/Projects/exp/test"
#define EXP_TEST_RESOURCES_DIR "/home/elise/Projects/exp/test/resources"
#define EXP_BINARY_DIR "/home/elise/Projects/exp/build"
#define EXP_LIBEXP_RUNTIME_SOURCE_DIR "/home/elise/Projects/exp/libexprt"
#define EXP_LIBEXP_RUNTIME_BINARY_DIR "/home/elise/Projects/exp/build/libexprt"
#define EXP_HOST_SYSTEM_LINUX
/* #undef EXP_HOST_SYSTEM_WINDOWS */
/* #undef EXP_HOST_SYSTEM_APPLE */
#define EXP_HOST_CPU_X86_64
/* #undef EXP_HOST_CPU_ARM_64 */
/* #undef EXP_HOST_CPU_RISCV */

#if defined(__GNUC__) && !defined(__clang__)
#define EXP_HOST_COMPILER_GCC
#elif defined(__clang__)
#define EXP_HOST_COMPILER_CLANG
#elif defined(_MSC_VER)
#define EXP_HOST_COMPILER_MSVC
#endif

#define EXP_VERSION_STRING "exp version (0.2.0) built at (2025-02-19 15:44:57) git revision (4a46535)"

/* #undef EXP_REMOVE_ASSERTS */
// NOLINTEND
// clang-format on

#endif // !EXP_UTILITY_CONFIG_H
