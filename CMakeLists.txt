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
  set(EXP_HOST_CPU_x64 true)
else()
  set(EXP_HOST_CPU_x64 false)
  message(FATAL_ERROR "Unsupported Host CPU: ${HOST_CPU}")
endif()

set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)

project(EXP LANGUAGES C ASM VERSION 0.2.0 DESCRIPTION "A hobby programming language.")

set(EXP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/exp/include")
set(EXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/exp/source")

set(EXP_LIBEXP_SUPPORT_DIR "${PROJECT_SOURCE_DIR}/libexp_support")
set(EXP_LIBEXP_SUPPORT_SOURCE_DIR "${EXP_LIBEXP_SUPPORT_DIR}/source")
set(EXP_LIBEXP_SUPPORT_INCLUDE_DIR "${EXP_LIBEXP_SUPPORT_DIR}/include")

set(EXP_TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(EXP_TEST_RESOURCES_DIR "${EXP_TEST_DIR}/resources")
set(EXP_BINARY_DIR "${PROJECT_BINARY_DIR}")

set(EXP_LIBEXP_RUNTIME_DIR "${PROJECT_SOURCE_DIR}/libexp_runtime")
set(EXP_LIBEXP_RUNTIME_BINARY_DIR "${EXP_BINARY_DIR}/libexp_runtime")

set(EXP_WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
set(EXP_SANITIZERS -fsanitize=address,undefined,leak)
set(EXP_PROFILE -pg)

if ($<CONFIG:Debug> OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} ${EXP_SANITIZERS})
  set(EXP_LINK_OPTIONS ${EXP_SANITIZERS})
elseif ($<CONFIG:RelWithDebInfo> OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} ${EXP_PROFILE})
  set(EXP_LINK_OPTIONS ${EXP_PROFILE})
else()
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS})
  set(EXP_LINK_OPTIONS)
endif()

message(STATUS "C compiler ${CMAKE_C_COMPILER}")
message(STATUS "C standard ${CMAKE_C_STANDARD}")

add_subdirectory(${EXP_LIBEXP_SUPPORT_DIR})
add_subdirectory(${EXP_LIBEXP_RUNTIME_DIR})
add_subdirectory(${EXP_SOURCE_DIR})


include(CTest)
add_subdirectory(${EXP_TEST_DIR})


