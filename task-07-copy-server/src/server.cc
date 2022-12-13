#include <array>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
#include <boost/system.hpp>
namespace po = boost::program_options;

#include "utils.hpp"

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

#include "command_parser/driver.hpp"
#include "req_parser/driver.hpp"

#include <algorithm>
#include <filesystem>
#include <optional>

namespace {

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

namespace asio = boost::asio;

class client_handler : public std::enable_shared_from_this<client_handler> {
private:
  asio::io_service &m_service;

  asio::posix::stream_descriptor m_client_input;
  asio::posix::stream_descriptor m_client_output;

  asio::streambuf m_input_buf;

  req_parser::driver m_drv;

public:
  client_handler(asio::io_service &service, int in, int out)
      : m_service{service}, m_client_input{service, in}, m_client_output{service, out} {}

  void read_request() {
    boost::asio::async_read_until(m_client_input, m_input_buf, '\n',
                                  [me = shared_from_this()](auto ec, auto sz) { me->read_request_done(ec, sz); });
  }

  void read_request_done(boost::system::error_code ec, std::size_t sz) {
    if (ec) return;

    std::istream is{&m_input_buf};
    m_drv.switch_input_stream(&is);
    bool result = m_drv.parse();

    // Flush everything that is left
    is.ignore(std::numeric_limits<std::streamsize>::max());

    // std::string_view resstr = result ? "[ack]\n" : "[err]\n";
    // auto             b = asio::buffer(resstr);

    // if (m_out_desc) {
    //   boost::asio::write(*m_out_desc, b);
    // }

    // read_command();
  }
};

class application_server {
  unsigned                 m_threads;
  std::vector<std::thread> m_thread_pool;
  asio::io_service         m_service;

  asio::posix::stream_descriptor                m_ctl_input;
  std::optional<asio::posix::stream_descriptor> m_ctl_output;

  struct ctl_handler : public std::enable_shared_from_this<ctl_handler> {
    asio::io_service &m_service;
    asio::streambuf   m_input_buf;

    asio::posix::stream_descriptor &m_in_desc;
    asio::posix::stream_descriptor *m_out_desc;

    command_parser::driver m_drv;

    ctl_handler(asio::io_service &service, asio::posix::stream_descriptor &in,
                asio::posix::stream_descriptor *out = nullptr)
        : m_service{service}, m_in_desc(in), m_out_desc(out) {}

    void read_command() {
      boost::asio::async_read_until(m_in_desc, m_input_buf, '\n',
                                    [me = shared_from_this()](auto ec, auto sz) { me->read_command_done(ec, sz); });
    }

    void read_command_done(boost::system::error_code ec, std::size_t sz) {
      if (ec) return;

      std::istream is{&m_input_buf};
      m_drv.switch_input_stream(&is);
      bool result = m_drv.parse();

      // Flush everything that is left
      is.ignore(std::numeric_limits<std::streamsize>::max());
      std::string_view resstr = "[err]\n";

      if (result) {
        resstr = "[err]\n";

        auto rx_fifo_fd = open(rx_fifo_name.c_str(), O_RDWR | O_NONBLOCK);
        if (rx_fifo_fd < 0) throw io_error{"Couldn't open the rx control named pipe"};
        auto tx_fifo_fd = open(tx_fifo_name.c_str(), O_RDWR | O_NONBLOCK);
      }

      auto b = asio::buffer(resstr);
      if (m_out_desc) {
        boost::asio::write(*m_out_desc, b);
      }

      read_command();
    }
  };

  using shared_ctl_handler = std::shared_ptr<ctl_handler>;

public:
  application_server(int in, std::optional<int> out, unsigned threads = 1)
      : m_threads{threads}, m_ctl_input{m_service, in} {
    if (!out) return;
    m_ctl_output = asio::posix::stream_descriptor{m_service, out.value()};
  }

  void start() {
    auto handler =
        std::make_shared<ctl_handler>(m_service, m_ctl_input, (m_ctl_output ? &m_ctl_output.value() : nullptr));
    handler->read_command();

    for (unsigned i = 0; i < m_threads; ++i) {
      m_thread_pool.push_back(std::thread([&]() { m_service.run(); }));
    }
  }

  void stop() {
    for (auto &v : m_thread_pool) {
      v.join();
    }
  }
};

class control_fifo_handler {};

} // namespace

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
  auto               tx_fifo_fd = open(tx_fifo_name.c_str(), O_RDWR | O_NONBLOCK);
  std::optional<int> tx_fifo_opt;

  if (tx_fifo_fd >= 0) {
    tx_fifo_opt = tx_fifo_fd;
  }

  application_server server{rx_fifo_fd, tx_fifo_opt, 5};
  server.start();
  server.stop();
}