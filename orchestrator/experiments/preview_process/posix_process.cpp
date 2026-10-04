#include "process.hpp"

#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <string>

namespace preview_process {

Outcome run(const std::filesystem::path& executable, const Fixture fixture,
            const unsigned int deadline_ms) {
    Outcome outcome{};
    const std::string path = executable.native();
    const char* const executable_bytes = path.c_str();
    const char* const mode = fixture_argument(fixture);
    // execv borrows these immutable bytes until exec/exit. Its legacy signature
    // is mutable; neither execv nor the child writes the buffers.
    char* const arguments[] = {const_cast<char*>(executable_bytes), const_cast<char*>(mode), nullptr};
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const std::chrono::steady_clock::time_point deadline = start + std::chrono::milliseconds(deadline_ms);
    const pid_t child = fork();
    if (child < 0) { outcome.failed = true; return outcome; }
    if (child == 0) {
        // No C++ allocation, cleanup or stdio in the post-fork/pre-exec child.
        execv(executable_bytes, arguments);
        _exit(126);
    }
    outcome.launched = true;
    int status = 0;
    while (!outcome.reaped) {
        const pid_t waited = waitpid(child, &status, WNOHANG);
        if (waited == child) { outcome.reaped = true; break; }
        if (waited < 0 && errno != EINTR) {
            // Do not signal a PID after ECHILD: it could already be reused.
            outcome.failed = true;
            return outcome;
        }
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (now >= deadline) { outcome.timed_out = true; break; }
        const struct timespec interval{0, 5000000};
        // EINTR only shortens this polling interval; recheck child and deadline.
        nanosleep(&interval, nullptr);
    }
    if (!outcome.reaped) {
        const int killed = kill(child, SIGKILL);
        if (killed != 0 && errno != ESRCH) { outcome.failed = true; }
        pid_t waited = -1;
        do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
        outcome.reaped = waited == child;
        if (!outcome.reaped) { outcome.failed = true; }
    }
    if (outcome.reaped && WIFEXITED(status)) { outcome.exit_code = WEXITSTATUS(status); }
    if (outcome.reaped && WIFSIGNALED(status)) {
        outcome.exit_code = 128 + WTERMSIG(status);
        if (!outcome.timed_out || WTERMSIG(status) != SIGKILL) { outcome.failed = true; }
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    outcome.elapsed_milliseconds = elapsed.count();
    return outcome;
}

int probe_allocation(const bool apply_limit) {
    // Apply after native loader startup, before any fixture allocation. This
    // deliberately does not claim a limit over exec/dynamic-loader startup.
    if (apply_limit) {
        struct rlimit limit{};
        limit.rlim_cur = static_cast<rlim_t>(memory_limit_bytes);
        limit.rlim_max = static_cast<rlim_t>(memory_limit_bytes);
        const int limited = setrlimit(RLIMIT_AS, &limit);
        if (limited != 0) {
            const int error = errno;
            std::printf("setrlimit_AS_refused_errno=%d\n", error);
            return limit_unavailable;
        }
    }
    void* const allocation = mmap(nullptr, allocation_bytes, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocation == MAP_FAILED) {
        if (errno == ENOMEM) { return allocation_refused; }
        return 1;
    }
    const int released = munmap(allocation, allocation_bytes);
    if (released != 0) { return 1; }
    return allocation_admitted;
}

}
