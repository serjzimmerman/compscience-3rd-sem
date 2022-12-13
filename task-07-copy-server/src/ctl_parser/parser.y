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
%require "3.5"

%defines

%define api.token.raw
%define api.parser.class { parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { command_parser }

%code requires {
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "command.hpp"
#include <variant>

namespace command_parser {
  class scanner;
  class driver;
}

using namespace command_parser;

}

%code top
{

#include <iostream>
#include <string>

#include "command_parser/driver.hpp"
#include "command_parser/scanner.hpp"
#include "bison_command_parser.hpp"

static command_parser::parser::symbol_type yylex(command_parser::scanner &p_scanner, command_parser::driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%lex-param { command_parser::scanner &scanner }
%lex-param { command_parser::driver &driver }
%parse-param { command_parser::scanner &scanner }
%parse-param { command_parser::driver &driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

/* Signle letter tokens */
%token REGISTER "register"
%token STATUS "status"

%token EOF 0 "end of file"

/* Terminals */
%token <std::string> STRING "string"

%type <command_parser::register_command> register_command
%type <command_parser::status_command> status_command

%start all

%%

all: command                              { driver.m_success = true; }

command:  register_command                { }
          | status_command                { }

register_command: REGISTER STRING STRING  { $$ = {$2, $3}; }

status_command: STATUS                    { $$ = {}; }

%%

// Bison expects us to provide implementation - otherwise linker complains
void command_parser::parser::error(const std::string &msg) {
  driver.m_success = false;
}