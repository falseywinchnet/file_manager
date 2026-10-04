#pragma once

#include <cstddef>
#include <filesystem>

namespace preview_process {

inline constexpr std::size_t memory_limit_bytes = 256U * 1024U * 1024U;
inline constexpr std::size_t allocation_bytes = 384U * 1024U * 1024U;
inline constexpr int allocation_refused = 20;
inline constexpr int allocation_admitted = 21;
inline constexpr int limit_unavailable = 77;

enum class Fixture { success, failure, delay, memory, memory_control, mac_headroom };

struct Outcome {
    bool launched = false;
    bool reaped = false;
    bool timed_out = false;
    bool failed = false;
    int exit_code = -1;
    long long elapsed_milliseconds = 0;
};

// Synchronous experiment. Borrows executable through return; owns its one child
// until exit is observed. No arbitrary commands, payloads or descendants.
[[nodiscard]] Outcome run(const std::filesystem::path& executable,
                          const Fixture fixture, const unsigned int deadline_ms);
[[nodiscard]] int probe_allocation(const bool apply_limit);
#if defined(__APPLE__)
[[nodiscard]] int probe_mac_headroom();
#endif
[[nodiscard]] const char* fixture_argument(const Fixture fixture) noexcept;

}
