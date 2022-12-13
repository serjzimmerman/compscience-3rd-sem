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
%define api.namespace { req_parser }

%code requires {
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "request.hpp"
#include <variant>

namespace req_parser {
  class scanner;
  class driver;
}

using namespace req_parser;

}

%code top
{

#include <iostream>
#include <string>

#include "req_parser/driver.hpp"
#include "req_parser/scanner.hpp"
#include "bison_req_parser.hpp"

static req_parser::parser::symbol_type yylex(req_parser::scanner &p_scanner, req_parser::driver &p_driver) {
  return p_scanner.get_next_token();
}

}

%lex-param { req_parser::scanner &scanner }
%lex-param { req_parser::driver &driver }
%parse-param { req_parser::scanner &scanner }
%parse-param { req_parser::driver &driver }

%define parse.trace
%define parse.error verbose
%define api.token.prefix {TOKEN_}

/* Signle letter tokens */
%token GET "get"

%token EOF 0 "end of file"

/* Terminals */
%token <std::string> STRING "string"

%type <req_parser::get_request> get_request

%start all

%%

all: command                              { driver.m_success = true; }

command:  get_request                     { }

get_request: GET STRING                   { $$ = {$2}; }

%%

// Bison expects us to provide implementation - otherwise linker complains
void req_parser::parser::error(const std::string &msg) {
  driver.m_success = false;
}