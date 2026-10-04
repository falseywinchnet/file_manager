#include "process.hpp"

#include <windows.h>

#include <chrono>
#include <cstdio>
#include <string>

namespace preview_process {

class Handle final {
public:
    explicit Handle(const HANDLE acquired) noexcept : value(acquired) {}
    ~Handle() { if (value != nullptr) { CloseHandle(value); } }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    const HANDLE value;
};

[[nodiscard]] static const wchar_t* wide_argument(const Fixture fixture) noexcept {
    switch (fixture) {
    case Fixture::success: return L"--child-success";
    case Fixture::failure: return L"--child-failure";
    case Fixture::delay: return L"--child-delay";
    case Fixture::memory: return L"--child-memory";
    case Fixture::memory_control: return L"--child-memory-control";
    }
    return L"--invalid";
}

Outcome run(const std::filesystem::path& executable, const Fixture fixture,
            const unsigned int deadline_ms) {
    Outcome outcome{};
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const std::wstring path = executable.native();
    std::wstring command = L"\"";
    command.append(path);
    command.append(L"\" ");
    command.append(wide_argument(fixture));
    const Handle job(CreateJobObjectW(nullptr, nullptr));
    if (job.value == nullptr) { outcome.failed = true; return outcome; }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
                                             JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
    limits.BasicLimitInformation.ActiveProcessLimit = 1;
    if (fixture == Fixture::memory) {
        limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
        limits.ProcessMemoryLimit = memory_limit_bytes;
    }
    const BOOL configured = SetInformationJobObject(job.value, JobObjectExtendedLimitInformation,
                                                    &limits, static_cast<DWORD>(sizeof(limits)));
    if (configured == FALSE) { outcome.failed = true; return outcome; }

    STARTUPINFOW startup{};
    startup.cb = static_cast<DWORD>(sizeof(startup));
    PROCESS_INFORMATION process{};
    // No inherited handles. The child cannot execute before assignment to limits.
    const BOOL created = CreateProcessW(path.c_str(), command.data(), nullptr, nullptr, FALSE,
                                       CREATE_SUSPENDED | CREATE_NO_WINDOW, nullptr, nullptr,
                                       &startup, &process);
    if (created == FALSE) { outcome.failed = true; return outcome; }
    outcome.launched = true;
    const Handle child(process.hProcess);
    const Handle primary_thread(process.hThread);
    const BOOL assigned = AssignProcessToJobObject(job.value, child.value);
    bool runnable = assigned != FALSE;
    if (runnable) {
        const DWORD previous_suspend_count = ResumeThread(primary_thread.value);
        runnable = previous_suspend_count == 1;
    }
    DWORD wait_result = WAIT_FAILED;
    if (runnable) {
        wait_result = WaitForSingleObject(child.value, deadline_ms);
        outcome.timed_out = wait_result == WAIT_TIMEOUT;
    }
    if (!runnable || wait_result != WAIT_OBJECT_0) {
        outcome.failed = !runnable || wait_result == WAIT_FAILED;
        // Direct termination covers failure to assign the still-suspended child.
        const BOOL terminated = TerminateProcess(child.value, 125);
        if (terminated == FALSE) {
            const DWORD already_exited = WaitForSingleObject(child.value, 0);
            if (already_exited != WAIT_OBJECT_0) { outcome.failed = true; }
        }
        wait_result = WaitForSingleObject(child.value, 5000);
    }
    outcome.reaped = wait_result == WAIT_OBJECT_0;
    if (!outcome.reaped) { outcome.failed = true; }
    DWORD code = 0;
    if (outcome.reaped) {
        const BOOL read_exit = GetExitCodeProcess(child.value, &code);
        if (read_exit == FALSE || code > 255) { outcome.failed = true; }
        else { outcome.exit_code = static_cast<int>(code); }
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    outcome.elapsed_milliseconds = elapsed.count();
    return outcome;
}

int probe_allocation(const bool apply_limit) {
    // On Windows the parent applies the job limit before resuming the child.
    (void)apply_limit;
    // Commit admission only: do not touch 384 MiB or claim resident-memory proof.
    void* const allocation = VirtualAlloc(nullptr, allocation_bytes,
                                         MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (allocation == nullptr) {
        const DWORD error = GetLastError();
        if (error == ERROR_NOT_ENOUGH_MEMORY || error == ERROR_COMMITMENT_LIMIT) {
            return allocation_refused;
        }
        return 1;
    }
    const BOOL released = VirtualFree(allocation, 0, MEM_RELEASE);
    if (released == FALSE) { return 1; }
    return allocation_admitted;
}

}
