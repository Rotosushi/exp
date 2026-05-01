# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

set(EXP_SUPPORT_BASE_DIR ${EXP_BASE_DIR}/support)
set(EXP_SUPPORT_SOURCE_DIR ${EXP_SUPPORT_BASE_DIR}/source)
set(EXP_SUPPORT_INCLUDE_DIR ${EXP_SUPPORT_BASE_DIR}/include)
set(EXP_SUPPORT_TEST_DIR ${EXP_SUPPORT_BASE_DIR}/test)

add_library(exp-support)
target_sources(exp-support
    PRIVATE
        ${EXP_SUPPORT_SOURCE_DIR}/support/cli_option_parser.c
        ${EXP_SUPPORT_SOURCE_DIR}/support/string_view.c
)
target_include_directories(exp-support
    PUBLIC
        ${EXP_SUPPORT_INCLUDE_DIR}
)

add_subdirectory(${EXP_SUPPORT_TEST_DIR})

