#include "fixture_links.hpp"
#include "file_manager/preview.hpp"

#include <cstdlib>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unistd.h>

namespace {

struct FixtureDirectory final {
    explicit FixtureDirectory(const std::filesystem::path& value) : path(value) {
        const bool created = std::filesystem::create_directory(path);
        if (!created) throw std::runtime_error("fixture directory already exists");
    }
    ~FixtureDirectory() {
        std::error_code ignored{};
        std::filesystem::remove_all(path, ignored);
    }
    FixtureDirectory(const FixtureDirectory&) = delete;
    FixtureDirectory& operator=(const FixtureDirectory&) = delete;
    std::filesystem::path path{};
};

void require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "preview test failed: " << message << '\n';
        throw std::runtime_error(std::string(message));
    }
}

void write(const std::filesystem::path& path, const std::string_view bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

int main() {
    try {
        const std::filesystem::path root = std::filesystem::temp_directory_path() /
            ("file-manager-preview-" + std::to_string(::getpid()));
        const FixtureDirectory fixture(root);
        const std::filesystem::path text = root / "notes Ω.md";
        write(text, "# Hello\n\nUTF-8 Ω\tline\n");
        file_manager::PreviewResult result = file_manager::load_preview(
            root, text, file_manager::observe_identity(text));
        require(result.kind == file_manager::PreviewKind::text &&
                    result.text_utf8.find("UTF-8 Ω") != std::string::npos &&
                    result.text_utf8.find('\t') == std::string::npos,
                "UTF-8 text must produce a bounded readable preview");

        const std::filesystem::path png = root / "image.png";
        const std::string png_signature("\x89PNG\r\n\x1a\n", 8);
        write(png, png_signature + "fixture");
        result = file_manager::load_preview(
            root, png, file_manager::observe_identity(png));
        require(result.kind == file_manager::PreviewKind::png &&
                    result.png_bytes.size() == 15U,
                "PNG signature must route encoded bytes to GUI.Forms");

        const std::filesystem::path binary = root / "not-text.txt";
        write(binary, std::string("a\0b", 3));
        result = file_manager::load_preview(
            root, binary, file_manager::observe_identity(binary));
        require(result.kind == file_manager::PreviewKind::unsupported &&
                    result.code == "not-utf8-text",
                "NUL-bearing text must remain unsupported");

        const std::filesystem::path unsupported = root / "archive.zip";
        write(unsupported, "PK");
        result = file_manager::load_preview(
            root, unsupported, file_manager::observe_identity(unsupported));
        require(result.kind == file_manager::PreviewKind::unsupported &&
                    result.code == "format-unavailable",
                "unadmitted formats must state unavailability without reading");

        const file_manager::ObjectIdentity stale = file_manager::observe_identity(text);
        std::filesystem::remove(text);
        write(text, "replacement");
        result = file_manager::load_preview(root, text, stale);
        require(result.kind == file_manager::PreviewKind::changed,
                "replacement must invalidate a selected preview revision");

        if (create_fixture_link("notes Ω.md", root / "linked.md")) {

        result = file_manager::load_preview(
            root, root / "linked.md",
            file_manager::observe_identity(root / "linked.md"));
        require(result.kind == file_manager::PreviewKind::refused &&
                    result.code == "symlink-refused",
                "preview must not follow symbolic links");
        }

        std::error_code ignored{};
        std::filesystem::remove_all(root, ignored);
        std::cout << "preview tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
