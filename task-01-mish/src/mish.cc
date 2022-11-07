#include "command.hpp"
#include "driver.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

static constexpr unsigned RD_PIPE = 0;
static constexpr unsigned WR_PIPE = 1;

std::optional<int> spawn_process(int in, int out, mish::i_command *cmd) {
  int pid;
  if ((pid = fork()) == 0) {
    if (in != STDIN_FILENO) {
      dup2(in, STDIN_FILENO);
      close(in);
    }

    if (out != STDOUT_FILENO) {
      dup2(out, STDOUT_FILENO);
      close(out);
    }

    if (!cmd->execute()) exit(EXIT_FAILURE);
  }

  else if (pid == -1) {
    perror("mish: ");
    return std::nullopt;
  }

  return pid;
}

bool execute(std::vector<std::unique_ptr<mish::i_command>> &&vec) {
  std::array<int, 2> fd;
  int                in = 0;
  std::vector<int>   pids;
  bool               success = true;

  for (unsigned i = 0; i < vec.size(); ++i) {
    if (i != vec.size() - 1) {
      if (pipe(fd.data()) < 0) {
        perror("mish: ");
        success = false;
        break;
      }

      auto possible_pid = spawn_process(in, fd[WR_PIPE], vec[i].get());
      if (!possible_pid) {
        success = false;
        break;
      }

      pids.push_back(possible_pid.value());

      close(fd[WR_PIPE]);
      in = fd[RD_PIPE];
      continue;
    }

    // Last iteration
    auto possible_pid = spawn_process(in, STDOUT_FILENO, vec[i].get());
    if (!possible_pid) {
      success = false;
      break;
    }

    pids.push_back(possible_pid.value());
  }

  int ret;
  for (const auto &v : pids) {
    waitpid(v, &ret, 0);
    if (ret != 0) success = false;
  }

  return success;
}

int main() {
  std::cout << "[mish]$ ";

  while (std::cin.good()) {
    std::string line;
    std::getline(std::cin, line);

    if (line.size() == 0 && !std::cin.good()) {
      return 0;
    }

    mish::driver       drv{};
    std::istringstream iss{line};

    drv.switch_input_stream(&iss);
    auto result = drv.parse();
    if (!result) {
      std::cout << "[mish]$ ";
      continue;
    }

    if (std::holds_alternative<std::unique_ptr<mish::i_command>>(drv.m_parsed)) {
      std::get<1>(drv.m_parsed)->execute();
    } else {
      execute(std::move(std::get<0>(drv.m_parsed)));
    }

    std::cout << "[mish]$ ";
  }
}