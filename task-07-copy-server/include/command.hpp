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

#include <string>
#include <variant>

namespace command_parser {

struct register_command {
  std::string rx_fifo_name;
  std::string tx_fifo_name;
};

struct status_command {};

using var_command = std::variant<std::monostate, status_command, register_command>;

} // namespace command_parser