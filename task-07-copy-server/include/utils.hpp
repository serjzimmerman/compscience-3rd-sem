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

#include <condition_variable>
#include <deque>
#include <mutex>

extern "C" {
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
}

namespace utils {

template <typename T> class shared_queue {
private:
  std::deque<T> m_underlying_queue;
  std::mutex    m_lock;

  std::condition_variable m_cv;

public:
  shared_queue() = default;

  auto dequeue() -> T {
    std::unique_lock<std::mutex> guard{m_lock};

    while (m_underlying_queue.empty()) {
      m_cv.wait(guard);
    }

    auto val = std::move(m_underlying_queue.back());
    m_underlying_queue.pop_back();
    return val;
  }

  auto enqueue(const T &val) -> void {
    std::unique_lock<std::mutex> guard{m_lock};
    m_underlying_queue.push_front(val);
    guard.release();
    m_cv.notify_one();
  }

  auto enqueue(T &&val) -> void {
    std::unique_lock<std::mutex> guard{m_lock};
    m_underlying_queue.push_front(val);
    guard.release();
    m_cv.notify_one();
  }
};

}; // namespace utils