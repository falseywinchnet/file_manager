#include "file_manager/filesystem_model.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

namespace file_manager {
namespace {

std::string ascii_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string stable_id_for(const std::filesystem::path& path,
                          const ObjectIdentity& identity) {
    if (identity.available()) {
        std::ostringstream stream;
        stream << "fm.object." << std::hex << std::setw(16) << std::setfill('0')
               << identity.device << '.' << std::setw(16) << identity.inode;
        return stream.str();
    }
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offset;
    const auto bytes = path.generic_string();
    for (const unsigned char value : bytes) {
        hash ^= value;
        hash *= prime;
    }
    std::ostringstream stream;
    stream << "fm.object." << std::hex << std::setw(16) << std::setfill('0')
           << hash;
    return stream.str();
}

std::filesystem::file_type file_type_from_mode(const mode_t mode) {
    if (S_ISREG(mode)) return std::filesystem::file_type::regular;
    if (S_ISDIR(mode)) return std::filesystem::file_type::directory;
    if (S_ISLNK(mode)) return std::filesystem::file_type::symlink;
    if (S_ISBLK(mode)) return std::filesystem::file_type::block;
    if (S_ISCHR(mode)) return std::filesystem::file_type::character;
    if (S_ISFIFO(mode)) return std::filesystem::file_type::fifo;
    if (S_ISSOCK(mode)) return std::filesystem::file_type::socket;
    return std::filesystem::file_type::unknown;
}

bool has_symlink_component(const std::filesystem::path& root,
                           const std::filesystem::path& candidate) {
    auto cursor = root;
    const auto relative = candidate.lexically_relative(root);
    for (const auto& component : relative) {
        if (component == ".") continue;
        cursor /= component;
        std::error_code error;
        const auto status = std::filesystem::symlink_status(cursor, error);
        if (error) return false;
        if (std::filesystem::is_symlink(status)) return true;
    }
    return false;
}

EntryKind kind_for(const std::filesystem::directory_entry& entry,
                   const std::filesystem::file_status status) {
    if (std::filesystem::is_symlink(status)) return EntryKind::symlink;
    if (std::filesystem::is_directory(status)) return EntryKind::folder;
    const auto extension = ascii_lower(entry.path().extension().string());
    static constexpr std::array image_extensions{
        ".png", ".jpg", ".jpeg", ".gif", ".webp", ".tiff", ".heic"};
    static constexpr std::array archive_extensions{
        ".zip", ".tar", ".gz", ".bz2", ".xz", ".7z"};
    static constexpr std::array audio_extensions{
        ".wav", ".mp3", ".m4a", ".aac", ".flac", ".ogg"};
    static constexpr std::array code_extensions{
        ".c", ".cc", ".cpp", ".h", ".hpp", ".go", ".rs", ".py",
        ".js", ".ts", ".html", ".css", ".json", ".toml", ".yaml"};
    const auto contains = [&extension](const auto& values) {
        return std::find(values.begin(), values.end(), extension) != values.end();
    };
    if (contains(image_extensions)) return EntryKind::image;
    if (contains(archive_extensions)) return EntryKind::archive;
    if (contains(audio_extensions)) return EntryKind::audio;
    if (contains(code_extensions)) return EntryKind::code;
    return std::filesystem::is_regular_file(status) ? EntryKind::document
                                                     : EntryKind::other;
}

std::string modified_text(const std::filesystem::directory_entry& entry) {
    std::error_code error;
    const auto time = entry.last_write_time(error);
    if (error) return "Unavailable";
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        time.time_since_epoch()).count();
    return "filesystem time " + std::to_string(seconds);
}

} // namespace

std::filesystem::path canonical_existing_directory(
    const std::filesystem::path& path) {
    std::error_code error;
    const auto canonical = std::filesystem::canonical(path, error);
    if (error) {
        throw std::runtime_error("cannot resolve directory: " + error.message());
    }
    const auto status = std::filesystem::status(canonical, error);
    if (error || !std::filesystem::is_directory(status)) {
        throw std::runtime_error("path is not an available directory");
    }
    return canonical;
}

bool path_is_within(const std::filesystem::path& root,
                    const std::filesystem::path& candidate) {
    auto root_it = root.begin();
    auto candidate_it = candidate.begin();
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
    const auto lexical = candidate.lexically_normal();
    if (!canonical_root.is_absolute() || !lexical.is_absolute()) return {};
    if (path_is_within(canonical_root, lexical)) return lexical;

    std::vector<std::filesystem::path> suffix;
    auto cursor = lexical;
    for (;;) {
        std::error_code error;
        if (std::filesystem::equivalent(canonical_root, cursor, error) &&
            !error) {
            auto rebased = canonical_root;
            for (auto component = suffix.rbegin(); component != suffix.rend();
                 ++component) {
                rebased /= *component;
            }
            return rebased.lexically_normal();
        }
        const auto parent = cursor.parent_path();
        if (parent.empty() || parent == cursor) break;
        suffix.push_back(cursor.filename());
        cursor = parent;
    }
    return {};
}

bool path_route_has_symlink(const std::filesystem::path& canonical_root,
                            const std::filesystem::path& candidate) {
    const auto lexical = candidate.lexically_normal();
    return path_is_within(canonical_root, lexical) &&
        has_symlink_component(canonical_root, lexical);
}

ObjectIdentity observe_identity(const std::filesystem::path& path) {
    struct stat observed {};
    if (::lstat(path.c_str(), &observed) != 0) return {};
    const auto modified_nanoseconds =
#if defined(__APPLE__)
        static_cast<std::uint64_t>(observed.st_mtimespec.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(observed.st_mtimespec.tv_nsec);
#else
        static_cast<std::uint64_t>(observed.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(observed.st_mtim.tv_nsec);
#endif
    return {static_cast<std::uint64_t>(observed.st_dev),
            static_cast<std::uint64_t>(observed.st_ino),
            static_cast<std::uint64_t>(observed.st_size),
            modified_nanoseconds,
            file_type_from_mode(observed.st_mode)};
}

bool object_is_hidden(const std::filesystem::path& path,
                      const std::string_view name) {
    if (!name.empty() && name.front() == '.') return true;
#if defined(__APPLE__)
    struct stat observed {};
    return ::lstat(path.c_str(), &observed) == 0 &&
        (observed.st_flags & UF_HIDDEN) != 0;
#else
    (void)path;
    return false;
#endif
}

DirectorySnapshot read_directory(const std::filesystem::path& root,
                                 const std::filesystem::path& requested,
                                 const std::string_view filter,
                                 const std::uint64_t generation,
                                 const CancellationCheck& cancelled,
                                 const bool show_hidden) {
    DirectorySnapshot result;
    result.root = root;
    result.location = requested;
    result.generation = generation;
    try {
        result.root = canonical_existing_directory(root);
        const auto lexical_request = (requested.is_absolute()
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

        const auto needle = ascii_lower(std::string(filter));
        std::error_code error;
        std::filesystem::directory_iterator iterator(
            result.location,
            std::filesystem::directory_options::skip_permission_denied,
            error);
        if (error) {
            result.error = "cannot enumerate location: " + error.message();
            return result;
        }
        constexpr std::size_t maximum_entries = 50'000;
        for (const auto& entry : iterator) {
            if (cancelled && cancelled()) {
                result.entries.clear();
                result.cancelled = true;
                return result;
            }
            if (result.entries.size() >= maximum_entries) {
                result.error = "location exceeds the 50,000-item frontend bound";
                break;
            }
            const auto name = entry.path().filename().string();
            if (!show_hidden && object_is_hidden(entry.path(), name)) continue;
            if (!needle.empty() && ascii_lower(name).find(needle) == std::string::npos) {
                continue;
            }
            const auto status = entry.symlink_status(error);
            if (error) {
                error.clear();
                continue;
            }
            DirectoryEntry item;
            item.path = entry.path();
            item.identity = observe_identity(item.path);
            item.stable_id = stable_id_for(item.path, item.identity);
            item.name = name;
            item.kind = kind_for(entry, status);
            item.directory = item.kind == EntryKind::folder;
            if (item.directory) {
                item.secondary_text = "Folder";
            } else if (std::filesystem::is_regular_file(status)) {
                const auto size = entry.file_size(error);
                item.secondary_text = error ? "File" : format_bytes(size);
                error.clear();
            } else if (item.kind == EntryKind::symlink) {
                item.secondary_text = "Symbolic link · not followed";
            } else {
                item.secondary_text = "Filesystem object";
            }
            item.modified_text = modified_text(entry);
            result.entries.push_back(std::move(item));
        }
        std::sort(result.entries.begin(), result.entries.end(),
                  [](const DirectoryEntry& left, const DirectoryEntry& right) {
            if (left.directory != right.directory) return left.directory;
            const auto left_name = ascii_lower(left.name);
            const auto right_name = ascii_lower(right.name);
            if (left_name != right_name) return left_name < right_name;
            return left.name < right.name;
        });
    } catch (const std::exception& error) {
        result.error = error.what();
    }
    return result;
}

std::string format_bytes(const std::uintmax_t bytes) {
    static constexpr std::array units{"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream stream;
    if (unit == 0) {
        stream << bytes;
    } else if (value < 10.0) {
        stream << std::fixed << std::setprecision(1) << value;
    } else {
        stream << std::fixed << std::setprecision(0) << value;
    }
    stream << ' ' << units[unit];
    return stream.str();
}

} // namespace file_manager
