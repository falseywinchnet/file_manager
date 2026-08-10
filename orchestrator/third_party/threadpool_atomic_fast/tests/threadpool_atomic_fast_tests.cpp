// Copyright (c) 2026 Joshuah Rainstar
// SPDX-License-Identifier: MIT

#include "threadpool_atomic_fast.hpp"

#include <atomic>
#include <cstdlib>
#include <cstdint>
#include <thread>
#include <vector>

namespace {
void require(bool condition) {
    if (!condition)
        std::abort();
}

void increment(void* argument) {
    auto* value = static_cast<std::atomic<std::uint64_t>*>(argument);
    value->fetch_add(1, std::memory_order_relaxed);
}

void increment_plain(void* argument) {
    auto* value = static_cast<std::uint32_t*>(argument);
    ++*value;
}
}

int main() {
    fileman::worker::atomic_thread_pool pool(4);
    require(pool.thread_count() == 4);
    std::atomic<std::uint64_t> completed{};
    std::vector<threadpool_task_t> tasks(10'000, {increment, &completed});
    pool.run(tasks);
    require(completed.load(std::memory_order_relaxed) == tasks.size());

    std::vector<std::thread> callers;
    for (int index = 0; index < 4; ++index) {
        callers.emplace_back([&pool, &completed] {
            std::vector<threadpool_task_t> batch(2'000, {increment, &completed});
            pool.run(batch);
        });
    }
    for (auto& caller : callers)
        caller.join();
    require(completed.load(std::memory_order_relaxed) == 18'000);

    for (int round = 0; round < 2'000; ++round) {
        std::vector<std::uint32_t> values(257);
        std::vector<threadpool_task_t> repeated;
        repeated.reserve(values.size());
        for (auto& value : values)
            repeated.push_back({increment_plain, &value});
        pool.run(repeated);
        for (const auto value : values)
            require(value == 1);
    }

    threadpool_task_t invalid{nullptr, nullptr};
    require(threadpool_run_checked(nullptr, &invalid, 1) ==
            THREADPOOL_INVALID_ARGUMENT);
    return 0;
}
