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

#include "file_wrapper.hpp"
#include <string>
#include <vector>

namespace mish {
class i_command {
protected:
  std::vector<std::string> m_args;

public:
  template <typename T> void add_argument(T &&arg) {
    m_args.emplace_back(std::forward<std::remove_reference_t<T>>(arg));
  };

  virtual bool run(const linux_fd::fd_type, const linux_fd::fd_type) = 0;
  virtual ~i_command() = default;
};

}; // namespace mish