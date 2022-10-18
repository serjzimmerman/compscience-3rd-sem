#include <algorithm>
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
                          auto function, unsigned subdiv_x = 1, unsigned subdiv_y = 1) {
  if (x_span.first > x_span.second) std::swap(x_span.first, x_span.second);
  if (y_span.first > y_span.second) std::swap(y_span.first, y_span.second);

  auto grid_size_x = (x_span.second - x_span.first) / subdiv_x;
  auto grid_size_y = (y_span.second - y_span.first) / subdiv_y;

  std::mutex mutex;
  long long  below = 0;

  auto calculation = [&mutex, &below, function](std::pair<double, double> p_x_span, std::pair<double, double> p_y_snap,
                                                std::size_t points) {
    std::random_device rd;
    std::mt19937       re{rd()};

    std::uniform_real_distribution dis_x{p_x_span.first, p_x_span.second};
    std::uniform_real_distribution dis_y{p_y_snap.first, p_y_snap.second};

    long long count = 0;

    for (std::size_t i = 0; i < points; ++i) {
      auto [x, y] = std::pair{dis_x(re), dis_y(re)};

      auto value = function(x);
      if (value < 0 && y > value)
        count--;
      else if (value > 0 && y < value)
        count++;
    }

    const std::lock_guard<std::mutex> lock(mutex);
    below += count;
  };

  auto points_per_thread = n_points / (subdiv_x * subdiv_y);
  auto total_points = points_per_thread * (subdiv_x * subdiv_y);

  boost::thread_group group;
  for (std::size_t i = 0; i < subdiv_x; ++i) {
    for (std::size_t j = 0; j < subdiv_y; ++j) {
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

double calculate_integral(std::size_t n_points, std::pair<double, double> x_span, auto function, unsigned subdiv_x = 1,
                          unsigned subdiv_y = 1) {
  return calculate_integral(n_points, x_span, {function(x_span.first), function(x_span.second)}, function, subdiv_x,
                            subdiv_y);
}

auto circle_equation = [](auto val) { return std::sqrt(1 - val * val); };

auto calculate_pi(std::size_t n_points, unsigned subdiv_x = 1, unsigned subdiv_y = 1) {
  return 4 * calculate_integral(n_points, {0, 1}, circle_equation, subdiv_x, subdiv_y);
}

void benchmark_pi(unsigned num_threads, std::size_t num_points, unsigned iters, bool verbose = false) {
  double pi_val;

  std::vector<double> results;
  results.resize(num_threads + 1, 0);

  if (!verbose) std::cout << std::fixed << "# thread \t\t time (s)\n";

  for (unsigned j = 0; j < iters; ++j) {
    if (verbose) {
      std::cout << "Iteration " << j << " :\n";
    }

    for (unsigned i = 1; i <= num_threads; ++i) {
      auto start = std::chrono::high_resolution_clock::now();
      pi_val = calculate_pi(num_points, i);
      auto finish = std::chrono::high_resolution_clock::now();
      results[i] = std::chrono::duration<double>{finish - start}.count();

      if (verbose) {
        std::cout << "Calculation on " << i << " threads took " << results[i] << " s, result: " << pi_val << "\n";
      }
    }
  }

  std::for_each(results.begin(), results.end(), [iters](auto &&val) { return val / iters; });
  for (unsigned i = 1; i < results.size(); ++i) {
    if (verbose) {
      std::cout << "Average time on " << i << " threads -- " << results[i] << " s\n";
    } else {
      std::cout << i << "\t\t\t" << results[i] << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  unsigned points, threads, iters;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")(
      "npoints,p", po::value<unsigned>(&points)->default_value(10000000), "Number of points to sample")(
      "nthreads,t", po::value<unsigned>(&threads)->default_value(16),
      "Benchmark up to threads")("niter,i", po::value<unsigned>(&iters)->default_value(10),
                                 "Number of iterations to average over")("verbose,v", "Print verbose output");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  benchmark_pi(threads, points, iters, vm.count("verbose"));
}