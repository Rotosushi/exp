# SPDX-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Check check IMPORTED_TARGET)
