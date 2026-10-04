#include "process.hpp"

#include <chrono>
#include <cstdio>
#include <exception>
#include <string_view>
#include <thread>

namespace preview_process {

const char* fixture_argument(const Fixture fixture) noexcept {
    switch (fixture) {
    case Fixture::success: return "--child-success";
    case Fixture::failure: return "--child-failure";
    case Fixture::delay: return "--child-delay";
    case Fixture::memory: return "--child-memory";
    case Fixture::memory_control: return "--child-memory-control";
    case Fixture::mac_headroom: return "--child-mac-headroom";
    }
    return "--invalid";
}

static void report(const char* const name, const Outcome& outcome) {
    std::printf("case=%s launched=%d reaped=%d timeout=%d failed=%d exit=%d elapsed_ms=%lld\n",
                name, static_cast<int>(outcome.launched), static_cast<int>(outcome.reaped),
                static_cast<int>(outcome.timed_out), static_cast<int>(outcome.failed),
                outcome.exit_code, outcome.elapsed_milliseconds);
}

[[nodiscard]] static bool completed(const Outcome& outcome, const int code) noexcept {
    const bool valid = outcome.launched && outcome.reaped && !outcome.timed_out &&
                       !outcome.failed && outcome.exit_code == code;
    return valid;
}

[[nodiscard]] static int lifecycle(const std::filesystem::path& executable) {
    const Outcome baseline = run(executable, Fixture::success, 3000);
    report("baseline", baseline);
    if (!completed(baseline, 0)) { return 1; }

    const Outcome failure = run(executable, Fixture::failure, 3000);
    report("nonzero-exit", failure);
    if (!completed(failure, 23)) { return 1; }

    // Stable fixed iteration count; every previous child is reaped before reuse.
    for (unsigned int iteration = 0; iteration < 4; ++iteration) {
        const Outcome cancelled = run(executable, Fixture::delay, 250);
        report("cancel-and-reap", cancelled);
        if (!cancelled.launched || !cancelled.reaped || !cancelled.timed_out ||
            cancelled.failed) { return 1; }
        const Outcome recovery = run(executable, Fixture::success, 3000);
        report("recovery", recovery);
        if (!completed(recovery, 0)) { return 1; }
    }
    return 0;
}

[[nodiscard]] static int memory(const std::filesystem::path& executable, const Fixture fixture) {
    const Outcome control = run(executable, Fixture::memory_control, 3000);
    report("unlimited-384MiB-control", control);
    if (!completed(control, allocation_admitted)) { return 1; }
    const Outcome outcome = run(executable, fixture, 3000);
    report(fixture_argument(fixture), outcome);
    if (completed(outcome, allocation_refused)) { return 0; }
#if defined(__APPLE__)
    if (completed(outcome, limit_unavailable) || completed(outcome, allocation_admitted)) {
        std::puts("LIMIT_UNAVAILABLE: this Mac does not establish the proposed address-space ceiling");
        return limit_unavailable;
    }
#endif
    return 1;
}

[[nodiscard]] static int execute(const int argument_count, char* const* const arguments) {
    if (argument_count != 2) { return 2; }
    const std::string_view mode(arguments[1]);
    if (mode == "--child-success") { return 0; }
    if (mode == "--child-failure") { return 23; }
    if (mode == "--child-delay") {
        // Bounded fixture even if its parent unexpectedly disappears.
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return 24;
    }
    if (mode == "--child-memory" || mode == "--child-memory-control") {
        const bool apply_limit = mode == "--child-memory";
        const int result = probe_allocation(apply_limit);
        return result;
    }
#if defined(__APPLE__)
    if (mode == "--child-mac-headroom") {
        const int result = probe_mac_headroom();
        return result;
    }
#endif
    const std::filesystem::path executable = std::filesystem::canonical(arguments[0]);
    if (mode == "--lifecycle") {
        const int result = lifecycle(executable);
        return result;
    }
    if (mode == "--memory-probe") {
        const int result = memory(executable, Fixture::memory);
        return result;
    }
#if defined(__APPLE__)
    if (mode == "--headroom-probe") {
        const int result = memory(executable, Fixture::mac_headroom);
        return result;
    }
#endif
    return 2;
}

}

int main(const int argument_count, char* const* const arguments) {
    try {
        const int result = preview_process::execute(argument_count, arguments);
        return result;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "experiment exception: %s\n", error.what());
        return 1;
    }
}
