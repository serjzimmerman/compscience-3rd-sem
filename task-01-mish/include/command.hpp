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

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

extern "C" {
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
}

namespace mish {

struct i_visitor;

struct i_command {
  bool m_need_redirect;

  i_command(bool n) : m_need_redirect(n) {}

  virtual bool execute(std::optional<std::string>, std::optional<std::string>) = 0;
  virtual ~i_command(){};
};

struct generic_command : i_command {
  std::string              m_command_name;
  std::vector<std::string> m_args;

  generic_command(std::string name, std::vector<std::string> &&args)
      : i_command{true}, m_command_name{name}, m_args{args} {}

  bool execute(std::optional<std::string> iput, std::optional<std::string> oput) override {
    int                 child;
    std::vector<char *> args;
    args.push_back(const_cast<char *>(m_command_name.c_str()));
    for (auto &v : m_args) {
      args.push_back(const_cast<char *>(v.c_str()));
    }
    args.push_back(nullptr);

    std::cerr << "here2\n";

    int fdo, fdi;
    if (iput) {
      fdo = open(iput.value().c_str(), O_RDONLY);
    }

    if (oput) {
      fdi = open(oput.value().c_str(), O_RDONLY);
    }

    if ((child = fork()) == 0) {
      if (iput) {
        int fdi = open(iput.value().c_str(), O_RDONLY);
        dup2(fdi, STDIN_FILENO);
        close(fdi);
      }

      if (oput) {
        int fdo = open(oput.value().c_str(), O_WRONLY);
        dup2(fdo, STDOUT_FILENO);
        close(fdo);
      }

      std::cerr << "herefork\n";
      std::cerr << "executing " << m_command_name << "\n";

      if (execvp(m_command_name.c_str(), args.data()) == -1) {
        perror("mish: ");
        exit(1);
      }
    }

    int ret;
    std::cerr << "Before wait\n";
    waitpid(child, &ret, 0);

    if (iput) {
      close(fdo);
    }

    if (oput) {
      close(fdi);
    }

    std::cerr << "After wait\n";
    return !ret;
  }
};

struct cd_command : i_command {
  std::string m_arg;

  cd_command(std::string arg) : i_command{false}, m_arg{arg} {}

  bool execute(std::optional<std::string>, std::optional<std::string>) override {
    if (chdir(m_arg.c_str()) == -1) {
      perror("cd: ");
      return false;
    }
    return true;
  }
};

struct pwd_command : i_command {
  pwd_command() : i_command{false} {}

  bool execute(std::optional<std::string>, std::optional<std::string> oput) override {
    auto  freer = [](char *ptr) { free(static_cast<void *>(ptr)); };
    char *path_string = getcwd(NULL, PATH_MAX);

    if (!path_string) return false;

    auto path = std::unique_ptr<char, decltype(freer)>{path_string, freer};

    if (oput) {
      std::ofstream os{oput.value()};
      if (!os.good()) return false;
      os << path.get() << "\n";
    } else {
      std::cout << path.get() << "\n";
    }

    return true;
  }
};

} // namespace mish