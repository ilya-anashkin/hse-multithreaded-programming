#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

template <class T>
class BufferedChannel {
 public:
  explicit BufferedChannel(int size)
      : capacity_(static_cast<std::size_t>(size)) {
    if (size < 0) {
      throw std::invalid_argument("Channel size must be non-negative");
    }
  }

  void Send(const T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_cv_.wait(
        lock, [this]() { return closed_ || queue_.size() < capacity_; });
    if (closed_) {
      throw std::runtime_error("Send on closed channel");
    }
    queue_.push_back(value);
    not_empty_cv_.notify_one();
  }

  std::optional<T> Recv() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_cv_.wait(lock, [this]() { return closed_ || !queue_.empty(); });
    if (queue_.empty()) {
      return std::nullopt;
    }

    T value = std::move(queue_.front());
    queue_.pop_front();
    not_full_cv_.notify_one();
    return value;
  }

  void Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_) {
      return;
    }
    closed_ = true;
    not_empty_cv_.notify_all();
    not_full_cv_.notify_all();
  }

 private:
  std::size_t capacity_;
  std::deque<T> queue_;
  bool closed_ = false;
  std::mutex mutex_;
  std::condition_variable not_empty_cv_;
  std::condition_variable not_full_cv_;
};
