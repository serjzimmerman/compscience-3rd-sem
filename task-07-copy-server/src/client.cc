#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <ratio>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;

#include "utils.hpp"

extern "C" {
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include <algorithm>
#include <optional>

int main(int argc, char *argv[]) {
  auto verbose = false;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("verbose,v", "Verbose output");
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_FAILURE;
  }

  verbose = vm.count("verbose");
}