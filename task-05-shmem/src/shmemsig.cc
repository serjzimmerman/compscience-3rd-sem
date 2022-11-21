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

using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

struct shared_page {
private:
  std::string m_path;
  char       *m_ptr;

public:
  shared_page(uint64_t size, const std::string &name) : m_path{name} {
    auto fd = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd < 0) throw std::runtime_error{"Could not create shmem object"};
    if (ftruncate(fd, size) < 0) throw std::runtime_error{"Could not truncate shmem to requires size"};

    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) throw std::runtime_error{"Could not map shmem"};

    m_ptr = static_cast<char *>(ptr);
  }

  shared_page(const shared_page &other) = delete;
  shared_page &operator=(const shared_page &other) = delete;

  shared_page(shared_page &&other) = default;
  shared_page &operator=(shared_page &&other) = default;

  ~shared_page() { shm_unlink(m_path.c_str()); };

  auto path() const { return m_path; }

  auto ptr() const { return m_ptr; }
  auto ptr() { return m_ptr; }
};

struct shared_pages {
private:
  std::vector<shared_page> m_pages;

  uint64_t m_count;
  uint64_t m_size;

public:
  shared_pages(uint64_t count, uint64_t size, std::string temp = "task-05-shmem-") : m_count{count}, m_size{size} {
    for (uint64_t i = 0; i < count; ++i) {
      m_pages.emplace_back(size, temp + std::to_string(i));
    }
  }

  auto operator[](std::size_t index) const { return m_pages.at(index).ptr(); }
  auto operator[](std::size_t index) { return m_pages.at(index).ptr(); }
};

// Global states for child and parent processes
struct __attribute__((packed)) rw_index {
  uint32_t page;
  uint32_t offset;
};

rw_index child_index;
// This is so ugly I can't even look at it straight. These globals need to be volatile so the compiler does not optimize
// access to them.
volatile bool should_read_child = false;
volatile bool should_write_parent = true;

template <typename R> std::string random_string(std::size_t length, R &g) {
  static const char charset[] = "0123456789"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz";

  std::uniform_int_distribution<unsigned> dist(0, sizeof(charset) - 2);
  std::string                             result;

  for (std::size_t i = 0; i < length; ++i) {
    result.push_back(charset[dist(g)]);
  }

  return result;
}

void parent_handler(int signo, siginfo_t *info, void *context) {
  assert(signo == SIGUSR2);
  should_write_parent = true;
}

void child_handler(int signo, siginfo_t *info, void *context) {
  assert(signo == SIGUSR1);
  auto     data = info->si_value;
  rw_index cast = std::bit_cast<rw_index>(data);

  should_read_child = true;
  child_index = cast;
}

bool bind_signal_handlers() {
  struct sigaction act = {0};

  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = &child_handler;

  if (sigaction(SIGUSR1, &act, NULL) == -1) {
    perror("sigaction: ");
    return false;
  }

  act.sa_sigaction = &parent_handler;

  if (sigaction(SIGUSR2, &act, NULL) == -1) {
    perror("sigaction: ");
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  bool     verbose = true;
  uint32_t count, size;
  uint64_t iters;

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("verbose,v", "Verbose output")(
      "pages", po::value<uint32_t>(&count)->default_value(32), "Number of shared memory objects")(
      "size", po::value<uint32_t>(&size)->default_value(4096),
      "Size of a single page")("iters,n", po::value<uint64_t>(&iters)->default_value(32), "Number of iterations");
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return EXIT_FAILURE;
  }

  verbose = vm.count("verbose");

  if (!bind_signal_handlers()) {
    return EXIT_FAILURE;
  }

  shared_pages pages = {count, size};

  std::ostream *os = &std::cout;

  pid_t parent_pid = getpid();
  pid_t child_pid;

  auto child_process = [&]() {
    std::vector<char> str;
    str.resize(size);

    for (uint64_t i = 0; i < iters; ++i) {
      // Wait while we don't have to read. This is an ugly busy waiting loop, but I can't come up with anything better
      // to use for this task, as locks are not allowed.
      while (!should_read_child) {
        __asm__ __volatile__("");
      }

      std::strcpy(str.data(), pages[child_index.page] + child_index.offset);

      *os << "[Child process]: read finished" << std::endl; // Use endl to flush the buffer.
      if (verbose) *os << "[Child process]: string = " << str.data() << std::endl;

      should_read_child = false;
      kill(parent_pid, SIGUSR2);
    }
  };

  auto parent_process = [&]() {
    std::uniform_int_distribution<uint32_t> length_distribution(1, size - 1);
    std::uniform_int_distribution<uint32_t> page_distribution(0, count - 1);

    std::mt19937 g{std::random_device{}()};

    for (uint64_t i = 0; i < iters; ++i) {
      // Wait while we don't have to read. This is an ugly busy waiting loop, but I can't come up with anything better
      // to use for this task, as locks are not allowed.
      while (!should_write_parent) {
        __asm__ __volatile__("");
      }

      auto length = length_distribution(g), page = page_distribution(g);
      auto full_size = length + 1;
      auto str = random_string(length, g);

      std::uniform_int_distribution<uint32_t> offset_dist(0, size - full_size);
      auto                                    offset = offset_dist(g);

      std::strcpy(pages[page] + offset, str.c_str());

      rw_index idx = {page, offset};
      auto     cast = std::bit_cast<sigval>(idx);

      *os << "[Parent process]: write finished" << std::endl; // Use endl to flush the buffer.
      if (verbose) *os << "[Parent process]: string = " << str << std::endl;

      should_write_parent = false;
      sigqueue(child_pid, SIGUSR1, cast);
    }
  };

  if ((child_pid = fork()) == 0) {
    try {
      child_process();
    } catch (std::exception &e) {
      std::cout << e.what();
    }

    exit(EXIT_SUCCESS);
  } else if (child_pid < 0) {
    exit(EXIT_FAILURE);
  }

  parent_process();

  int exit_code = 0;
  waitpid(child_pid, &exit_code, 0);
  if (exit_code) return EXIT_FAILURE;
}