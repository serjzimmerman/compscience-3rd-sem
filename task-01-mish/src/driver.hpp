/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "parser.hpp"
#include <memory>
#include <optional>
#include <string>

#define YY_DECL yy::parser::symbol_type yylex(mish::driver &drv)
YY_DECL;

namespace mish {
class driver {
  std::string m_cw_string;

public:
  driver() {}
  driver(const std::string &p_str) : m_cw_string{p_str} {}
  driver(std::string &&p_str) : m_cw_string{std::move(p_str)} {}
};
} // namespace mish
