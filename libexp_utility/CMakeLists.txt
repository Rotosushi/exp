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

set(EXP_UTILITY_SOURCE_FILES
  ${EXP_SOURCE_DIR}/utility/allocation.c
  ${EXP_SOURCE_DIR}/utility/array_growth.c
  ${EXP_SOURCE_DIR}/utility/hash.c
  ${EXP_SOURCE_DIR}/utility/int_types.c
  ${EXP_SOURCE_DIR}/utility/io.c
  ${EXP_SOURCE_DIR}/utility/log.c
  ${EXP_SOURCE_DIR}/utility/nearest_power.c
  ${EXP_SOURCE_DIR}/utility/numeric_conversions.c
  ${EXP_SOURCE_DIR}/utility/panic.c
  ${EXP_SOURCE_DIR}/utility/process.c
  ${EXP_SOURCE_DIR}/utility/string_view.c
  ${EXP_SOURCE_DIR}/utility/string.c
)

add_library(exp_utility
  ${EXP_UTILITY_SOURCE_FILES}
)
target_include_directories(exp_utility PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_utility PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_utility PUBLIC ${EXP_LINK_OPTIONS})

