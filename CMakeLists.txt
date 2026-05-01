# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

project(exp)

set(EXP_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/exp)
set(EXP_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CTest)
include(${EXP_CMAKE_DIR}/deps.cmake)
include(${EXP_CMAKE_DIR}/discoverTests.cmake)

add_subdirectory(${EXP_BASE_DIR})
