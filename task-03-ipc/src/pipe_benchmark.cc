#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;

extern "C" {
#include <sys/wait.h>
#include <unistd.h>
}

#include <algorithm>
#include <optional>

using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

std::optional<std::chrono::duration<double, std::milli>> run_benchmark(unsigned bs, unsigned count) {
  std::vector<char> random_vector;
  random_vector.resize(bs);
  
  random_bytes_engine rbe;
  rbe.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::generate(random_vector.begin(), random_vector.end(), std::ref(rbe));

  std::array<int, 2> pipe;
  if (pipe2(pipe.data(), 0) == -1) {
    std::cerr << "Can't open pipe\n";
    return std::nullopt;
  }

  int child;
  if ((child = fork()) == 0) {
    close(pipe[1]);

    unsigned iter = 0;
    while (read(pipe[0], random_vector.data(), bs) != 0)
      ;

    exit(0);
  }

  auto start_all = std::clock();

  close(pipe[0]);
  for (unsigned i = 0; i < count; ++i) {
    write(pipe[1], random_vector.data(), bs);
  }
  close(pipe[1]);

  int exit_code;
  waitpid(child, &exit_code, 0);

  auto finish_all = std::clock();
  auto duration = std::chrono::duration<double, std::ratio<1, CLOCKS_PER_SEC>>(finish_all - start_all);

  return duration;
}

int main(int argc, char *argv[]) {
  unsigned bs, count;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("bs", po::value<unsigned>(&bs)->default_value(1024),
                                                          "Bytes to transfer in a single iteration")(
      "count", po::value<unsigned>(&count)->default_value(4096), "Total number of transfers");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  auto result = run_benchmark(bs, count);
  if (!result) {
    return EXIT_FAILURE;
  }

  unsigned long long total_count = bs * count;
  double             speed = static_cast<double>(total_count) / result->count();

  std::cout << "Total bytes transfered:\t\t" << total_count << "\nTime elapsed while copying:\t" << result->count()
            << " ms\nApproximate transfer speed:\t" << std::fixed << speed << "bytes/ms\n";
}