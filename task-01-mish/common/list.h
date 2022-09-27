/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "error.h"
#include "memutil.h"

#ifndef LIST_H
#define LIST_H

#define LIST_STRUCT_NAME__(NAME) NAME##_s__
#define LIST_TYPE_NAME__(NAME) NAME

#define LIST_NODE_STRUCT_NAME__(NAME) NAME##_node_s__

#define LIST_NODE_INIT_FUNCTION_NAME__(NAME) NAME##_node##_init
#define LIST_FREE_FUNCTION_NAME__(NAME) NAME##_free
#define LIST_INIT_FUNCTION_NAME__(NAME) NAME##_init
#define LIST_PUSH_BACK_FUNCTION_NAME__(NAME) NAME##_push_back
#define LIST_INITS_FUNCTION_NAME__(NAME) NAME##_inits

#define LIST_OPERATIONS__(NAME, T)                                                                                     \
  static inline struct LIST_NODE_STRUCT_NAME__(NAME) * LIST_NODE_INIT_FUNCTION_NAME__(NAME)() {                        \
    struct LIST_NODE_STRUCT_NAME__(NAME) * node__;                                                                     \
    CALLOC_CHECKED(node__, 1, sizeof(struct LIST_NODE_STRUCT_NAME__(NAME)));                                           \
    return node__;                                                                                                     \
  }                                                                                                                    \
                                                                                                                       \
  static inline void LIST_FREE_FUNCTION_NAME__(NAME)(NAME * list__) {                                                  \
    ASSERTION(list__);                                                                                                 \
    struct LIST_NODE_STRUCT_NAME__(NAME) *curr__ = list__->first, *prev__ = NULL;                                      \
    while (curr__) {                                                                                                   \
      free(prev__);                                                                                                    \
      prev__ = curr__;                                                                                                 \
      curr__ = curr__->next;                                                                                           \
    }                                                                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  static inline void LIST_PUSH_BACK_FUNCTION_NAME__(NAME)(NAME * list__, NAME##_node * node__) {                       \
    ASSERTION(list__);                                                                                                 \
    ASSERTION(node__);                                                                                                 \
    list__->length++;                                                                                                  \
    if (!list__->first) {                                                                                              \
      list__->first = list__->last = node__;                                                                           \
      return;                                                                                                          \
    }                                                                                                                  \
    list__->last->next = node__;                                                                                       \
    list__->last = node__;                                                                                             \
  }                                                                                                                    \
                                                                                                                       \
  static inline NAME LIST_INITS_FUNCTION_NAME__(NAME)() {                                                              \
    NAME list__;                                                                                                       \
    list__.first = list__.last = NULL;                                                                                 \
    list__.length = 0;                                                                                                 \
    return list__;                                                                                                     \
  }                                                                                                                    \
  static inline NAME *LIST_INIT_FUNCTION_NAME__(NAME)() {                                                              \
    NAME *list__;                                                                                                      \
    CALLOC_CHECKED(list__, 1, sizeof(NAME));                                                                           \
    return list__;                                                                                                     \
  }

#define LIST_TEMPLATE(NAME, T)                                                                                         \
  typedef struct LIST_NODE_STRUCT_NAME__(NAME) {                                                                       \
    struct LIST_NODE_STRUCT_NAME__(NAME) * next;                                                                       \
    T value;                                                                                                           \
  } NAME##_node;                                                                                                       \
                                                                                                                       \
  typedef struct LIST_STRUCT_NAME__(NAME) {                                                                            \
    struct LIST_NODE_STRUCT_NAME__(NAME) * first, *last;                                                               \
    unsigned length;                                                                                                   \
  } NAME;                                                                                                              \
  LIST_OPERATIONS__(NAME, T)

#endif