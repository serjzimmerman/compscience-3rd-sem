%option noyywrap

%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.y.h"
#include "memutil.h"
%}

WS        [\t\n\r ]
ID        [^\t\n\r ]

%%

"|"                       { return PIPE; }
">"                       { return REDIR_OUT; }
"<"                       { return REDIR_INP; }
"pwd"                     { return BUILTIN_CMD_PWD; }
"cd"                      { return BUILTIN_CMD_CD; }

{ID}+                     { CALLOC_CHECKED(yylval.strval, yyleng + 1, 1); strncpy(yylval.strval, yytext, yyleng); return ID; }
{WS}+                     { }

%%

void yy_set_input_string_(const char *str) {
  yy_scan_string(str);
}

void yy_delete_current_buffer_() {
  yy_delete_buffer(YY_CURRENT_BUFFER);
}