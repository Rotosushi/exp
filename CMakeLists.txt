cmake_minimum_required(VERSION 3.20)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(EXP_HOST_OS_LINUX 1)
  execute_process(
    COMMAND uname -m
    OUTPUT_VARIABLE HOST_CPU
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  set(EXP_HOST_OS_LINUX 0)
  message(FATAL_ERROR "Unsupported Host OS")
endif()



if(HOST_CPU STREQUAL "x86_64")
  set(EXP_HOST_CPU_x64 1)
else()
  set(EXP_HOST_CPU_x64 0)
  message(FATAL_ERROR "Unsupported Host CPU: ${HOST_CPU}")
endif()

set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)

project(EXP LANGUAGES C ASM VERSION 0.1.0 DESCRIPTION "A hobby programming language.")

set(EXP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/exp/include")
set(EXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/exp/source")
set(EXP_TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(LIBEXP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/libexp")
set(EXP_BUILD_DIR "${PROJECT_BINARY_DIR}")

set(EXP_WARNINGS -Wall -Wdeprecated -Wextra -Wpedantic -Wconversion -Werror)
set(EXP_COMPILE_OPTIONS ${EXP_WARNINGS})
set(EXP_LINK_OPTIONS)

execute_process(
  COMMAND git rev-parse HEAD 
  OUTPUT_VARIABLE EXP_GIT_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ($<CONFIG:Debug> OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(EXP_DEBUG 1)
else()
  set(EXP_DEBUG 0)
endif()

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
add_subdirectory(${LIBEXP_SOURCE_DIR})

include(CTest)
add_subdirectory(${EXP_TEST_DIR})


