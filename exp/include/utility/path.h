// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_FILESYSTEM_PATH_H
#define EXP_FILESYSTEM_PATH_H

#include "utility/string.h"

typedef struct Path {
  String string;
} Path;

/**
 * @brief Create a new empty path
 *
 * @return Path
 */
Path path_create();

// #TODO:
// Path path_create_from_string(String *string);
// Path path_create_from_string_view(StringView *string_view);
// Path path_create_from_string_literal(const char *literal, u64 length);

/**
 * @brief Free a given path
 *
 * @param path
 */
void path_destroy(Path *restrict path);

/**
 * @brief returns a StringView of the path
 *
 * @param path
 * @return StringView
 */
StringView path_to_view(const Path *restrict path);

/**
 * @brief returns if the path is empty.
 *
 * @param path
 * @return true
 * @return false
 */
bool path_empty(Path const *restrict path);

/**
 * @brief assigns <data> to <path>
 *
 * @param path
 * @param data
 * @param length
 */
void path_assign(Path *restrict path, char const *restrict data);

/**
 * @brief clone the given path
 *
 * @param path
 * @return Path
 */
Path path_clone(Path *restrict path);

/**
 * @brief compare two paths lexicographically
 *
 * @param p1
 * @param p2
 * @return i32
 */
i32 path_compare(Path const *restrict p1, Path const *restrict p2);

/**
 * @brief appends the given path, placing a
 * directory separator between
 *
 * @param p1 the destination
 * @param p2 the source
 */
void path_append(Path *restrict p1, const Path *restrict p2);

/**
 * @brief appends the given path, not placing a
 * directory separator between
 *
 * @param p1 the destination
 * @param p2 the source
 */
void path_concat(Path *restrict p1, const Path *restrict p2);

/**
 * @brief if the path refers to a file, replaces the
 * files extension with the given path's extension.
 *
 * @param p1 the destination
 * @param p2 the source
 */
void path_replace_extension(Path *restrict p1, const Path *restrict p2);

// /**
//  * @brief if the path refers to a file, removes
//  * the filename section of the path.
//  *
//  * @param p1 the target
//  */
// void path_remove_filename(Path *restrict p1);

// /**
//  * @brief if the path refers to a file, replaces
//  * the filename section with the given path.
//  * else equivalent to path_concat
//  *
//  * @param p1 the destination
//  * @param p2 the source
//  */
// void path_replace_filename(Path *restrict p1, const Path *restrict p2);

// /**
//  * @brief return the root-name of the path, if present.
//  * otherwise returns an empty path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_root_name(Path const *restrict path);

// /**
//  * @brief returns the root directory of the path, if present,
//  * otherwise returns an empty path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_root_directory(Path const *restrict path);

// /**
//  * @brief returns the root path of the path, if present.
//  * otherwise returns an empty path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_root_path(Path const *restrict path);

// /**
//  * @brief returns the path relative to the root path.
//  *
//  * @param path
//  * @return Path
//  */
// Path path_relative_path(Path const *restrict path);

// /**
//  * @brief returns the path of the parent path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_parent_path(Path const *restrict path);

// /**
//  * @brief returns the filename component of the path,
//  * if any. otherwise returns an empty path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_filename(Path const *restrict path);

// /**
//  * @brief returns the filename of the path,
//  * without an extension.
//  *
//  * @param path
//  * @return Path
//  */
// Path path_stem(Path const *restrict path);

// /**
//  * @brief returns the extension of the path, if present.
//  * otherwise returns an empty path
//  *
//  * @param path
//  * @return Path
//  */
// Path path_extension(Path const *restrict path);

// bool path_has_root_path(Path const *restrict path);
// bool path_has_root_name(Path const *restrict path);
// bool path_has_root_directory(Path const *restrict path);
// bool path_has_relative_path(Path const *restrict path);
// bool path_has_parent_path(Path const *restrict path);
// bool path_has_filename(Path const *restrict path);
// bool path_has_stem(Path const *restrict path);
// bool path_has_extension(Path const *restrict path);

// bool path_is_absolute(Path const *restrict path);
// bool path_is_relative(Path const *restrict path);

#endif // !EXP_FILESYSTEM_PATH_H