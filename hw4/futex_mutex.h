#pragma once

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

class futex_mutex {
public:
    futex_mutex() noexcept : state_(0) {}
    futex_mutex(const futex_mutex&) = delete;
    futex_mutex& operator=(const futex_mutex&) = delete;

    void lock() {
		// State 0 -> State 1
        int expected = 0;
        if (state_.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            return;
        }

        for (;;) {
            int s = state_.load(std::memory_order_relaxed);
            if (s != 0) {
                if (s != 2) {
					// State 1 -> State 2
                    expected = 1;
                    (void)state_.compare_exchange_strong(expected, 2, std::memory_order_relaxed);
                }
                futex_wait(2);
                continue;
            }

            // State 0 -> State 2
            expected = 0;
            if (state_.compare_exchange_strong(expected, 2, std::memory_order_acquire)) {
                return;
            }
        }
    }

    bool try_lock() {
		// State 0 -> State 1
        int expected = 0;
        return state_.compare_exchange_strong(expected, 1, std::memory_order_acquire);
    }

    void unlock() {
        // If it was state 1, just drop to state 0
        int prev = state_.exchange(0, std::memory_order_release);
        if (prev == 2) { // If it was state 2, just wake one
            futex_wake(1);
        }
    }

private:
    // States: 0 unlocked, 1 locked no waiters, 2 locked contended
    std::atomic<int> state_;

    static int futex_syscall(int* uaddr, int op, int val) {
        return static_cast<int>(syscall(SYS_futex, uaddr, op, val, nullptr, nullptr, 0));
    }

    void futex_wait(int expected) {
        int* addr = reinterpret_cast<int*>(&state_);
        for (;;) {
            int rc = futex_syscall(addr, FUTEX_WAIT, expected);
            if (rc == 0) return;
            if (errno == EAGAIN) return;
            if (errno == EINTR) continue;
            return;
        }
    }

    void futex_wake(int count) {
        int* addr = reinterpret_cast<int*>(&state_);
        (void)futex_syscall(addr, FUTEX_WAKE, count);
    }
};
