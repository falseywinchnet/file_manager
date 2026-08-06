#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace file_manager_demoboard {

struct FixtureObject final {
    std::string id;
    std::string name;
    std::string kind;
    std::string size;
    std::string icon_role;
    bool selected{};
};

struct CapabilityEntry final {
    std::string id;
    std::string state;
    std::string detail;
};

struct FixturePath final {
    std::string id;
    std::string path;
    std::vector<std::string> segments;
    bool available{true};
};

struct FixturePathCompletion final {
    std::string id;
    std::string path;
};

struct FixturePathResolution final {
    bool valid{};
    std::string expanded_path;
    std::string destination_id;
    std::string explanation;
};

class FixtureCatalogue final {
public:
    [[nodiscard]] static const FixtureCatalogue& instance();
    [[nodiscard]] std::string_view id() const noexcept { return id_; }
    [[nodiscard]] std::uint64_t generation() const noexcept { return generation_; }
    [[nodiscard]] std::string_view current_path() const noexcept {
        return current_path_;
    }
    [[nodiscard]] std::span<const FixtureObject> project_objects() const noexcept {
        return project_objects_;
    }
    [[nodiscard]] const FixturePath& current_path_stack() const noexcept {
        return current_path_stack_;
    }
    [[nodiscard]] std::span<const FixturePath> recent_paths() const noexcept {
        return recent_paths_;
    }
    [[nodiscard]] std::span<const FixturePathCompletion> path_completions()
        const noexcept { return path_completions_; }
    [[nodiscard]] FixturePathResolution resolve_path(
        std::string_view input) const;
    [[nodiscard]] std::vector<FixturePathCompletion> complete_path(
        std::string_view input) const;

private:
    FixtureCatalogue();

    std::string id_;
    std::uint64_t generation_{};
    std::string current_path_;
    std::vector<FixtureObject> project_objects_;
    FixturePath current_path_stack_;
    std::vector<FixturePath> recent_paths_;
    std::vector<FixturePathCompletion> path_completions_;
};

[[nodiscard]] std::vector<CapabilityEntry> initial_capability_report();

} // namespace file_manager_demoboard
