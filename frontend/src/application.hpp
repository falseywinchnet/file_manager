#pragma once

#include "file_manager/file_operations.hpp"
#include "file_manager/filesystem_model.hpp"
#include "file_manager/internal_drag.hpp"
#include "file_manager/checksum.hpp"
#include "file_manager/platform_commands.hpp"
#include "file_manager/preview.hpp"
#include "file_manager_sapphire.gui_tree.wf.hpp"
#include "fileman_orchestrator/client.hpp"

#include "gui_forms/gui_forms.hpp"
#include "gui_forms/platform/macos_host.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace file_manager {

class Application final : public std::enable_shared_from_this<Application> {
public:
    Application(std::filesystem::path protected_root,
                std::optional<std::filesystem::path> quarantine_root,
                bool mutations_enabled,
                std::string engine_root_id);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    [[nodiscard]] std::unique_ptr<gui_forms::Window> make_window();
    void bind_host(std::function<void()> wake,
                   std::function<void()> request_close);
    void drain_ui();
    void stop();

private:
    using NativeForm = web_forms_generated_file_manager_sapphire::NativeForm;

    void install_dynamic_controls();
    void install_command_surfaces();
    void install_handlers();
    std::shared_ptr<gui_forms::Command> make_command(
        std::string id, std::string text, std::string description,
        std::function<void()> action);
    void show_menu(const std::shared_ptr<gui_forms::ContextMenu>& menu,
                   const gui_forms::Control::Ptr& owner);
    void update_command_state();
    void rebuild_breadcrumb();
    void set_path_editing(bool editing);
    void rebuild_object_order();
    void set_view_mode(gui_forms::ObjectViewMode mode);
    void set_sort_mode(std::string mode);
    void show_properties();
    void toggle_folder_tree();
    void toggle_selection_pane();
    void show_about();
    void post_worker(std::function<void()> work);
    void post_ui(std::function<void()> work);
    void worker_loop();
    void request_bootstrap();
    void request_settings();
    void request_services();
    void show_settings();
    void hide_settings();
    void select_settings_tab(std::string tab, std::string title);
    void apply_settings_state(
        fileman::orchestrator::SettingsSchemaInfo schema,
        fileman::orchestrator::SettingsSnapshotInfo snapshot,
        std::string notice = {});
    void rebuild_settings_page();
    void rebuild_services_page();
    void run_service_command(std::string service_id,
                             std::string command_id,
                             std::optional<std::string> root_id);
    [[nodiscard]] bool confirm_service_command(
        std::string_view service_id, std::string_view command_id);
    void edit_setting(std::string id,
                      fileman::orchestrator::SettingValue value);
    void cancel_settings_edits();
    void reset_settings_page();
    void apply_settings_changes();
    void apply_settings_commit(
        fileman::orchestrator::SettingsCommitInfo commit);
    void update_settings_actions();
    void apply_runtime_settings();
    void request_navigation(std::filesystem::path path, bool add_history);
    void apply_directory(DirectorySnapshot snapshot, bool add_history);
    void navigate_back();
    void navigate_forward();
    void navigate_up();
    void toggle_view_mode();
    void apply_filter();
    void request_engine_search(bool next_page = false);
    void apply_engine_search(fileman::orchestrator::SearchPageInfo page,
                             std::string query,
                             std::uint64_t generation,
                             bool append);
    [[nodiscard]] std::optional<DirectoryEntry> selected_entry() const;
    void update_selection(std::string_view stable_id);
    void request_preview(const DirectoryEntry& entry);
    void apply_preview(PreviewResult result, std::string stable_id,
                       std::uint64_t generation);
    void reset_preview();
    void request_checksum();
    void apply_checksum(ChecksumResult result, std::string expected,
                        std::uint64_t generation);
    void request_open();
    void request_terminal();
    void copy_current_path();
    void run_platform_command(PlatformCommandKind kind,
                              const DirectoryEntry& entry);
    void apply_platform_command(PlatformCommandKind kind,
                                PlatformCommandResult result,
                                std::filesystem::path path);
    void update_mutation_controls();
    void activate(std::string_view stable_id);
    void request_create_folder();
    void begin_rename();
    void commit_rename(std::string basename);
    void cancel_rename();
    void capture_transfer(bool move);
    void paste_transfer();
    void observe_object_pointer(const gui_forms::PointerEvent& event);
    void request_internal_drop(const DirectoryEntry& source,
                               const DirectoryEntry& destination,
                               bool copy);
    void request_quarantine();
    void request_undo();
    void apply_operation(OperationResult result);
    void apply_transfer(OperationResult result, std::uint64_t generation);
    void show_operation_failure(const OperationResult& result);
    void rebuild_tree(const DirectorySnapshot& snapshot);
    void set_status(std::string text, std::string summary);

    std::filesystem::path protected_root_;
    std::filesystem::path location_;
    std::string engine_root_id_;
    std::string filter_;
    std::atomic_uint64_t requested_generation_{};
    std::atomic_uint64_t search_generation_{};
    std::atomic_uint64_t preview_generation_{};
    std::atomic_uint64_t checksum_generation_{};
    std::uint64_t applied_generation_{};
    std::vector<std::filesystem::path> history_;
    std::size_t history_index_{};
    std::unordered_map<std::string, DirectoryEntry> entries_;
    std::unordered_map<std::string, std::filesystem::path> tree_locations_;
    std::unique_ptr<FileOperationService> operations_;
    std::optional<ObjectIdentity> pending_selection_identity_;
    std::optional<std::string> pending_delete_id_;
    std::optional<std::string> rename_target_id_;
    struct PendingTransfer final {
        DirectoryEntry entry;
        bool move{};
    };
    std::optional<PendingTransfer> pending_transfer_;
    std::atomic_uint64_t transfer_generation_{};
    bool transfer_in_flight_{};
    std::string pointer_drag_hover_id_;
    InternalDragController pointer_drag_;
    std::optional<fileman::orchestrator::SettingsSchemaInfo> settings_schema_;
    std::optional<fileman::orchestrator::SettingsSnapshotInfo> settings_snapshot_;
    std::optional<fileman::orchestrator::ServicesSnapshotInfo> services_snapshot_;
    std::unordered_map<std::string, fileman::orchestrator::SettingValue>
        pending_settings_;
    std::string settings_tab_{"general"};
    bool settings_open_{};
    bool settings_loading_{};
    bool settings_apply_in_flight_{};
    bool services_loading_{};
    bool service_command_in_flight_{};
    bool checksum_in_flight_{};
    std::string service_notice_;
    bool show_hidden_{};
    bool show_extensions_{true};
    bool checksum_visible_{true};
    bool terminal_visible_{true};
    bool builtin_previews_enabled_{true};
    bool search_showing_{};
    bool search_loading_{};
    std::optional<fileman::orchestrator::SearchCursorInfo> search_cursor_;
    std::vector<std::string> search_order_;
    std::uint64_t next_host_request_id_{1};

    NativeForm form_;
    std::shared_ptr<gui_forms::MenuStrip> menu_strip_;
    std::shared_ptr<gui_forms::SplitContainer> workspace_split_;
    std::shared_ptr<gui_forms::SplitContainer> selection_split_;
    std::shared_ptr<gui_forms::Panel> content_surface_;
    std::shared_ptr<gui_forms::Panel> inspector_surface_;
    std::shared_ptr<gui_forms::PropertyList> settings_property_list_;
    std::shared_ptr<gui_forms::FlowLayoutPanel> breadcrumb_;
    std::shared_ptr<gui_forms::Button> path_edit_button_;
    std::shared_ptr<gui_forms::TextBox> path_box_;
    std::shared_ptr<gui_forms::TextBox> search_box_;
    std::shared_ptr<gui_forms::TreeView> tree_;
    std::shared_ptr<gui_forms::ObjectView> objects_;
    std::shared_ptr<gui_forms::CorrespondenceView> correspondence_;
    std::shared_ptr<gui_forms::PropertyList> property_list_;
    std::shared_ptr<gui_forms::PictureBox> preview_picture_;
    std::shared_ptr<gui_forms::Label> preview_text_;
    std::shared_ptr<gui_forms::TextBox> expected_checksum_box_;
    std::shared_ptr<gui_forms::TextBox> rename_box_;
    std::vector<std::shared_ptr<gui_forms::Button>> breadcrumb_buttons_;
    std::vector<std::shared_ptr<gui_forms::Label>> breadcrumb_separators_;
    std::vector<std::shared_ptr<gui_forms::Command>> commands_;
    std::vector<std::shared_ptr<gui_forms::ContextMenu>> menus_;
    std::shared_ptr<gui_forms::ContextMenu> move_copy_menu_;
    std::shared_ptr<gui_forms::ContextMenu> view_menu_;
    std::shared_ptr<gui_forms::ContextMenu> sort_menu_;
    std::shared_ptr<gui_forms::ContextMenu> object_menu_;
    std::shared_ptr<gui_forms::ContextMenu> background_menu_;
    std::shared_ptr<gui_forms::Command> command_open_;
    std::shared_ptr<gui_forms::Command> command_new_folder_;
    std::shared_ptr<gui_forms::Command> command_copy_;
    std::shared_ptr<gui_forms::Command> command_move_;
    std::shared_ptr<gui_forms::Command> command_paste_;
    std::shared_ptr<gui_forms::Command> command_undo_;
    std::shared_ptr<gui_forms::Command> command_delete_;
    std::shared_ptr<gui_forms::Command> command_rename_;
    std::shared_ptr<gui_forms::Command> command_properties_;
    std::shared_ptr<gui_forms::Command> command_select_all_;
    std::shared_ptr<gui_forms::Command> command_back_;
    std::shared_ptr<gui_forms::Command> command_forward_;
    std::shared_ptr<gui_forms::Command> command_up_;
    std::shared_ptr<gui_forms::Command> command_root_;
    std::shared_ptr<gui_forms::Command> command_icons_;
    std::shared_ptr<gui_forms::Command> command_details_;
    std::shared_ptr<gui_forms::Command> command_sort_name_;
    std::shared_ptr<gui_forms::Command> command_sort_kind_;
    std::shared_ptr<gui_forms::Command> command_sort_size_;
    std::shared_ptr<gui_forms::Command> command_sort_modified_;
    std::shared_ptr<gui_forms::Command> command_refresh_;
    std::shared_ptr<gui_forms::Command> command_toggle_tree_;
    std::shared_ptr<gui_forms::Command> command_toggle_selection_;
    std::shared_ptr<gui_forms::Command> command_checksum_;
    std::shared_ptr<gui_forms::Command> command_terminal_;
    std::shared_ptr<gui_forms::Command> command_copy_path_;
    std::shared_ptr<gui_forms::Command> command_settings_;
    std::shared_ptr<gui_forms::Command> command_close_;
    std::vector<gui_forms::SubscriptionToken> subscriptions_;
    std::vector<gui_forms::SubscriptionToken> breadcrumb_subscriptions_;
    std::vector<gui_forms::SubscriptionToken> settings_subscriptions_;
    gui_forms::Window* window_{};
    gui_forms::ImageId preview_image_id_{};

    std::thread worker_;
    std::mutex worker_mutex_;
    std::condition_variable worker_cv_;
    std::queue<std::function<void()>> worker_queue_;
    std::atomic_bool stopping_{};
    bool details_mode_{true};

    std::mutex ui_mutex_;
    std::queue<std::function<void()>> ui_queue_;
    std::function<void()> wake_;
    std::function<void()> request_close_;
    std::string sort_mode_{"name"};
};

} // namespace file_manager
