#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>

#include <mutex>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;

#include <boost/thread.hpp>

extern "C" {
#include <sys/wait.h>
#include <unistd.h>
}

#include <algorithm>
#include <optional>

double calculate_integral(std::size_t n_points, std::pair<double, double> x_span, std::pair<double, double> y_span,
                          auto function, unsigned subdiv = 1) {
  if (x_span.first > x_span.second) std::swap(x_span.first, x_span.second);
  if (y_span.first > y_span.second) std::swap(y_span.first, y_span.second);

  auto grid_size_x = (x_span.second - x_span.first) / subdiv;
  auto grid_size_y = (y_span.second - y_span.first) / subdiv;

  std::mutex  mutex;
  std::size_t below = 0;

  auto calculation = [&mutex, &below, function](std::pair<double, double> p_x_span, std::pair<double, double> p_y_snap,
                                                std::size_t points) {
    std::random_device rd;
    std::mt19937       re{rd()};

    std::uniform_real_distribution dis_x{p_x_span.first, p_x_span.second};
    std::uniform_real_distribution dis_y{p_y_snap.first, p_y_snap.second};

    std::size_t count = 0;

    for (std::size_t i = 0; i < points; ++i) {
      auto [x, y] = std::pair{dis_x(re), dis_y(re)};

      auto value = function(x);
      if (std::abs(y) <= std::abs(value)) count++;
    }

    const std::lock_guard<std::mutex> lock(mutex);
    below += count;
  };

  auto points_per_thread = n_points / (subdiv * subdiv);
  auto total_points = points_per_thread * (subdiv * subdiv);

  boost::thread_group group;
  for (std::size_t i = 0; i < subdiv; ++i) {
    for (std::size_t j = 0; j < subdiv; ++j) {
      auto thread_x_span = std::pair{x_span.first + grid_size_x * i, x_span.first + grid_size_x * (i + 1)};
      auto thread_y_span = std::pair{y_span.first + grid_size_y * j, y_span.first + grid_size_y * (j + 1)};

      group.add_thread(new boost::thread{[calculation, thread_x_span, thread_y_span, points_per_thread]() {
        calculation(thread_x_span, thread_y_span, points_per_thread);
      }});
    }
  }
  group.join_all();

  return static_cast<double>(below) / total_points * (x_span.second - x_span.first) * (y_span.second - y_span.first);
}

double calculate_integral(std::size_t n_points, std::pair<double, double> x_span, auto function, unsigned subdiv = 1) {
  return calculate_integral(n_points, x_span, {function(x_span.first), function(x_span.second)}, function, subdiv);
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

  std::cout << 4 * calculate_integral(
                   100000000, {0.0, 1}, [](auto val) { return std::sqrt(1 - val * val); }, 3)
            << "\n";
}