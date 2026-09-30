#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace file_manager {
[[nodiscard]] bool valid_platform_basename(std::string_view name);
[[nodiscard]] std::filesystem::path user_home_directory();
[[nodiscard]] std::vector<std::filesystem::path> local_volume_roots();
[[nodiscard]] std::string path_generic_utf8(const std::filesystem::path& path);
[[nodiscard]] std::string path_utf8(const std::filesystem::path& path);
[[nodiscard]] std::filesystem::path path_from_utf8(std::string_view text);
} // namespace file_manager
