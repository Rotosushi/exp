# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

set(EXP_SUPPORT_TEST_INCLUDE_DIR ${EXP_SUPPORT_TEST_DIR}/include)
set(EXP_SUPPORT_TEST_SOURCE_DIR ${EXP_SUPPORT_TEST_DIR}/source)

add_library(test-exp-support)
target_sources(test-exp-support
    PRIVATE
        ${EXP_SUPPORT_TEST_SOURCE_DIR}/support/test_cli_option_parser.c
        ${EXP_SUPPORT_TEST_SOURCE_DIR}/support/test_string_view.c
)
target_include_directories(test-exp-support
    PUBLIC
        ${EXP_SUPPORT_INCLUDE_DIR}
        ${EXP_SUPPORT_TEST_INCLUDE_DIR}
)
target_link_libraries(test-exp-support
    PRIVATE
        exp-support
        PkgConfig::Check
)