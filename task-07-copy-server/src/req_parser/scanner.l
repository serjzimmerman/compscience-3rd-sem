/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

%{
#include <iostream>
#include <numeric>

#include "bison_req_parser.hpp"
#include "req_parser/scanner.hpp"

using namespace req_parser;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="req_parser_"
%option yyclass="scanner"

whitespace  [ \t\n\r]
quoted_string ["]([^"\\\n]|\\(.|\n))*["]

%%

"get"           { return parser::make_GET(); }

{whitespace}+   { }
<<EOF>>         { return parser::make_EOF(); }

{quoted_string} { return parser::make_STRING(yytext); }
.               { throw parser::syntax_error{""}; }

%%
