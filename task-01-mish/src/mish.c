#include "error.h"
#include "parser.y.h"
#include "scanner.l.h"
#include <linux/limits.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "list.h"

LIST_TEMPLATE(list, int);

void error_callback(void **result, const char *str, va_list va) {
  WARNING("Parser encountered an error, aborting...\n");
}

int execute_pwd() {
  const char *path = getcwd(NULL, PATH_MAX);
  printf("%s\n", path);
  free(path);
}

int execute_cd(const char *path) {
  ASSERTION(path);

  if (chdir(path) == -1) {
    perror("cd: ");
    return -1;
  }

  return 0;
}

int execute_generic(command_t *cmd) {
  ASSERTION(cmd);
  ASSERTION(cmd->type == COMMAND_GENERIC);

  char **args;
  CALLOC_CHECKED(args, cmd->args->length + 2, sizeof(char *));

  unsigned i = 0;
  
  for (command_arg_list_node *node = cmd->args->first; node != NULL; node = node->next, ++i) {
    args[i + 1] = node->value;
  }

  args[0] = cmd->path;

  if (execvp(cmd->path, args) == -1) {
    perror("mish: ");
    return -1;
  }

  free(args);
  return 0;
}

int main() {
  while (1) {
    const char   *str = readline(NULL);
    command_list *lst;

    int pip[2][2];
    if (pipe(&pip[0][0]) == -1) {
      perror("mish: ");
      exit(EXIT_FAILURE);
    }
    if (pipe(&pip[1][0]) == -1) {
      perror("mish: ");
      exit(EXIT_FAILURE);
    }

    yy_set_input_string_(str);
    yyparse(&lst);
    yy_delete_current_buffer_();

    unsigned i = 0;
    for (command_list_node *node = lst->first; node != NULL; node = node->next, i++) {
      int child;

      command_t *cmd = node->value;
      switch (cmd->type) {
      case COMMAND_BUILTIN_CD: execute_cd(cmd->args->first->value); continue;
      case COMMAND_BUILTIN_PWD: execute_pwd(); continue;
      case COMMAND_GENERIC: goto main_generic_command;
      default: ERROR("Whatever happened?!\n"); break;
      }

    main_generic_command:
      if ((child = fork()) == 0) {
        close(pip[i & 1][1]);
        close(pip[(i + 1) & 1][0]);

        if (i != 0) dup2(pip[i & 1][0], 0);
        if (i != lst->length - 1) dup2(pip[(i + 1) & 1][1], 1);

        execute_generic(cmd);
      }

      if (child == -1) {
        perror("mish: ");
      }

      int exit_code;
      waitpid(child, &exit_code, 0);
    }

    close(pip[0][0]);
    close(pip[0][1]);
    close(pip[1][0]);
    close(pip[1][1]);
  }
}