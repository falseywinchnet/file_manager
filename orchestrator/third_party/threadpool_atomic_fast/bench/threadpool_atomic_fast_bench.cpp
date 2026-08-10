#include "threadpool_atomic_fast.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

namespace {
using clock_type = std::chrono::steady_clock;

void transform(void* argument) {
    auto& value = *static_cast<std::uint64_t*>(argument);
    auto current = value;
    for (int round = 0; round < 32; ++round) {
        current += UINT64_C(0x9e3779b97f4a7c15);
        current = (current ^ (current >> 30U)) * UINT64_C(0xbf58476d1ce4e5b9);
        current = (current ^ (current >> 27U)) * UINT64_C(0x94d049bb133111eb);
        current ^= current >> 31U;
    }
    value = current;
}

std::uint64_t percentile(const std::vector<std::uint64_t>& sorted,
                         std::size_t numerator, std::size_t denominator) {
    return sorted[(sorted.size() - 1U) * numerator / denominator];
}

void report(const char* implementation, std::uint32_t threads,
            std::size_t batch, std::vector<std::uint64_t> durations) {
    std::sort(durations.begin(), durations.end());
    const auto total = std::accumulate(durations.begin(), durations.end(),
                                       UINT64_C(0));
    std::cout << "{\"implementation\":\"" << implementation
              << "\",\"threads\":" << threads << ",\"batch\":" << batch
              << ",\"samples\":" << durations.size()
              << ",\"mean_ns\":" << total / durations.size()
              << ",\"p50_ns\":" << percentile(durations, 50, 100)
              << ",\"p95_ns\":" << percentile(durations, 95, 100)
              << ",\"p99_ns\":" << percentile(durations, 99, 100)
              << ",\"max_ns\":" << durations.back() << "}\n";
}
}

int main(int argc, char** argv) {
    const auto threads = static_cast<std::uint32_t>(
        argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1);
    fileman::worker::atomic_thread_pool pool(threads);
    constexpr std::size_t batches[] = {1, 32, 256, 4'096, 65'536};
    std::uint64_t checksum = 0;
    for (const auto batch : batches) {
        const std::size_t samples = batch >= 65'536 ? 40 : 200;
        std::vector<std::uint64_t> values(batch, 1);
        std::vector<threadpool_task_t> tasks;
        tasks.reserve(batch);
        for (auto& value : values)
            tasks.push_back({transform, &value});
        for (int warmup = 0; warmup < 20; ++warmup)
            pool.run(tasks);
        std::vector<std::uint64_t> durations;
        durations.reserve(samples);
        for (std::size_t sample = 0; sample < samples; ++sample) {
            const auto started = clock_type::now();
            pool.run(tasks);
            durations.push_back(static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    clock_type::now() - started).count()));
        }
        checksum ^= values.front();
        report("c_atomic_batch", threads, batch, std::move(durations));

        durations.clear();
        durations.reserve(samples);
        for (std::size_t sample = 0; sample < samples; ++sample) {
            const auto started = clock_type::now();
            for (auto& value : values)
                transform(&value);
            durations.push_back(static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    clock_type::now() - started).count()));
        }
        checksum ^= values.back();
        report("cpp_serial", 1, batch, std::move(durations));
    }
    return checksum == UINT64_MAX ? 1 : 0;
}
