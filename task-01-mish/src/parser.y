/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%skeleton "lalr1.cc"
%require "3.8"

%defines

%define api.token.raw
%define api.parser.class { parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { mish }

%code requires {
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "command.hpp"
#include <variant>

namespace mish {
  class scanner;
  class driver;
}

using namespace mish;

}

%code top
{

#include <iostream>
#include <string>

#include "driver.hpp"
#include "scanner.hpp"
#include "bison_mish_parser.hpp"

static mish::parser::symbol_type yylex(mish::scanner &p_scanner, mish::driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%lex-param { mish::scanner &scanner }
%lex-param { mish::driver &driver }
%parse-param { mish::scanner &scanner }
%parse-param { mish::driver &driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

/* Signle letter tokens */
%token REDIR_OUT ">"
%token REDIR_IN "<"
%token PIPE "|"
%token BUILTIN_CD "cd"
%token BUILTIN_PWD "pwd"

/* Terminals */
%token <std::string> IDENTIFIER "identifier"

%type <std::vector<std::string>> arguments
%type <std::unique_ptr<mish::i_command>> command
%type <std::variant<std::vector<std::unique_ptr<mish::i_command>>, std::unique_ptr<mish::i_command>>> program

%start all

%%

all: program                        { driver.m_parsed = std::move($1); }

program:  program PIPE command      { std::get<0>($$)= std::move(std::get<0>($1)); std::get<0>($$).push_back(std::move($3)); }
          | command                 { std::get<0>($$).push_back(std::move($1)); }
          | BUILTIN_CD IDENTIFIER   { $$ = std::make_unique<mish::cd_command>($2); }
          | BUILTIN_PWD             { $$ = std::make_unique<mish::pwd_command>(); }

command:  IDENTIFIER arguments      { $$ = std::make_unique<mish::generic_command>($1, std::move($2)); }

arguments:  arguments IDENTIFIER    { $$ = std::move($1); $$.push_back($2); }
            | %empty                { }
            
%%

// Bison expects us to provide implementation - otherwise linker complains
void mish::parser::error(const std::string &message) {
  std::cout << "Error: " << message << "\n";
  driver.m_success = false;
}