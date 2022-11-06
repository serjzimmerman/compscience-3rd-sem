#include "command.hpp"
#include "driver.hpp"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

static char *fifo_1 = "/tmp/mish_fifo1";
static char *fifo_2 = "/tmp/mish_fifo2";

bool execute(std::vector<std::unique_ptr<mish::i_command>> &&vec) {
  for (unsigned i = 0; i < vec.size(); ++i) {
    std::optional<std::string> iput;
    std::optional<std::string> oput;

    if (i != 0) {
      iput = fifo_1;
    }

    if (i != vec.size() - 1) {
      oput = fifo_2;
    }

    if (!vec[i]->execute(iput, oput)) {
      return false;
    }

    std::swap(fifo_1, fifo_2);
  }

  return true;
}

int main() {
  mkfifo(fifo_1, 0666);
  mkfifo(fifo_2, 0666);

  auto fd1 = open(fifo_1, O_WRONLY | O_NONBLOCK);
  auto fd2 = open(fifo_2, O_WRONLY | O_NONBLOCK);
  close(fd1);
  close(fd2);

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
    if (!result) continue;

    execute(std::move(drv.m_parsed));
  }

  remove(fifo_1);
  remove(fifo_2);

  std::unique_ptr<mish::i_command> pwd = std::make_unique<mish::pwd_command>();
  pwd->execute(std::nullopt, std::nullopt);
}