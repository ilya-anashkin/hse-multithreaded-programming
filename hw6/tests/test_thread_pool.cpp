#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

#include "future.h"

using namespace std::chrono_literals;

namespace {

TEST(ThreadPool, ComputesResults) {
  hw6::ThreadPool pool(4);
  std::vector<hw6::Future<int>> futures;
  futures.reserve(32);

  for (int i = 0; i < 32; ++i) {
    futures.push_back(pool.Submit([i] {
      std::this_thread::sleep_for(2ms);
      return i * i;
    }));
  }

  for (int i = 0; i < 32; ++i) {
    EXPECT_EQ(futures[i].Get(), i * i);
  }
}

TEST(ThreadPool, SupportsVoidTasks) {
  hw6::ThreadPool pool(3);
  std::atomic<int> counter = 0;

  auto first = pool.Submit([&counter] { counter.fetch_add(1); });
  auto second = pool.Submit([&counter] { counter.fetch_add(2); });

  first.Get();
  second.Get();

  EXPECT_EQ(counter.load(), 3);
}

TEST(ThreadPool, WaitAndReadinessWork) {
  hw6::ThreadPool pool(2);

  auto future = pool.Submit([] {
    std::this_thread::sleep_for(40ms);
    return 17;
  });

  EXPECT_FALSE(future.IsReady());
  future.Wait();
  EXPECT_TRUE(future.IsReady());
  EXPECT_EQ(future.Get(), 17);
}

TEST(ThreadPool, PropagatesExceptions) {
  hw6::ThreadPool pool(2);

  auto future = pool.Submit([]() -> int {
    throw std::runtime_error("boom");
  });

  EXPECT_THROW(future.Get(), std::runtime_error);
}

TEST(ThreadPool, NestedSubmitWorks) {
  hw6::ThreadPool pool(4);

  auto outer = pool.Submit([&pool] {
    auto inner = pool.Submit([] { return 21; });
    return inner.Get() * 2;
  });

  EXPECT_EQ(outer.Get(), 42);
}

TEST(ThreadPool, DestructorWaitsForQueuedTasks) {
  std::atomic<int> completed = 0;

  {
    hw6::ThreadPool pool(2);
    for (int i = 0; i < 8; ++i) {
      pool.Submit([&completed] {
        std::this_thread::sleep_for(5ms);
        completed.fetch_add(1);
      });
    }
  }

  EXPECT_EQ(completed.load(), 8);
}

TEST(Future, GetInvalidatesState) {
  hw6::ThreadPool pool(1);
  auto future = pool.Submit([] { return 5; });

  EXPECT_TRUE(future.Valid());
  EXPECT_EQ(future.Get(), 5);
  EXPECT_FALSE(future.Valid());
  EXPECT_THROW(future.Get(), std::runtime_error);
}

}  // namespace
