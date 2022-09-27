%language "c"

%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(void **result, const char *s);
extern void *ast_current_root;

%}

%code requires {
#include "commands.h"
}

%define parse.error custom
%union {
  char *strval;
  command_arg_list *args;
  command_list *commands;
  command_t *cmd;
}

%token REDIR_OUT
%token REDIR_INP
%token PIPE

%token BUILTIN_CMD_PWD
%token BUILTIN_CMD_CD

%token<strval> ID 

%nterm<args> arguments
%nterm<cmd> command
%nterm<commands> program

%initial-action { };
%parse-param { void **result }

%start unit

%%

unit:   program                       { *result = $1; }
;

program:  program PIPE command        { $$ = $1; command_list_node *node = command_list_node_init(); node->value = $3; command_list_push_back($$, node); }
          | command                   { $$ = command_list_init(); command_list_node *node = command_list_node_init(); node->value = $1; command_list_push_back($$, node); }
;

command:  BUILTIN_CMD_CD ID           { $$ = command_cd($2); }
          | BUILTIN_CMD_PWD           { $$ = command_pwd(); }
          | ID arguments              { $$ = command_generic($1, $2); }
;

arguments: arguments ID               { command_arg_list_node *node = command_arg_list_node_init(); node->value = $2; command_arg_list_push_back($1, node); $$ = $1; }
          | ID                        { $$ = command_arg_list_init(); command_arg_list_node *node = command_arg_list_node_init(); node->value = $1; command_arg_list_push_back($$, node);}
          | %empty                    { $$ = NULL; }
;

%%

static int yyreport_syntax_error (const yypcontext_t *yyctx, void **result) {
  // AST_REPORT_ERROR(result, "Parse error\n");
}

void yyerror(void **result, const char *s) {

}