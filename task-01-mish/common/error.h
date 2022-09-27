/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include <stdio.h>

#if defined(DEBUG)

#define WARNING(format, ...) MESSAGE("Warning", format, ##__VA_ARGS__);
#define ERROR(format, ...)                                                     \
  do {                                                                         \
    MESSAGE("Fatal error", format, ##__VA_ARGS__);                             \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#else
#define WARNING(format, ...)                                                   \
  do {                                                                         \
  } while (0)
#define ERROR(format, ...)                                                     \
  do {                                                                         \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
#endif

#define MESSAGE(errtype, format, ...)                                          \
  do {                                                                         \
    fprintf(stderr, "%s: at line %d of %s in function %s\n" format, errtype,   \
            __LINE__, __FILE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);           \
  } while (0)

#define ASSERTION(condition)                                                   \
  if (!(condition)) {                                                          \
    do {                                                                       \
      MESSAGE("Assertion (" #condition ") failed", "");                        \
      exit(EXIT_FAILURE);                                                      \
    } while (0);                                                               \
  }
#endif
