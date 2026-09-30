#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <thread>
#include <vector>

namespace fileman::orchestrator::windows_local {

[[noreturn]] inline void fail(const char* message) {
    throw ClientError(std::string(message) + " (Windows error " + std::to_string(GetLastError()) + ")");
}

class Handle final {
public:
    explicit Handle(HANDLE value) : value_(value) {
        if (value == nullptr || value == INVALID_HANDLE_VALUE) fail("open local handle");
    }
    ~Handle() { CloseHandle(value_); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    HANDLE get() const noexcept { return value_; }
    HANDLE release() noexcept { HANDLE result = value_; value_ = nullptr; return result; }
private:
    HANDLE value_;
};

class Allocation final {
public:
    explicit Allocation(void* pointer) : pointer_(pointer) {}
    ~Allocation() { LocalFree(pointer_); }
    Allocation(const Allocation&) = delete;
    Allocation& operator=(const Allocation&) = delete;
private:
    void* pointer_;
};

inline std::wstring sid_text(PSID sid) {
    LPWSTR value = nullptr;
    if (!ConvertSidToStringSidW(sid, &value)) fail("read local SID");
    const Allocation allocation(value);
    return value;
}

inline std::wstring process_sid(HANDLE process) {
    HANDLE raw = nullptr;
    if (!OpenProcessToken(process, TOKEN_QUERY, &raw)) fail("query local process token");
    const Handle token(raw);
    DWORD size = 0;
    GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
    if (size == 0 || size > 65536) fail("invalid process token bound");
    std::vector<std::uintptr_t> bytes((size + sizeof(std::uintptr_t) - 1) / sizeof(std::uintptr_t));
    if (!GetTokenInformation(token.get(), TokenUser, bytes.data(), size, &size)) fail("read process token");
    return sid_text((*reinterpret_cast<const TOKEN_USER*>(bytes.data())).User.Sid);
}

inline void validate_acl(HANDLE handle) {
    PSID owner = nullptr;
    PACL acl = nullptr;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (GetSecurityInfo(handle, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                        &owner, nullptr, &acl, nullptr, &descriptor) != ERROR_SUCCESS) fail("read local object security");
    const Allocation allocation(descriptor);
    const std::wstring user = process_sid(GetCurrentProcess());
    if (owner == nullptr || acl == nullptr || sid_text(owner) != user || (*acl).AceCount == 0) fail("local object is not private");
    for (DWORD index = 0; index < (*acl).AceCount; ++index) {
        void* raw = nullptr;
        if (!GetAce(acl, index, &raw)) fail("inspect local DACL");
        const ACCESS_ALLOWED_ACE* ace = static_cast<const ACCESS_ALLOWED_ACE*>(raw);
        if ((*ace).Header.AceType != ACCESS_ALLOWED_ACE_TYPE ||
            sid_text(const_cast<DWORD*>(&(*ace).SidStart)) != user) fail("local DACL grants another identity");
    }
}

inline void validate_kind(HANDLE file, bool directory) {
    BY_HANDLE_FILE_INFORMATION information{};
    if (!GetFileInformationByHandle(file, &information) ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 ||
        ((information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) != directory) fail("invalid no-follow local object");
    validate_acl(file);
}

inline void validate_directory(const std::filesystem::path& path) {
    if (!path.is_absolute()) fail("runtime directory must be absolute");
    for (const std::filesystem::path& part : path) {
        if (part == L"..") fail("runtime directory contains parent traversal");
    }
    std::filesystem::path ancestor = path.parent_path();
    while (!ancestor.empty()) {
        const DWORD attributes = GetFileAttributesW(ancestor.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
            fail("runtime path has an unavailable or reparse ancestor");
        const std::filesystem::path parent = ancestor.parent_path();
        if (parent == ancestor) break;
        ancestor = parent;
    }
    const Handle directory(CreateFileW(path.c_str(), GENERIC_READ | READ_CONTROL,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    validate_kind(directory.get(), true);
}

inline std::string read_private(const std::filesystem::path& path, std::size_t limit) {
    const Handle file(CreateFileW(path.c_str(), GENERIC_READ | READ_CONTROL, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    validate_kind(file.get(), false);
    std::string bytes(limit + 1, '\0');
    DWORD count = 0;
    if (!ReadFile(file.get(), bytes.data(), static_cast<DWORD>(bytes.size()), &count, nullptr)) fail("read private record");
    if (count == 0 || count > limit) fail("private record outside byte bound");
    bytes.resize(count);
    return bytes;
}

inline std::wstring utf16(const std::string& value) {
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) fail("invalid UTF-8 pipe metadata");
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), count) != count) fail("decode pipe metadata");
    return result;
}

inline std::intptr_t connect(const std::string& endpoint, std::uint32_t expected_pid, const std::string& user_sid) {
    if (endpoint.rfind("\\\\.\\pipe\\", 0) != 0 || endpoint.find_first_of("\\/", 9) != std::string::npos ||
        utf16(user_sid) != process_sid(GetCurrentProcess())) fail("endpoint is not a same-user local pipe");
    const std::wstring name = utf16(endpoint);
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    HANDLE raw = INVALID_HANDLE_VALUE;
    while (true) {
        raw = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION, nullptr);
        if (raw != INVALID_HANDLE_VALUE) break;
        if (GetLastError() != ERROR_PIPE_BUSY || std::chrono::steady_clock::now() >= deadline) fail("connect local pipe");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    Handle pipe(raw);
    ULONG pid = 0;
    if (!GetNamedPipeServerProcessId(pipe.get(), &pid) || pid != expected_pid) fail("pipe server PID mismatch");
    const Handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
    if (process_sid(process.get()) != utf16(user_sid)) fail("pipe server SID mismatch");
    return reinterpret_cast<std::intptr_t>(pipe.release());
}

inline void close(std::intptr_t pipe) noexcept { CloseHandle(reinterpret_cast<HANDLE>(pipe)); }

inline void transfer(std::intptr_t pipe_value, void* buffer, std::size_t size, bool writing) {
    HANDLE pipe = reinterpret_cast<HANDLE>(pipe_value);
    const Handle event(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    std::size_t offset = 0;
    while (offset < size) {
        if (std::chrono::steady_clock::now() >= deadline) {
            SetLastError(WAIT_TIMEOUT);
            fail("local pipe deadline");
        }
        OVERLAPPED overlap{};
        overlap.hEvent = event.get();
        ResetEvent(event.get());
        DWORD amount = static_cast<DWORD>(size - offset);
        char* bytes = static_cast<char*>(buffer) + offset;
        const BOOL started = writing ? WriteFile(pipe, bytes, amount, nullptr, &overlap)
                                     : ReadFile(pipe, bytes, amount, nullptr, &overlap);
        if (!started && GetLastError() != ERROR_IO_PENDING) fail("local pipe I/O");
        const std::chrono::milliseconds remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
        const DWORD wait = remaining.count() > 0 ? static_cast<DWORD>(remaining.count()) : 0;
        DWORD transferred = 0;
        if (!GetOverlappedResultEx(pipe, &overlap, &transferred, wait, FALSE)) {
            const DWORD error = GetLastError();
            CancelIoEx(pipe, &overlap);
            GetOverlappedResult(pipe, &overlap, &transferred, TRUE);
            SetLastError(error);
            fail("local pipe deadline or I/O failure");
        }
        if (transferred == 0) fail("local pipe disconnected");
        offset += transferred;
    }
}
} // namespace fileman::orchestrator::windows_local
