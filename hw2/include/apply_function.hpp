#pragma once

#include <algorithm>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

template <typename T>
void ApplyFunction(std::vector<T>& data,
                   const std::function<void(T&)>& transform,
                   const int threadCount = 1) {
  if (data.empty()) {
    return;
  }

  const int normalizedThreadCount = std::max(1, threadCount);
  const std::size_t workerCount = std::min<std::size_t>(
      data.size(), static_cast<std::size_t>(normalizedThreadCount));

  if (workerCount == 1) {
    for (auto& value : data) {
      transform(value);
    }
    return;
  }

  std::vector<std::thread> workers;
  workers.reserve(workerCount);

  std::exception_ptr firstError;
  std::mutex errorMutex;

  const std::size_t chunkSize = data.size() / workerCount;
  const std::size_t remainder = data.size() % workerCount;

  std::size_t begin = 0;
  for (std::size_t i = 0; i < workerCount; ++i) {
    const std::size_t end = begin + chunkSize + (i < remainder ? 1 : 0);

    workers.emplace_back([&, begin, end] {
      try {
        for (std::size_t idx = begin; idx < end; ++idx) {
          transform(data[idx]);
        }
      } catch (...) {
        std::lock_guard<std::mutex> guard(errorMutex);
        if (!firstError) {
          firstError = std::current_exception();
        }
      }
    });

    begin = end;
  }

  for (auto& worker : workers) {
    worker.join();
  }

  if (firstError) {
    std::rethrow_exception(firstError);
  }
}
