%skeleton "lalr1.cc" // -*- C++ -*-
%require "3.8.1"
%header

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
#include <string>
class driver;
}

// The parsing context.
%param { mish::driver& drv }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "driver.hpp"
}

%define api.token.prefix {TOK_}
%token
  LESS      "<"
  GREATER   ">"
  PIPE      "|"
;

%token <std::string>  "identifier"
%nterm <int> command

