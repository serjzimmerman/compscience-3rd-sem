#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/asio/read_until.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
#include <boost/system.hpp>

#include <boost/asio.hpp>
namespace po = boost::program_options;

#include "utils.hpp"

extern "C" {
#include <fcntl.h>
#include <readline/readline.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include "driver.hpp"
#include <algorithm>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct fifo_error : public std::runtime_error {
  fifo_error(std::string s) : std::runtime_error{s} {}
};

struct io_error : public std::runtime_error {
  io_error(std::string s) : std::runtime_error{s} {}
};

void create_fifo(const std::string &path) {
  std::error_code code;
  if (fs::is_fifo(path, code)) return;

  int res = mkfifo(path.c_str(), 0666); // Create named pipe at path
  if (res < 0) {
    throw fifo_error{std::strerror(errno)};
  }
}

class c_file {
private:
  static constexpr auto file_deleter = [](std::FILE *ptr) { std::fclose(ptr); };

  std::unique_ptr<std::FILE, decltype(file_deleter)> m_underlying;

public:
  c_file(std::FILE *p) : m_underlying{p, file_deleter} {}

  std::FILE *get() { return m_underlying.get(); }

  std::string read_line() {
    char       *line_ptr = nullptr;
    std::size_t count;

    getline(&line_ptr, &count, m_underlying.get());

    static constexpr auto                    deleter = [](char *ptr) { std::free(ptr); };
    std::unique_ptr<char, decltype(deleter)> str = {line_ptr, deleter};

    if (!line_ptr) throw std::runtime_error{std::strerror(errno)};

    return std::string{str.get()};
  }
};

class client_handler {
private:
  boost::asio::io_service        &m_service;
  boost::asio::io_service::strand m_write_strand;
  boost::asio::streambuf          m_input_buf;

public:
  client_handler(boost::asio::io_service &service) : m_service{service}, m_write_strand{service} {}

  void read_command() {}
};

template <typename t_connection_handler> class application_server {
  unsigned                 m_threads;
  std::vector<std::thread> m_thread_pool;
  boost::asio::io_service  m_service;

  struct ctl_handler : public std::enable_shared_from_this<ctl_handler> {
    boost::asio::io_service               &m_service;
    boost::asio::io_service::strand        m_write_strand;
    boost::asio::streambuf                 m_input_buf;
    boost::asio::posix::stream_descriptor &m_stream_desc;

    ctl_handler(boost::asio::io_service &service) : m_service{service}, m_write_strand{service} {}

    auto shared_from_this() { return std::make_shared<ctl_handler>(this); }

    void read_command() {
      boost::asio::async_read_until(m_stream_desc, m_input_buf, '\n',
                                    [me = shared_from_this()](auto ec, auto sz) { me->read_command_done(ec, sz); });
    }

    void read_command_done(boost::system::error_code ec, std::size_t sz) {
      if (ec) return;

      std::istream is{&m_input_buf};

      command_parser::driver drv{};
      drv.switch_input_stream(&is);
      bool result = drv.parse();

      std::string res = (result ? "[ack]\n" : "[err]\n");

      if (result) {}

      read_command();
    }

    std::string execute_command() {}
  };

  using shared_ctl_handler = std::shared_ptr<ctl_handler>;

public:
  application_server(unsigned threads = 1) : m_threads{threads} {}

  void create_new_connection(shared_ctl_handler handler, const boost::system::error_code &error) {
    if (error) return;
  }
};

class control_fifo_handler {};

int main(int argc, char *argv[]) {
  auto        verbose = false;
  std::string control_fifo_path;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("verbose,v", "Verbose output")(
      "size", po::value<std::string>(&control_fifo_path)->default_value("/tmp/copy_ctl"), "Control FIFO path");
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_FAILURE;
  }

  verbose = vm.count("verbose");

  std::string rx_fifo_name = control_fifo_path + "_tx";
  std::string tx_fifo_name = control_fifo_path + "_rx";

  create_fifo(rx_fifo_name);
  create_fifo(tx_fifo_name);

  auto rx_fifo_fd = open(rx_fifo_name.c_str(), O_RDWR | O_NONBLOCK);
  if (rx_fifo_fd < 0) throw io_error{"Couldn't open the rx control named pipe"};
  auto tx_fifo_fd = open(tx_fifo_name.c_str(), O_RDWR | O_NONBLOCK);

  // application_info app = {{rx_fifo_fd, tx_fifo_opt}};

  boost::asio::io_service               service;
  boost::asio::posix::stream_descriptor rx_fifo_descriptor = {service, rx_fifo_fd};

  boost::asio::streambuf b;

  boost::asio::async_read_until(rx_fifo_descriptor, b, "\n", [&b](boost::system::error_code ec, std::size_t sz) {
    if (ec) {
      std::cout << "error\n";
      return;
    };
    std::istream is{&b};
    std::string  s;
    is >> s;
    std::cout << s;
  });

  service.run();
}