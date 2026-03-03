#include "futex_mutex.h"
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

TEST(FutexMutex, TryLockWorks) {
    futex_mutex m;
    ASSERT_TRUE(m.try_lock());
    ASSERT_FALSE(m.try_lock());
    m.unlock();
    ASSERT_TRUE(m.try_lock());
    m.unlock();
}

TEST(FutexMutex, MutualExclusionCounter) {
    futex_mutex m;
    int counter = 0;

    constexpr int kThreads = 4;
    constexpr int kIters = 200000;

    std::vector<std::thread> th;
    th.reserve(kThreads);

    for (int t = 0; t < kThreads; ++t) {
        th.emplace_back([&] {
            for (int i = 0; i < kIters; ++i) {
                m.lock();
                ++counter;
                m.unlock();
            }
        });
    }

    for (auto& x : th) x.join();
    ASSERT_EQ(counter, kThreads * kIters);
}
