/*
 * File: bitset.c                                                              *
 * Project: support                                                            *
 * Created on: Wednesday, March 26th 2025                                      *
 * Author: Cade Weinberg                                                       *
 */

#include "support/bitset.h"

extern Bitset bitset_create();
extern void   bitset_set(Bitset *bitset, u8 index);
extern void   bitset_clear(Bitset *bitset, u8 index);
extern bool   bitset_check(Bitset *bitset, u8 index);
