#pragma once

#include "file_manager/filesystem_model.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace file_manager {

enum class DocumentPickerProfile : std::uint8_t {
    open_file,
    open_files,
    select_folder,
    save_as,
    import_files,
    export_file,
};

struct DocumentTypeFilter final {
    std::string id;
    std::string label;
    std::vector<std::string> extensions;
};

struct DocumentPickerRequest final {
    DocumentPickerProfile profile{DocumentPickerProfile::open_file};
    std::filesystem::path protected_root;
    std::filesystem::path initial_location;
    std::string owner_application_id;
    std::size_t maximum_selection{1};
    bool show_hidden{};
    bool allow_create_folder{};
    bool allow_native_fallback{};
    bool orchestrator_session_valid{true};
    std::vector<DocumentTypeFilter> filters;
    std::string active_filter_id;
    std::string suggested_name;
    std::string default_extension;
};

struct DocumentSelectionObservation final {
    std::filesystem::path path;
    ObjectIdentity identity;
    bool existing{};
};

enum class DocumentPickerTerminal : std::uint8_t {
    accepted,
    cancelled,
    validation_error,
    unavailable,
    overwrite_confirmation_required,
};

struct DocumentPickerResult final {
    DocumentPickerTerminal terminal{DocumentPickerTerminal::validation_error};
    std::string code;
    std::string message;
    std::vector<DocumentSelectionObservation> selections;
    bool native_fallback_permitted{};

    [[nodiscard]] bool accepted() const noexcept {
        return terminal == DocumentPickerTerminal::accepted;
    }
};

// FileSelectionController is the independently consumable semantic boundary.
// It navigates directly from the filesystem and therefore does not require the
// Engine, but acceptance fails closed when its Orchestrator selection session
// is no longer valid.
class FileSelectionController final {
public:
    explicit FileSelectionController(DocumentPickerRequest request);

    [[nodiscard]] const DocumentPickerRequest& request() const noexcept;
    [[nodiscard]] const DirectorySnapshot& browser() const noexcept;
    [[nodiscard]] const std::vector<std::string>& selected_ids() const noexcept;
    [[nodiscard]] const std::string& filename() const noexcept;
    [[nodiscard]] const std::string& active_filter_id() const noexcept;
    [[nodiscard]] const std::string& last_error() const noexcept;
    [[nodiscard]] bool show_hidden() const noexcept;

    [[nodiscard]] bool navigate(const std::filesystem::path& location);
    [[nodiscard]] bool refresh();
    [[nodiscard]] bool set_selection(std::vector<std::string> stable_ids);
    [[nodiscard]] bool set_filename(std::string filename);
    [[nodiscard]] bool set_active_filter(std::string filter_id);
    [[nodiscard]] bool set_show_hidden(bool show_hidden);
    void set_orchestrator_session_valid(bool valid) noexcept;

    [[nodiscard]] DocumentPickerResult accept(
        bool overwrite_confirmed = false) const;
    [[nodiscard]] DocumentPickerResult cancel() const;

private:
    [[nodiscard]] bool profile_accepts_multiple() const noexcept;
    [[nodiscard]] bool profile_saves() const noexcept;
    [[nodiscard]] bool entry_visible(const DirectoryEntry& entry) const;
    [[nodiscard]] const DirectoryEntry* find_entry(
        std::string_view stable_id) const noexcept;
    [[nodiscard]] DocumentPickerResult selection_error(
        std::string code, std::string message) const;

    DocumentPickerRequest request_;
    DirectorySnapshot browser_;
    std::vector<std::string> selected_ids_;
    std::string filename_;
    std::string active_filter_id_;
    std::string last_error_;
    bool show_hidden_{};
    bool orchestrator_session_valid_{};
    std::uint64_t generation_{};
};

} // namespace file_manager
