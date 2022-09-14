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
#include "icommand.hpp"

namespace mish {
class cd_command final : public i_command {
public:
  bool run(const linux_fd::fd_type, const linux_fd::fd_type) override;
};

class pwd_command final : public i_command {
public:
  bool run(const linux_fd::fd_type, const linux_fd::fd_type) override;
};
}; // namespace mish