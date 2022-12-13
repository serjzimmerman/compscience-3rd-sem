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

#include "bison_command_parser.hpp"
#include "scanner.hpp"

using namespace command_parser;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="command_parser_"
%option yyclass="scanner"

whitespace  [ \t\n\r]
quoted_string ["]([^"\\\n]|\\(.|\n))*["]

%%

"register"      { return parser::make_REGISTER(); }
"status"        { return parser::make_STATUS(); }

{whitespace}+   { }
<<EOF>>         { return parser::make_EOF(); }

{quoted_string} { return parser::make_STRING(yytext); }
.               { throw parser::syntax_error{""}; }

%%
