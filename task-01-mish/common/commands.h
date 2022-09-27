/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "list.h"

#ifndef COMMANDS_H
#define COMMANDS_H

enum command_type_e {
  COMMAND_GENERIC,
  COMMAND_BUILTIN_PWD,
  COMMAND_BUILTIN_CD,
};

LIST_TEMPLATE(command_arg_list, char *);

typedef struct {
  enum command_type_e type;
  const char         *path;
  command_arg_list   *args;
} command_t;

static inline command_t *command_init() {
  command_t *cmd;
  CALLOC_CHECKED(cmd, 1, sizeof(command_t));
  return cmd;
}

static inline command_t *command_pwd() {
  command_t *cmd = command_init();
  cmd->type = COMMAND_BUILTIN_PWD;
  cmd->path = NULL;
  cmd->args = NULL;
  return cmd;
}

static inline command_t *command_cd(const char *path) {
  command_t *cmd = command_init();
  cmd->type = COMMAND_BUILTIN_CD;
  cmd->path = NULL;
  cmd->args = command_arg_list_init();
  
  command_arg_list_node *node = command_arg_list_node_init();
  node->value = path;
  command_arg_list_push_back(cmd->args, node);

  return cmd;
}

static inline command_t *command_generic(const char *path, command_arg_list *args) {
  command_t *cmd = command_init();
  cmd->type = COMMAND_GENERIC;
  cmd->path = path;
  cmd->args = args;
  return cmd;
}

static inline void command_free(command_t *cmd) {
  if (!cmd) return;

  if (cmd->args) {
    command_arg_list_free(cmd->args);
  }

  free(cmd->path);
  free(cmd->args);
  free(cmd);
}

LIST_TEMPLATE(command_list, command_t *);

typedef struct {
  const char   *input, *output;
  command_list *program;
} unit_program_t;

static inline unit_program_t *unit_program_init() {
  unit_program_t *unit;
  CALLOC_CHECKED(unit, 1, sizeof(unit_program_t));
  return unit;
}

#endif