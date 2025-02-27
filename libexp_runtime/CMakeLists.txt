# Copyright (C) 2024 Cade Weinberg
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

set(EXP_LIBSTART_SOURCES
  ${EXP_LIBEXP_RUNTIME_SOURCE_DIR}/exp_start.s
)

add_library(exp_runtime_start
  ${EXP_LIBSTART_SOURCES}
)

set(EXP_LIBEXP_RUNTIME_SOURCES
  ${EXP_LIBEXP_RUNTIME_SOURCE_DIR}/exp_syscalls.s
  ${EXP_LIBEXP_RUNTIME_SOURCE_DIR}/exp_copy.s
)

add_library(exp_runtime
  ${EXP_LIBEXP_RUNTIME_SOURCES}
)



