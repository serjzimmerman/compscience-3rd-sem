/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer mish_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL mish::parser::symbol_type mish::scanner::get_next_token()

#include "bison_mish_parser.hpp"

namespace mish {

class scanner : public yyFlexLexer {
private:
public:
  scanner() {}
  mish::parser::symbol_type get_next_token();
};

} // namespace mish