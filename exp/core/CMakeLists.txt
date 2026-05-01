# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

set(EXP_CORE_DIR ${EXP_BASE_DIR}/core)

add_executable(exp)
target_sources(exp PRIVATE
    ${EXP_CORE_DIR}/source/main.c
)

add_subdirectory(${EXP_CORE_DIR}/test)