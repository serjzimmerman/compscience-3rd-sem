/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "bison_command_parser.hpp"
#include "command.hpp"
#include "scanner.hpp"

#include <memory>
#include <variant>
#include <vector>

namespace command_parser {

class driver {
private:
  scanner m_scanner;
  parser  m_parser;

  friend class parser;
  friend class scanner;

public:
  bool m_success = true;

  driver() : m_scanner{}, m_parser{m_scanner, *this} {}

  bool parse() {
    m_success = true;
    m_parser.parse();
    return m_success;
  }

  void switch_input_stream(std::istream *is) { m_scanner.switch_streams(is, nullptr); }
};
} // namespace command_parser