#pragma once

#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace hw6 {

template <class T>
class FutureState {
 public:
  void SetValue(T value) {
    std::lock_guard lock(mutex_);
    EnsurePending();
    value_.emplace(std::move(value));
    ready_ = true;
    cv_.notify_all();
  }

  T Get() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return ready_; });
    if (retrieved_) {
      throw std::runtime_error("Future value already retrieved");
    }
    retrieved_ = true;
    if (exception_) {
      std::rethrow_exception(exception_);
    }
    return std::move(*value_);
  }

  void Wait() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return ready_; });
  }

  bool IsReady() const {
    std::lock_guard lock(mutex_);
    return ready_;
  }

  void SetException(std::exception_ptr exception) {
    std::lock_guard lock(mutex_);
    EnsurePending();
    exception_ = exception;
    ready_ = true;
    cv_.notify_all();
  }

 private:
  void EnsurePending() const {
    if (ready_) {
      throw std::logic_error("Future state already satisfied");
    }
  }

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool ready_ = false;
  bool retrieved_ = false;
  std::optional<T> value_;
  std::exception_ptr exception_;
};

template <>
class FutureState<void> {
 public:
  void SetValue() {
    std::lock_guard lock(mutex_);
    EnsurePending();
    ready_ = true;
    cv_.notify_all();
  }

  void Get() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return ready_; });
    if (retrieved_) {
      throw std::runtime_error("Future value already retrieved");
    }
    retrieved_ = true;
    if (exception_) {
      std::rethrow_exception(exception_);
    }
  }

  void Wait() {
    std::unique_lock lock(mutex_);
    cv_.wait(lock, [this] { return ready_; });
  }

  bool IsReady() const {
    std::lock_guard lock(mutex_);
    return ready_;
  }

  void SetException(std::exception_ptr exception) {
    std::lock_guard lock(mutex_);
    EnsurePending();
    exception_ = exception;
    ready_ = true;
    cv_.notify_all();
  }

 private:
  void EnsurePending() const {
    if (ready_) {
      throw std::logic_error("Future state already satisfied");
    }
  }

  mutable std::mutex mutex_;
  std::condition_variable cv_;
  bool ready_ = false;
  bool retrieved_ = false;
  std::exception_ptr exception_;
};

template <class T>
class Future {
 public:
  Future() = default;

  explicit Future(std::shared_ptr<FutureState<T>> state)
      : state_(std::move(state)) {}

  bool Valid() const noexcept { return static_cast<bool>(state_); }

  void Wait() const {
    EnsureValid();
    state_->Wait();
  }

  bool IsReady() const {
    EnsureValid();
    return state_->IsReady();
  }

  T Get() {
    EnsureValid();
    auto state = std::move(state_);
    state_.reset();
    return state->Get();
  }

 private:
  void EnsureValid() const {
    if (!state_) {
      throw std::runtime_error("Future has no state");
    }
  }

  std::shared_ptr<FutureState<T>> state_;
};

template <>
class Future<void> {
 public:
  Future() = default;

  explicit Future(std::shared_ptr<FutureState<void>> state)
      : state_(std::move(state)) {}

  bool Valid() const noexcept { return static_cast<bool>(state_); }

  void Wait() const {
    EnsureValid();
    state_->Wait();
  }

  bool IsReady() const {
    EnsureValid();
    return state_->IsReady();
  }

  void Get() {
    EnsureValid();
    auto state = std::move(state_);
    state_.reset();
    state->Get();
  }

 private:
  void EnsureValid() const {
    if (!state_) {
      throw std::runtime_error("Future has no state");
    }
  }

  std::shared_ptr<FutureState<void>> state_;
};

class ThreadPool {
 public:
  explicit ThreadPool(std::size_t workers_count) : stopping_(false) {
    if (workers_count == 0) {
      throw std::invalid_argument("ThreadPool size must be positive");
    }
    workers_.reserve(workers_count);
    for (std::size_t i = 0; i < workers_count; ++i) {
      workers_.emplace_back([this] { WorkerLoop(); });
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  ~ThreadPool() {
    {
      std::lock_guard lock(mutex_);
      stopping_ = true;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  template <class F, class... Args>
  auto Submit(F&& func, Args&&... args) {
    using Result = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

    auto state = std::make_shared<FutureState<Result>>();
    auto task = MakeTask<Result>(state, std::forward<F>(func),
                                 std::forward<Args>(args)...);

    {
      std::lock_guard lock(mutex_);
      if (stopping_) {
        throw std::runtime_error("ThreadPool is stopping");
      }
      tasks_.push(std::move(task));
    }
    cv_.notify_one();
    return Future<Result>(std::move(state));
  }

 private:
  template <class Result, class F, class... Args>
  static std::function<void()> MakeTask(
      std::shared_ptr<FutureState<Result>> state, F&& func, Args&&... args) {
    auto bound = [fn = std::forward<F>(func),
                  tpl = std::make_tuple(std::forward<Args>(args)...),
                  state = std::move(state)]() mutable {
      try {
        if constexpr (std::is_void_v<Result>) {
          std::apply(std::move(fn), std::move(tpl));
          state->SetValue();
        } else {
          state->SetValue(std::apply(std::move(fn), std::move(tpl)));
        }
      } catch (...) {
        state->SetException(std::current_exception());
      }
    };
    return std::function<void()>(std::move(bound));
  }

  void WorkerLoop() {
    for (;;) {
      std::function<void()> task;
      {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });
        if (stopping_ && tasks_.empty()) {
          return;
        }
        task = std::move(tasks_.front());
        tasks_.pop();
      }
      task();
    }
  }

  std::mutex mutex_;
  std::condition_variable cv_;
  bool stopping_;
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
};

}  // namespace hw6
