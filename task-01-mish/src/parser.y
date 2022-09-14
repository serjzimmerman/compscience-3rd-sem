%require "3.2"
%language "c++"
%define api.value.type variant
%define api.token.constructor

%param {yyscan_t yyscanner}

%code requires {
#include <memory>
#include "token.yy.hh"
}

%token GREATER ">"
%token LESS "<"
%token PIPE "|"
%token CD_BUILTIN "cd"
%token PWD_BUILTIN "pwd"

%empty epsilon

%token<std::string> ID arg
%nterm<std::unique_ptr<mish::i_command>> command builtin_command command_arg_pack
%nterm<std::vector<std::string>> args

%%

arg :   ID |
        epsilon

args :  args arg |
        epsilon

builtin_command : CD_BUILTIN  |
                  PWD_BUILTIN

command : builtin_command |
          ID

command_arg_pack : command args

expr : command_arg_pack
