#include <stdarg.h>
#include <stdio.h>

#include "builtin_commands.hpp"
#include "file_wrapper.hpp"
#include "icommand.hpp"

#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
  std::unique_ptr<mish::i_command> p = std::make_unique<mish::cd_command>();

  std::string arg;
  std::cin >> arg;
  p.get()->add_argument(arg);

  p.get()->run(0, 1);

  std::cout << get_current_dir_name();
}