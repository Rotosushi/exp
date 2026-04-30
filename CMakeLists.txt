# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

project(exp)

include(CTest)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps.cmake)

set(EXP_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/exp)

add_subdirectory(${EXP_BASE_DIR})
