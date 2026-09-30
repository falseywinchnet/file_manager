#include "file_manager/filesystem_model.hpp"
#include "file_manager/platform_paths.hpp"
#include "file_manager/platform_commands.hpp"
#include "file_manager/preview.hpp"
#include "file_manager/checksum.hpp"
#include "../src/native_file.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <tuple>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>

namespace {
void require(const bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename Value>
void put_reparse_field(std::span<std::byte> data, const std::size_t offset,
                       const Value value) {
    require(offset <= data.size() && sizeof(Value) <= data.size() - offset,
            "fixture field exceeds its buffer");
    std::memcpy(data.data() + offset, &value, sizeof(value));
}

std::vector<std::byte> reparse_fixture(const std::wstring& target, const DWORD flags) {
    require(target.size() <= (MAXIMUM_REPARSE_DATA_BUFFER_SIZE - 20U) / 2U,
            "fixture target exceeds the reparse buffer");
    const std::size_t target_bytes = target.size() * 2U;
    std::vector<std::byte> data(20U + target_bytes);
    put_reparse_field(data, 0, DWORD{IO_REPARSE_TAG_SYMLINK});
    put_reparse_field(data, 4, static_cast<WORD>(data.size() - 8));
    put_reparse_field(data, 10, static_cast<WORD>(target_bytes));
    put_reparse_field(data, 16, flags);
    std::memcpy(data.data() + 20, target.data(), target_bytes);
    return data;
}

void require_reparse_rejected(const std::span<const std::byte> data,
                              std::error_code& error) {
    const std::filesystem::path result = file_manager::parse_windows_symlink_target(data, error);
    require(result.empty() && error,
            "malformed or unsupported reparse data must fail closed");
}

struct SymlinkCase final {
    const wchar_t* input;
    const wchar_t* expected;
    DWORD flags;
};

void symlink_parser_tests() {
    std::error_code error{};
    for (const SymlinkCase& example : {
            SymlinkCase{L"..\\missing \u03a9\\file.txt", L"..\\missing \u03a9\\file.txt", DWORD{1}},
            SymlinkCase{L"\\??\\C:\\missing\\file.txt", L"\\\\?\\C:\\missing\\file.txt", DWORD{0}},
            SymlinkCase{L"\\??\\UNC\\server\\share\\file", L"\\\\?\\UNC\\server\\share\\file", DWORD{0}}}) {
        const std::vector<std::byte> data = reparse_fixture(example.input, example.flags);
        const std::filesystem::path parsed = file_manager::parse_windows_symlink_target(data, error);
        if (error || parsed != example.expected) {
            std::wcerr << L"reparse input=" << example.input << L" parsed=" << parsed.native()
                       << L" error=" << error.value() << L'\n';
        }
        require(parsed == example.expected && !error,
                "native reparse parser must preserve relative/absolute/UNC Unicode targets");
    }
    const std::vector<std::byte> valid = reparse_fixture(L"relative-target", 1);
    for (std::size_t length = 0; length < valid.size(); ++length) {
        require_reparse_rejected(std::span<const std::byte>(valid.data(), length), error);
    }
    std::vector<std::byte> bad = valid;
    put_reparse_field(bad, 0, DWORD{IO_REPARSE_TAG_MOUNT_POINT});
    require_reparse_rejected(bad, error);
    require(error == std::errc::operation_not_supported, "junction must not become a symlink copy");
    bad = valid;
    put_reparse_field(bad, 8, WORD{1});
    require_reparse_rejected(bad, error);
    bad = valid;
    put_reparse_field(bad, 10, WORD{3});
    require_reparse_rejected(bad, error);
    bad = valid;
    put_reparse_field(bad, 8, WORD{0xfffe});
    require_reparse_rejected(bad, error);
    bad = valid;
    put_reparse_field(bad, 12, WORD{0xfffe});
    require_reparse_rejected(bad, error);
    bad = valid;
    put_reparse_field(bad, 16, DWORD{2});
    require_reparse_rejected(bad, error);
    bad = valid;
    put_reparse_field(bad, 20, WORD{0});
    require_reparse_rejected(bad, error);
    require_reparse_rejected(reparse_fixture(L"C:\\absolute", 1), error);
    require_reparse_rejected(reparse_fixture(L"relative", 0), error);
    require_reparse_rejected(reparse_fixture(L"", 1), error);
    // Substitute name is authoritative; a different printable name is ignored.
    std::vector<std::byte> offset = reparse_fixture(L"displayactual", 1);
    put_reparse_field(offset, 8, WORD{14});
    put_reparse_field(offset, 10, WORD{12});
    put_reparse_field(offset, 12, WORD{0});
    put_reparse_field(offset, 14, WORD{14});
    const std::filesystem::path parsed_offset = file_manager::parse_windows_symlink_target(offset, error);
    require(parsed_offset == L"actual" && !error,
            "parser must honor substitute offset instead of print name");
}

struct Fixture final {
    std::filesystem::path root;
    Fixture() : root(std::filesystem::temp_directory_path() /
        ("file-manager-windows-" + std::to_string(GetCurrentProcessId()))) {
        std::filesystem::create_directory(root);
    }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;

    ~Fixture() {
        std::error_code ignored{};
        std::filesystem::remove_all(root, ignored);
    }
};

void create_junction(const std::filesystem::path& target,
                     const std::filesystem::path& link) {
    struct MountPointData final {
        DWORD tag{IO_REPARSE_TAG_MOUNT_POINT};
        WORD bytes{};
        WORD reserved{};
        WORD substitute_offset{};
        WORD substitute_bytes{};
        WORD print_offset{};
        WORD print_bytes{};
        wchar_t paths[1024]{};
    } data{};
    const std::wstring substitute = L"\\??\\" + target.wstring();
    const std::wstring display = target.wstring();
    require(substitute.size() + display.size() + 2U < 1024U, "junction fixture path too long");
    std::copy(substitute.begin(), substitute.end(), data.paths);
    std::copy(display.begin(), display.end(), data.paths + substitute.size() + 1U);
    data.substitute_bytes = static_cast<WORD>(substitute.size() * sizeof(wchar_t));
    data.print_offset = static_cast<WORD>((substitute.size() + 1U) * sizeof(wchar_t));
    data.print_bytes = static_cast<WORD>(display.size() * sizeof(wchar_t));
    data.bytes = static_cast<WORD>(8U + data.print_offset + data.print_bytes + sizeof(wchar_t));
    std::filesystem::create_directory(link);
    const HANDLE handle = CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    require(handle != INVALID_HANDLE_VALUE, "cannot open junction fixture");
    DWORD returned{};
    const BOOL created = DeviceIoControl(handle, FSCTL_SET_REPARSE_POINT, &data,
        data.bytes + 8U, nullptr, 0, &returned, nullptr);
    CloseHandle(handle);
    require(created != 0, "cannot create junction fixture");
}
}

int main() {
    try {
        symlink_parser_tests();
        Fixture fixture{};
        const std::filesystem::path file = fixture.root / u8"notes Ω 日本語.txt";
        std::ofstream(file, std::ios::binary) << "abc";
        const file_manager::ObjectIdentity before = file_manager::observe_identity(file);
        require(before.available() && before.size == 3, "native file identity unavailable");
        const std::string encoded = file_manager::path_utf8(file);
        const std::filesystem::path decoded = file_manager::path_from_utf8(encoded);
        require(decoded == file,
                "Unicode path round trip failed");
        const file_manager::DirectorySnapshot listing =
            file_manager::read_directory(fixture.root, fixture.root, {}, 1);
        require(listing.available() && listing.entries.size() == 1 &&
                listing.entries[0].name == file_manager::path_utf8(file.filename()),
                "Unicode directory listing failed");
        const file_manager::PreviewResult preview =
            file_manager::load_preview(fixture.root, file, before);
        require(preview.kind == file_manager::PreviewKind::text && preview.text_utf8 == "abc",
                "Unicode preview failed");
        const file_manager::ChecksumResult checksum =
            file_manager::checksum_sha256(fixture.root, file, before);
        require(checksum.succeeded() && checksum.digest_hex ==
                "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                "Unicode checksum failed");

        const std::filesystem::path renamed = fixture.root / u8"renamed Ω.txt";
        std::filesystem::rename(file, renamed);
        const file_manager::ObjectIdentity renamed_identity = file_manager::observe_identity(renamed);
        require(before == renamed_identity,
                "rename changed stable Windows identity");
        const std::filesystem::path hardlink = fixture.root / L"hardlink.txt";
        std::filesystem::create_hard_link(renamed, hardlink);
        const file_manager::ObjectIdentity linked_identity = file_manager::observe_identity(hardlink);
        require(before == linked_identity,
                "hard link must identify the same object");
        file_manager::ObjectIdentity upper = before;
        upper.inode_high ^= 1ULL;
        require(!(before == upper), "upper Windows identity bits were ignored");

        const std::filesystem::path target = fixture.root / L"target";
        const std::filesystem::path junction = fixture.root / L"junction";
        std::filesystem::create_directory(target);
        std::ofstream(target / L"inside.txt") << "inside";
        create_junction(target, junction);
        std::error_code link_error{};
        static_cast<void>(file_manager::read_native_symlink(junction, link_error));
        require(link_error == std::errc::operation_not_supported,
                "native reader must explicitly refuse a live junction");
        const file_manager::ObjectIdentity junction_identity = file_manager::observe_identity(junction);
        const bool junction_route = file_manager::path_route_has_symlink(fixture.root, junction / L"inside.txt");
        require(junction_identity.type == std::filesystem::file_type::symlink && junction_route,
                "Windows junction must remain a no-follow reparse object");
        const file_manager::DirectorySnapshot junction_listing = file_manager::read_directory(fixture.root, junction, {}, 2);
        require(!junction_listing.available(),
                "navigation must refuse a junction route");
        std::filesystem::remove(junction);
        std::filesystem::remove_all(target);

        const BOOL hidden_set = SetFileAttributesW(renamed.c_str(), FILE_ATTRIBUTE_HIDDEN);
        require(hidden_set != 0,
                "cannot set fixture hidden attribute");
        const file_manager::DirectorySnapshot hidden =
            file_manager::read_directory(fixture.root, fixture.root, {}, 2);
        require(hidden.available() && hidden.entries.empty(),
                "Windows hidden attribute must hide every hard-link view");
        const BOOL hidden_cleared = SetFileAttributesW(renamed.c_str(), FILE_ATTRIBUTE_NORMAL);
        require(hidden_cleared != 0,
                "cannot clear fixture hidden attribute");

        file_manager::PlatformCommandPlan plan{};
        const file_manager::PlatformCommandResult planned =
            file_manager::make_platform_command_plan(
                file_manager::PlatformCommandKind::open_default,
                fixture.root, renamed, before, plan);
        require(planned.code == "ok" && plan.selected_path == renamed,
                "Unicode launch plan failed");
        plan.arguments.push_back("unadmitted");
        const file_manager::PlatformCommandResult refused = file_manager::execute_platform_command(plan);
        require(refused.code == "invalid-plan",
                "tampered launch plan was not refused");
        require(!file_manager::valid_platform_basename("..\\escape") &&
                !file_manager::valid_platform_basename("file:stream") &&
                !file_manager::valid_platform_basename("CON.txt") &&
                !file_manager::valid_platform_basename("trailing.") &&
                file_manager::valid_platform_basename("ordinary.txt"),
                "Windows basename validation failed");
        std::cout << "Windows identity, Unicode, preview, checksum and launch-plan tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
