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

#include "bison_mish_parser.hpp"
#include "scanner.hpp"

using namespace mish;
%}

%option noyywrap nounput noinput nodefault 
%option c++
%option prefix="mish_"
%option yyclass="scanner"

whitespace  [ \t\n\r]
string      [^ \t\n\r]+

%%

">"             { return parser::make_REDIR_OUT(); }
"<"             { return parser::make_REDIR_IN(); }
"|"             { return parser::make_PIPE(); }
"pwd"           { return parser::make_BUILTIN_PWD(); }
"cd"            { return parser::make_BUILTIN_CD(); }

{whitespace}+   { }
<<EOF>>         { return parser::make_YYEOF(); }

{string}        { return parser::make_IDENTIFIER(yytext); }

%%
