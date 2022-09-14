/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "builtin_commands.hpp"
#include <array>
#include <linux/limits.h>
#include <optional>
#include <stdexcept>
#include <string_view>

extern "C" {
#include <sys/stat.h>
}

namespace mish {
using path_type = std::array<char, PATH_MAX>;

std::optional<path_type> working_dir() {
  path_type cwd;

  if (!getcwd(cwd.data(), cwd.size())) {
    return std::nullopt;
  }

  return cwd;
}

bool cd_command::run(const linux_fd::fd_type input, const linux_fd::fd_type output) {
  if (m_args.size() < 1) return false;

  const auto &folder = m_args[0];
  struct stat st;
  std::string output_msg;

  if (stat(folder.c_str(), &st) != 0) {
    const char no_file_msg[] = "cd: no such file or directory: ";
    output_msg += no_file_msg + folder + "\n";
    write(output, output_msg.c_str(), output_msg.length());
    return false;
  }

  if (!S_ISDIR(st.st_mode)) {
    const char not_a_dir_msg[] = "cd: not a directory: ";
    output_msg += not_a_dir_msg + folder + "\n";
    write(output, output_msg.c_str(), output_msg.length());
    return false;
  }

  if (chdir(folder.c_str())) {
    write(output, "cd: unknown error", 17);
    return false;
  }

  return true;
}

bool pwd_command::run(const linux_fd::fd_type input, const linux_fd::fd_type output) {
  auto cwd = working_dir();

  if (!cwd) {
    std::string_view msg = "pwd: unknown error";
    write(output, msg.data(), msg.length());
    return false;
  }

  std::string_view s = cwd.value().data();
  return (write(output, s.data(), s.length()) == s.length());
}
} // namespace mish