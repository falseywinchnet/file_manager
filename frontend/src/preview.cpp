#include "file_manager/platform_paths.hpp"
#include "file_manager/preview.hpp"
#include "native_file.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <cctype>
#include <optional>

namespace file_manager {
namespace {

PreviewResult terminal(const PreviewKind kind, std::string code,
                       std::string message,
                       const std::filesystem::path& path) {
    PreviewResult result{};
    result.kind = kind;
    result.code = std::move(code);
    result.message = std::move(message);
    result.path = path;
    return result;
}

bool text_extension(std::string extension) {
    for (char& character : extension) {
        const unsigned char byte = static_cast<unsigned char>(character);
        const int lowered = std::tolower(byte);
        character = static_cast<char>(lowered);
    }
    static constexpr std::array<std::string_view, 26> extensions{
        ".txt", ".md", ".csv", ".tsv", ".json", ".xml", ".yaml",
        ".yml", ".toml", ".ini", ".log", ".c", ".cc", ".cpp",
        ".h", ".hpp", ".m", ".mm", ".go", ".rs", ".py", ".sh",
        ".html", ".css", ".js", ".ts"};
    const bool supported = std::find(extensions.begin(), extensions.end(), extension) !=
        extensions.end();
    return supported;
}

// Return the complete UTF-8 prefix. Only a code point cut by our read limit
// may be omitted; incomplete data at the actual end of a file is malformed.
std::optional<std::size_t> utf8_preview_extent(const std::string_view value,
                                             const bool truncated) noexcept {
    std::size_t index{};
    while (index < value.size()) {
        const unsigned char first = static_cast<unsigned char>(value[index]);
        std::size_t continuation{};
        std::uint32_t codepoint{};
        if (first <= 0x7fU) {
            ++index;
            continue;
        }
        if ((first & 0xe0U) == 0xc0U) {
            continuation = 1U;
            codepoint = first & 0x1fU;
        } else if ((first & 0xf0U) == 0xe0U) {
            continuation = 2U;
            codepoint = first & 0x0fU;
        } else if ((first & 0xf8U) == 0xf0U) {
            continuation = 3U;
            codepoint = first & 0x07U;
        } else {
            return std::nullopt;
        }
        if (index + continuation >= value.size()) {
            if (!truncated) return std::nullopt;
            const std::size_t remaining = value.size() - index - 1U;
            for (std::size_t offset = 1U; offset <= remaining; ++offset) {
                const unsigned char byte = static_cast<unsigned char>(value[index + offset]);
                if ((byte & 0xc0U) != 0x80U) return std::nullopt;
            }
            return index;
        }
        for (std::size_t offset = 1U; offset <= continuation; ++offset) {
            const unsigned char byte = static_cast<unsigned char>(value[index + offset]);
            if ((byte & 0xc0U) != 0x80U) return std::nullopt;
            codepoint = (codepoint << 6U) | (byte & 0x3fU);
        }
        if ((continuation == 1U && codepoint < 0x80U) ||
            (continuation == 2U && codepoint < 0x800U) ||
            (continuation == 3U && codepoint < 0x10000U) ||
            codepoint > 0x10ffffU ||
            (codepoint >= 0xd800U && codepoint <= 0xdfffU)) {
            return std::nullopt;
        }
        index += continuation + 1U;
    }
    const std::size_t extent = value.size();
    return extent;
}

std::string readable_text(std::string value, const bool truncated) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    for (char& character : value) {
        const unsigned char byte = static_cast<unsigned char>(character);
        if (byte < 0x20U && character != '\n' && character != '\r') {
            character = ' ';
        }
    }
    if (truncated) value += "\n\n… preview limited to 64 KiB";
    return value;
}

} // namespace

PreviewResult load_preview(const std::filesystem::path& protected_root,
                           const std::filesystem::path& selected_path,
                           const ObjectIdentity& expected_identity,
                           const CancellationCheck& cancelled) {
    if (cancelled && cancelled()) {
        const PreviewResult failure_result = terminal(PreviewKind::unavailable, "cancelled",
            "preview cancelled", selected_path);
        return failure_result;
    }
    std::filesystem::path root{};
    try {
        root = canonical_existing_directory(protected_root);
    } catch (const std::exception& error) {
        const PreviewResult failure_result = terminal(PreviewKind::unavailable, "root-unavailable",
                        error.what(), selected_path);
        return failure_result;
    }
    std::error_code absolute_error{};
    const std::filesystem::path supplied_root = std::filesystem::absolute(
        protected_root, absolute_error).lexically_normal();
    const std::filesystem::path supplied_path = (selected_path.is_absolute()
        ? selected_path
        : supplied_root / selected_path).lexically_normal();
    const std::filesystem::path path = (!absolute_error &&
                       path_is_within(supplied_root, supplied_path))
        ? (root / supplied_path.lexically_relative(supplied_root))
              .lexically_normal()
        : supplied_path;
    if (!path_is_within(root, path)) {
        const PreviewResult failure_result = terminal(PreviewKind::refused, "outside-protected-root",
                        "preview path is outside the protected root", path);
        return failure_result;
    }
    if (path_route_has_symlink(root, path)) {
        const PreviewResult failure_result = terminal(PreviewKind::refused, "symlink-refused",
                        "preview never follows a symbolic link", path);
        return failure_result;
    }
    const std::string extension = path_utf8(path.extension());
    std::string lowered = extension;
    for (char& character : lowered) {
        const unsigned char byte = static_cast<unsigned char>(character);
        const int lower_byte = std::tolower(byte);
        character = static_cast<char>(lower_byte);
    }
    const bool png = lowered == ".png";
    const bool text = text_extension(lowered);
    if (!png && !text) {
        const PreviewResult failure_result = terminal(PreviewKind::unsupported, "format-unavailable",
                        "Preview is not available for this file type. Supported now: PNG images and UTF-8 text files.",
                        path);
        return failure_result;
    }

    NativeReadFile descriptor(path);
    if (!descriptor.available()) {
        const PreviewResult failure_result = terminal(PreviewKind::unavailable, "open-failed",
            std::string("cannot open preview: ") + descriptor.error_message(), path);
        return failure_result;
    }
    const ObjectIdentity opened = descriptor.identity();
    if (opened.type != std::filesystem::file_type::regular) {
        const PreviewResult failure_result = terminal(PreviewKind::refused, "not-regular-file",
                        "preview accepts one regular file", path);
        return failure_result;
    }
    if (!opened.available() ||
        (expected_identity.available() &&
         !expected_identity.same_revision(opened))) {
        const PreviewResult failure_result = terminal(PreviewKind::changed, "selection-changed",
                        "selected file changed before preview began", path);
        return failure_result;
    }
    const std::size_t limit = png ? maximum_png_preview_bytes
                           : maximum_text_preview_bytes;
    const std::uint64_t total = opened.size;
    if (png && total > limit) {
        const PreviewResult failure_result = terminal(PreviewKind::unsupported, "image-too-large",
                        "PNG preview exceeds the 16 MiB encoded-byte bound",
                        path);
        return failure_result;
    }
    std::vector<std::byte> bytes{};
    bytes.resize(static_cast<std::size_t>(std::min<std::uint64_t>(total, limit)));
    std::size_t offset{};
    while (offset < bytes.size()) {
        if (cancelled && cancelled()) {
            const PreviewResult failure_result = terminal(PreviewKind::unavailable, "cancelled",
                            "preview cancelled", path);
        return failure_result;
        }
        const std::ptrdiff_t count = descriptor.read(bytes.data() + offset,
                                                     bytes.size() - offset);
        if (count < 0) {
            const PreviewResult failure_result = terminal(PreviewKind::unavailable, "read-failed",
                std::string("cannot read preview: ") + descriptor.error_message(),
                path);
        return failure_result;
        }
        if (count == 0) break;
        offset += static_cast<std::size_t>(count);
    }
    bytes.resize(offset);
    const ObjectIdentity after = descriptor.identity();
    if (!opened.same_revision(after) ||
        !opened.same_revision(observe_identity(path))) {
        const PreviewResult failure_result = terminal(PreviewKind::changed, "changed-during-read",
                        "file changed or was replaced during preview", path);
        return failure_result;
    }

    PreviewResult result{};
    result.path = path;
    result.identity = opened;
    result.code = "ok";
    if (png) {
        static constexpr std::array<std::byte, 8> signature{
            std::byte{0x89}, std::byte{'P'}, std::byte{'N'}, std::byte{'G'},
            std::byte{0x0d}, std::byte{0x0a}, std::byte{0x1a}, std::byte{0x0a}};
        if (bytes.size() < signature.size() ||
            !std::equal(signature.begin(), signature.end(), bytes.begin())) {
            const PreviewResult failure_result = terminal(PreviewKind::unsupported, "invalid-png",
                            "file extension says PNG but its signature does not",
                            path);
        return failure_result;
        }
        result.kind = PreviewKind::png;
        result.message = "bounded PNG preview";
        result.png_bytes = std::move(bytes);
    } else {
        std::string value{};
        if (!bytes.empty()) {
            value.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        }
        const bool truncated = total > maximum_text_preview_bytes;
        const std::optional<std::size_t> extent = utf8_preview_extent(value, truncated);
        if (value.find('\0') != std::string::npos || !extent) {
            const PreviewResult failure_result = terminal(PreviewKind::unsupported, "not-utf8-text",
                            "text preview requires valid UTF-8 without NUL bytes",
                            path);
        return failure_result;
        }
        value.resize(*extent);
        result.kind = PreviewKind::text;
        result.message = "bounded UTF-8 text preview";
        result.text_utf8 = readable_text(
            std::move(value), truncated);
    }
    return result;
}

} // namespace file_manager
