# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

set(EXP_TEST_BASE_DIR ${EXP_BASE_DIR}/test)

add_executable(test-exp)
target_sources(test-exp
    PRIVATE
        ${EXP_TEST_BASE_DIR}/source/main.c
)
target_include_directories(test-exp
    PRIVATE
        ${EXP_TEST_BASE_DIR}/include
)
target_link_libraries(test-exp
    PRIVATE
        test-exp-support
        PkgConfig::Check
)

discoverTests(test-exp)
