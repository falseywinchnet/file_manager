#include "file_manager/platform_paths.hpp"
#include "file_manager/filesystem_model.hpp"
#include "native_file.hpp"
#include "directory_name_order.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <span>
#include <stdexcept>
#include <utility>
#include <sys/stat.h>

namespace file_manager {
namespace {

std::string ascii_lower(std::string value) {
    for (char& character : value) {
        const unsigned char byte = static_cast<unsigned char>(character);
        const int lowered = std::tolower(byte);
        character = static_cast<char>(lowered);
    }
    return value;
}

std::string stable_id_for(const std::filesystem::path& path,
                          const ObjectIdentity& identity) {
    if (identity.available()) {
        std::ostringstream stream{};
        stream << "fm.object." << std::hex << std::setw(16) << std::setfill('0')
               << identity.device << '.' << std::setw(16) << identity.inode;
        if (identity.inode_high != 0) stream << '.' << std::setw(16) << identity.inode_high;
        const std::string result = stream.str();
        return result;
    }
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offset;
    const std::string bytes = path_generic_utf8(path);
    for (const unsigned char value : bytes) {
        hash ^= value;
        hash *= prime;
    }
    std::ostringstream stream{};
    stream << "fm.object." << std::hex << std::setw(16) << std::setfill('0')
           << hash;
    const std::string result = stream.str();
    return result;
}

bool has_symlink_component(const std::filesystem::path& root,
                           const std::filesystem::path& candidate) {
    std::filesystem::path cursor = root;
    const std::filesystem::path relative = candidate.lexically_relative(root);
    for (const std::filesystem::path& component : relative) {
        if (component == ".") continue;
        cursor /= component;
        std::error_code error{};
        const std::filesystem::file_status status = std::filesystem::symlink_status(cursor, error);
        if (error) return false;
        if (std::filesystem::is_symlink(status)) return true;
#if defined(_WIN32)
        const DWORD attributes = GetFileAttributesW(cursor.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) return true;
#endif
    }
    return false;
}

bool contains_extension(const std::span<const std::string_view> values,
                        const std::string_view extension) {
    const bool found = std::find(values.begin(), values.end(), extension) != values.end();
    return found;
}

EntryKind kind_for(const std::filesystem::path& path,
                   const std::filesystem::file_type type) {
    if (type == std::filesystem::file_type::symlink) return EntryKind::symlink;
    if (type == std::filesystem::file_type::directory) return EntryKind::folder;
    if (type != std::filesystem::file_type::regular) return EntryKind::other;
    const std::filesystem::path suffix = path.extension();
    const std::string suffix_text = path_utf8(suffix);
    const std::string extension = ascii_lower(suffix_text);
    static constexpr std::array<std::string_view, 7> image_extensions{
        ".png", ".jpg", ".jpeg", ".gif", ".webp", ".tiff", ".heic"};
    static constexpr std::array<std::string_view, 6> archive_extensions{
        ".zip", ".tar", ".gz", ".bz2", ".xz", ".7z"};
    static constexpr std::array<std::string_view, 6> audio_extensions{
        ".wav", ".mp3", ".m4a", ".aac", ".flac", ".ogg"};
    static constexpr std::array<std::string_view, 15> code_extensions{
        ".c", ".cc", ".cpp", ".h", ".hpp", ".go", ".rs", ".py",
        ".js", ".ts", ".html", ".css", ".json", ".toml", ".yaml"};
    if (contains_extension(image_extensions, extension)) return EntryKind::image;
    if (contains_extension(archive_extensions, extension)) return EntryKind::archive;
    if (contains_extension(audio_extensions, extension)) return EntryKind::audio;
    if (contains_extension(code_extensions, extension)) return EntryKind::code;
    return EntryKind::document;
}

} // namespace

std::filesystem::path canonical_existing_directory(
    const std::filesystem::path& path) {
    std::error_code error{};
    const std::filesystem::path canonical = std::filesystem::canonical(path, error);
    if (error) {
        throw std::runtime_error("cannot resolve directory: " + error.message());
    }
    const std::filesystem::file_status status = std::filesystem::status(canonical, error);
    if (error || !std::filesystem::is_directory(status)) {
        throw std::runtime_error("path is not an available directory");
    }
    return canonical;
}

bool path_is_within(const std::filesystem::path& root,
                    const std::filesystem::path& candidate) {
    std::filesystem::path::const_iterator root_it = root.begin();
    std::filesystem::path::const_iterator candidate_it = candidate.begin();
    for (; root_it != root.end(); ++root_it, ++candidate_it) {
        if (candidate_it == candidate.end() || *root_it != *candidate_it) {
            return false;
        }
    }
    return true;
}

std::optional<std::filesystem::path> rebase_path_from_equivalent_root(
    const std::filesystem::path& canonical_root,
    const std::filesystem::path& candidate) {
    const std::filesystem::path lexical = candidate.lexically_normal();
    if (!canonical_root.is_absolute() || !lexical.is_absolute()) return {};
    if (path_is_within(canonical_root, lexical)) return lexical;

    std::vector<std::filesystem::path> suffix{};
    std::filesystem::path cursor = lexical;
    for (;;) {
        std::error_code error{};
        if (std::filesystem::equivalent(canonical_root, cursor, error) &&
            !error) {
            std::filesystem::path rebased = canonical_root;
            for (std::vector<std::filesystem::path>::reverse_iterator component = suffix.rbegin(); component != suffix.rend();
                 ++component) {
                rebased /= *component;
            }
            rebased = rebased.lexically_normal();
            return rebased;
        }
        const std::filesystem::path parent = cursor.parent_path();
        if (parent.empty() || parent == cursor) break;
        suffix.push_back(cursor.filename());
        cursor = parent;
    }
    return {};
}

bool path_route_has_symlink(const std::filesystem::path& canonical_root,
                            const std::filesystem::path& candidate) {
    const std::filesystem::path lexical = candidate.lexically_normal();
    const bool found = path_is_within(canonical_root, lexical) &&
        has_symlink_component(canonical_root, lexical);
    return found;
}

std::optional<NavigationTarget> resolve_navigation_target(
    const std::vector<std::filesystem::path>& admitted_roots,
    const std::filesystem::path& current_location,
    const std::filesystem::path& home_root,
    const std::filesystem::path& requested) {
    if (requested.empty() || admitted_roots.empty()) return {};

    std::filesystem::path expanded = requested;
    const std::string text = path_utf8(requested);
    if (text == "~") {
        expanded = home_root;
    } else if (text.starts_with("~/")) {
        expanded = home_root / text.substr(2);
    } else if (!expanded.is_absolute()) {
        expanded = current_location / expanded;
    }
    expanded = expanded.lexically_normal();

    std::optional<NavigationTarget> best{};
    std::size_t best_depth{};
    for (const std::filesystem::path& supplied_root : admitted_roots) {
        const std::filesystem::path root = supplied_root.lexically_normal();
        std::filesystem::path candidate = expanded;
        if (!path_is_within(root, candidate)) {
            const std::optional<std::filesystem::path> rebased = rebase_path_from_equivalent_root(root, candidate);
            if (!rebased) continue;
            candidate = *rebased;
        }
        const std::size_t depth = static_cast<std::size_t>(
            std::distance(root.begin(), root.end()));
        if (!best || depth > best_depth) {
            best = NavigationTarget{root, std::move(candidate)};
            best_depth = depth;
        }
    }
    return best;
}

ObjectIdentity observe_identity(const std::filesystem::path& path) {
    const NativeObjectObservation observed = observe_native_object(path);
    return observed.identity;
}
bool object_is_hidden(const std::filesystem::path& path,
                      const std::string_view name) {
    if (!name.empty() && name.front() == '.') return true;
#if defined(_WIN32)
    const DWORD attributes = GetFileAttributesW(path.c_str());
    const bool hidden = attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    return hidden;
#elif defined(__APPLE__)
    struct stat observed {};
    const int status = ::lstat(path.c_str(), &observed);
    const bool hidden = status == 0 && (observed.st_flags & UF_HIDDEN) != 0;
    return hidden;
#else
    static_cast<void>(path);
    return false;
#endif
}

DirectorySnapshot read_directory(const std::filesystem::path& root,
                                 const std::filesystem::path& requested,
                                 const std::string_view filter,
                                 const std::uint64_t generation,
                                 const CancellationCheck& cancelled,
                                 const bool show_hidden) {
    DirectorySnapshot result{};
    result.root = root;
    result.location = requested;
    result.generation = generation;
    try {
        result.root = canonical_existing_directory(root);
        const std::filesystem::path lexical_request = (requested.is_absolute()
            ? requested
            : result.root / requested).lexically_normal();
        if (!path_is_within(result.root, lexical_request)) {
            result.error = "requested location is outside the protected root";
            return result;
        }
        if (has_symlink_component(result.root, lexical_request)) {
            result.error = "requested location traverses a symbolic link";
            return result;
        }
        result.location = canonical_existing_directory(lexical_request);
        if (!path_is_within(result.root, result.location)) {
            result.error = "requested location is outside the protected root";
            return result;
        }
        if (cancelled && cancelled()) {
            result.cancelled = true;
            return result;
        }

        const std::string needle = ascii_lower(std::string(filter));
        std::error_code error{};
        std::filesystem::directory_iterator iterator(
            result.location,
            std::filesystem::directory_options::skip_permission_denied,
            error);
        if (error) {
            result.error = "cannot enumerate location: " + error.message();
            return result;
        }
        constexpr std::size_t maximum_entries = 50'000;
        for (const std::filesystem::directory_entry& entry : iterator) {
            if (cancelled && cancelled()) {
                result.entries.clear();
                result.cancelled = true;
                return result;
            }
            if (result.entries.size() >= maximum_entries) {
                result.error = "location exceeds the 50,000-item frontend bound";
                break;
            }
            const std::string name = path_utf8(entry.path().filename());
            if (!show_hidden && object_is_hidden(entry.path(), name)) continue;
            if (!needle.empty() && ascii_lower(name).find(needle) == std::string::npos) {
                continue;
            }
            DirectoryEntry item{};
            item.path = entry.path();
            const NativeObjectObservation observed = observe_native_object(item.path);
            if (!observed.error && observed.identity.available()) {
                item.identity = observed.identity;
                item.metadata = observed.facts;
            }
            item.stable_id = stable_id_for(item.path, item.identity);
            item.name = name;
            item.kind = kind_for(item.path, item.identity.type);
            item.directory = item.kind == EntryKind::folder;
            if (!item.identity.available()) {
                item.secondary_text = "Unavailable";
            } else if (item.directory) {
                item.secondary_text = "Folder";
            } else if (item.identity.type == std::filesystem::file_type::regular) {
                item.secondary_text = item.metadata.logical_size ?
                    format_bytes(*item.metadata.logical_size) : "Unavailable";
            } else if (item.kind == EntryKind::symlink) {
                item.secondary_text = "Symbolic link · not followed";
            } else {
                item.secondary_text = "Filesystem object";
            }
            item.modified_text = item.metadata.modified ?
                format_modified_time(*item.metadata.modified) : "Unavailable";
            result.entries.push_back(std::move(item));
        }
        std::sort(result.entries.begin(), result.entries.end(), detail::DirectoryNameOrder{});
    } catch (const std::exception& error) {
        result.error = error.what();
    }
    return result;
}

std::string format_modified_time(const ObservedFileTime& time) {
    if (time.nanoseconds >= 1'000'000'000U || !std::in_range<std::time_t>(time.unix_seconds)) {
        return "Unavailable";
    }
    const std::time_t seconds = static_cast<std::time_t>(time.unix_seconds);
    std::tm local{};
#if defined(_WIN32)
    const errno_t status = localtime_s(&local, &seconds);
    if (status != 0) return "Unavailable";
#else
    const std::tm* const result = localtime_r(&seconds, &local);
    if (result == nullptr) return "Unavailable";
#endif
    char text[32]{};
    const std::size_t written = std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M", &local);
    if (written == 0U) return "Unavailable";
    return text;
}

std::string format_modified_time(const std::filesystem::file_time_type time) {
    const std::chrono::system_clock::time_point system_time =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            std::chrono::file_clock::to_sys(time));
    const std::time_t seconds = std::chrono::system_clock::to_time_t(system_time);
    std::tm local{};
#if defined(_WIN32)
    if (localtime_s(&local, &seconds) != 0) return "Unavailable";
#else
    if (localtime_r(&seconds, &local) == nullptr) return "Unavailable";
#endif
    char text[32]{};
    if (std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M", &local) == 0) return "Unavailable";
    return text;
}

std::string format_bytes(const std::uintmax_t bytes) {
    static constexpr std::array<std::string_view, 5> units{"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream stream{};
    if (unit == 0) {
        stream << bytes;
    } else if (value < 10.0) {
        stream << std::fixed << std::setprecision(1) << value;
    } else {
        stream << std::fixed << std::setprecision(0) << value;
    }
    stream << ' ' << units[unit];
    const std::string result = stream.str();
    return result;
}

} // namespace file_manager
