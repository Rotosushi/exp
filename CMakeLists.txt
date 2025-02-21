# Copyright (C) 2025 Cade Weinberg
# 
# This file is part of exp.
# 
# exp is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# exp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with exp.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(EXP_HOST_SYSTEM_LINUX true)
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE HOST_CPU
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  set(EXP_HOST_SYSTEM_LINUX false)
  message(FATAL_ERROR "Unsupported Host System: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

if(HOST_CPU STREQUAL "x86_64")
  set(EXP_HOST_CPU_X86_64 true)
else()
  set(EXP_HOST_CPU_X86_64 false)
  message(FATAL_ERROR "Unsupported Host CPU: ${HOST_CPU}")
endif()

set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)

project(EXP LANGUAGES C ASM VERSION 0.2.0 DESCRIPTION "A hobby programming language.")

find_package(Doxygen
  REQUIRED dot
)

set(EXP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/exp/include")
set(EXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/exp/source")
set(EXP_TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(EXP_DOCS_DIR ${PROJECT_SOURCE_DIR}/docs) 
set(EXP_TEST_RESOURCES_DIR "${EXP_TEST_DIR}/resources")
set(EXP_BINARY_DIR "${PROJECT_BINARY_DIR}")
set(EXP_LIBEXP_RUNTIME_SOURCE_DIR "${PROJECT_SOURCE_DIR}/libexprt")
set(EXP_LIBEXP_RUNTIME_BINARY_DIR "${EXP_BINARY_DIR}/libexprt")

set(EXP_WARNINGS -Wall -Werror -Wdeprecated -Wextra -Wpedantic -Wconversion)
set(EXP_SANITIZERS -fsanitize=address,undefined,leak)
set(EXP_PROFILE -pg)
set(EXP_FUZZ -fsanitize-coverage=trace-pc)

# TODO: add a Sanitize, Fuzzing, Profiling build types
if (CMAKE_BUILD_TYPE STREQUAL "Sanitize")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} -g ${EXP_SANITIZERS})
  set(EXP_LINK_OPTIONS ${EXP_SANITIZERS})
elseif (CMAKE_BUILD_TYPE STREQUAL "Profile")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} ${EXP_PROFILE})
  set(EXP_LINK_OPTIONS ${EXP_WARNINGS} ${EXP_PROFILE})
elseif (CMAKE_BUILD_TYPE STREQUAL "Fuzzing")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} ${EXP_FUZZ})
  set(EXP_LINK_OPTIONS)
else()
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS})
  set(EXP_LINK_OPTIONS)
endif()

execute_process(
  COMMAND git rev-parse --short HEAD 
  OUTPUT_VARIABLE EXP_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TIMESTAMP EXP_BUILD_TIME "%Y-%m-%d %H:%M:%S")

set(EXP_VERSION_STRING "")
string(APPEND EXP_VERSION_STRING 
"exp version ("
${EXP_VERSION_MAJOR}
"."
${EXP_VERSION_MINOR}
"."
${EXP_VERSION_PATCH}
") built at ("
${EXP_BUILD_TIME}
") git revision ("
${EXP_GIT_REVISION}
")"
)

configure_file(
  ${EXP_INCLUDE_DIR}/utility/config.h.in 
  ${EXP_INCLUDE_DIR}/utility/config.h
)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C standard: ${CMAKE_C_STANDARD}")

add_subdirectory(${EXP_SOURCE_DIR})
add_subdirectory(${EXP_LIBEXP_RUNTIME_SOURCE_DIR})
add_subdirectory(${EXP_DOCS_DIR})
include(CTest)
add_subdirectory(${EXP_TEST_DIR})



