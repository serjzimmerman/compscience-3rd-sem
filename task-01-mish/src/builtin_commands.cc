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
#include <stdexcept>

extern "C" {
#include <sys/stat.h>
}

namespace mish {
void cd_command::run(const linux_fd::fd_type input, const linux_fd::fd_type output) {
  if (m_args.size() < 1) return;

  const auto &folder = m_args[0];
  struct stat st;
  std::string output_msg;

  if (stat(folder.c_str(), &st) != 0) {
    const char no_file_msg[] = "cd: no such file or directory: ";
    output_msg += no_file_msg + folder + "\n";
    write(output, output_msg.c_str(), output_msg.length());
    return;
  }

  if (!S_ISDIR(st.st_mode)) {
    const char not_a_dir_msg[] = "cd: not a directory: ";
    output_msg += not_a_dir_msg + folder + "\n";
    write(output, output_msg.c_str(), output_msg.length());
    return;
  }

  if (chdir(folder.c_str())) perror("cd: ");
}
} // namespace mish