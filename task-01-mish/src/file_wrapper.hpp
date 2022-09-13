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

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include <memory>

namespace mish {
class linux_fd {
public:
  using fd_type = int;

private:
  fd_type m_desc;

public:
  linux_fd() : m_desc(-1) {}
  linux_fd(fd_type p_desc) : m_desc{p_desc < 0 ? -1 : p_desc} {}
  linux_fd(const linux_fd &) = delete; // clang-format off
  
  linux_fd(linux_fd &&other) { std::swap(m_desc, other.m_desc); }
  linux_fd &operator=(linux_fd &&other) { if (this != &other) std::swap(m_desc, other.m_desc); return *this; }; 
  ~linux_fd() { if (valid()) close(m_desc); } // clang-format on

  bool valid() const { return m_desc != -1; }

  operator bool() const { return valid(); }
  operator fd_type() const { return m_desc; }
};

}; // namespace mish