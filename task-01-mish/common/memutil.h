/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#ifndef MEMUTIL_H
#define MEMUTIL_H

#include "error.h"
#include <stdlib.h>

#define ALLOC_PTR_CHECK(pointer)                                               \
  do {                                                                         \
    if (!(pointer)) {                                                          \
      ERROR("Memory allocation failed\n");                                     \
    }                                                                          \
  } while (0)

#define CALLOC_CHECKED(pointer, count, size)                                   \
  do {                                                                         \
    (pointer) = calloc(count, size);                                           \
    if (!(pointer)) {                                                          \
      ERROR("Cannot allocate enough memory, of total size: %lu\n",             \
            (count) * (size));                                                 \
    }                                                                          \
  } while (0)

#endif