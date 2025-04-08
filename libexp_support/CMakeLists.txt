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
  ${EXP_LIBEXP_SUPPORT_INCLUDE_DIR}/support/config.h.in 
  ${EXP_LIBEXP_SUPPORT_INCLUDE_DIR}/support/config.h
)

set(EXP_SUPPORT_SOURCE_FILES
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/allocation.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/array_growth.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/bitset.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/cli_option_parser.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/hash.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/io.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/message.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/nearest_power.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/numeric_conversions.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/panic.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/process.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/random.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/scalar.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/string_view.c
  ${EXP_LIBEXP_SUPPORT_SOURCE_DIR}/support/string.c
)

add_library(exp_support
  ${EXP_SUPPORT_SOURCE_FILES}
)
target_include_directories(exp_support PUBLIC 
${EXP_LIBEXP_SUPPORT_INCLUDE_DIR}
)
target_compile_options(exp_support PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_support PUBLIC ${EXP_LINK_OPTIONS})

