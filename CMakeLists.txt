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
message(STATUS "C standard ${CMAKE_C_STANDARD}")
message(STATUS "C compiler ${CMAKE_C_COMPILER}")

project(EXP LANGUAGES C ASM VERSION 0.1.0 DESCRIPTION "A hobby programming language.")

set(EXP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/exp/include")
set(EXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/exp/source")
set(EXP_TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(EXP_BINARY_DIR "${PROJECT_BINARY_DIR}")
set(EXP_LIBEXP_RUNTIME_SOURCE_DIR "${PROJECT_SOURCE_DIR}/libexp_runtime")
set(EXP_LIBEXP_RUNTIME_BINARY_DIR "${EXP_BINARY_DIR}/libexp_runtime")

set(EXP_WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
set(EXP_SANITIZERS -fsanitize=address,undefined,leak)

if ($<CONFIG:Debug> OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS} ${EXP_SANITIZERS})
  set(EXP_LINK_OPTIONS ${EXP_SANITIZERS})
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

add_subdirectory(${EXP_SOURCE_DIR})
add_subdirectory(${EXP_LIBEXP_RUNTIME_SOURCE_DIR})

include(CTest)
add_subdirectory(${EXP_TEST_DIR})


