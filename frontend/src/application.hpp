#pragma once

#include "file_manager/file_operations.hpp"
#include "file_manager/document_picker.hpp"
#include "file_manager/filesystem_model.hpp"
#include "file_manager/internal_drag.hpp"
#include "file_manager/checksum.hpp"
#include "file_manager/platform_commands.hpp"
#include "file_manager/preview.hpp"
#include "file_manager_sapphire.gui_tree.wf.hpp"
#include "fileman_orchestrator/client.hpp"

#include "gui_forms/gui_forms.hpp"
#if defined(__APPLE__)
#include "gui_forms/platform/macos_host.hpp"
#endif

#include <atomic>
#include <array>
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
#include <unordered_set>
#include <vector>

namespace file_manager {

class ApplicationInteractionProbe;

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
    void bind_secondary_surfaces(
        std::function<void(const std::filesystem::path&)> show_open_picker,
        std::function<void()> show_about_window);
    void document_picker_completed(const DocumentPickerResult& result);
    void drain_ui();
    void stop();

private:
    struct SuggestionUnavailable;
    struct SuggestionReady;
    struct SuggestionCancelled;
    struct SuggestionNotAdmitted;
    struct SuggestionWork;
    struct ServiceCommandUnavailable;
    struct ServiceCommandReady;
    struct ServiceCommandWork;
    struct SettingsRefreshUnavailable;
    struct SettingsConflictReady;
    struct SettingsCommitReady;
    struct SettingsCommitWork;
    struct NavigationReady;
    struct NavigationCancelled;
    struct NavigationWork;
    struct TreeExpansionReady;
    struct TreeExpansionCancelled;
    struct TreeExpansionWork;
    struct CriteriaUnavailable;
    struct CriteriaReady;
    struct SearchCancelled;
    struct CriteriaWork;
    struct SearchUnavailable;
    struct SearchReady;
    struct SearchWork;
    struct PreviewReady;
    struct PreviewCancelled;
    struct PreviewWork;
    struct ChecksumReady;
    struct ChecksumProgressReady;
    struct ChecksumProgressReport;
    struct ChecksumCancelled;
    struct ChecksumWork;
    struct PlatformCommandReady;
    struct PlatformCommandWork;
    struct CreateFolderReady;
    struct CreateFolderWork;
    struct RenameReady;
    struct RenameWork;
    struct PropertyRenameReady;
    struct PropertyRenameWork;
    struct TransferReady;
    struct TransferCancelled;
    struct TransferWork;
    struct InternalDropReady;
    struct InternalDropCancelled;
    struct InternalDropWork;
    struct QuarantineReady;
    struct QuarantineWork;
    struct UndoReady;
    struct UndoWork;
    struct BootstrapUnavailable;
    struct BootstrapReady;
    struct BootstrapWork;
    struct SettingsUnavailable;
    struct SettingsReady;
    struct SettingsWork;
    struct ServicesUnavailable;
    struct ServicesReady;
    struct ServicesWork;
    friend class ApplicationInteractionProbe;
    friend class ApplicationLatencyProbe;

    using EntryMap = std::unordered_map<std::string, DirectoryEntry>;
    using PathMap = std::unordered_map<std::string, std::filesystem::path>;
    using TreeEntriesMap = std::unordered_map<std::string, std::vector<DirectoryEntry>>;
    using SettingsValueMap = std::unordered_map<std::string, fileman::orchestrator::SettingValue>;

    using NativeForm = web_forms_generated_file_manager_sapphire::NativeForm;

    void install_dynamic_controls();
    void install_command_shelf_controls();
    void install_house_art();
    void install_command_surfaces();
    void select_all_objects();
    void navigate_home();
    void select_home_tree();
    void select_volumes_tree();
    void refresh_view();
    void toggle_settings();
    void close_window();
    void install_accelerators();
    void bind_menu_open(const std::shared_ptr<gui_forms::ContextMenu>& menu,
                        const std::shared_ptr<gui_forms::DropDownButton>& button);
    void bind_command_accelerator(const std::shared_ptr<gui_forms::Command>& command,
                                 gui_forms::KeyGesture gesture,
                                 bool before_focused_route = false,
                                 std::function<bool()> predicate = {});
    void bind_navigation_accelerator(const std::shared_ptr<gui_forms::Command>& command,
                                    std::uint32_t key);
    bool focused_is_object_surface() const;
    bool focused_is_navigation_surface() const;
    bool focused_is_not_text_editor() const;
    bool focused_is_objects() const;
    void install_handlers();
    void bind_button_action(const std::shared_ptr<gui_forms::Button>& button,
                            std::function<void()> action);
    void bind_settings_tab(const std::shared_ptr<gui_forms::Button>& button,
                           std::string id, std::string title);
    void request_more_results();
    void on_shelf_sort_button_drop_down_requested(gui_forms::DropDownButton&);
    void on_shelf_sort_button_drop_down_close_requested(gui_forms::DropDownButton&);
    void on_shelf_view_button_drop_down_requested(gui_forms::DropDownButton&);
    void on_shelf_view_button_drop_down_close_requested(gui_forms::DropDownButton&);
    void on_shelf_move_copy_button_drop_down_requested(gui_forms::DropDownButton&);
    void on_shelf_move_copy_button_drop_down_close_requested(gui_forms::DropDownButton&);
    void on_shelf_overflow_button_drop_down_requested(gui_forms::DropDownButton&);
    void on_shelf_overflow_button_drop_down_close_requested(gui_forms::DropDownButton&);
    void on_tree_root_mode_button_drop_down_requested(gui_forms::DropDownButton&);
    void on_tree_root_mode_button_drop_down_close_requested(gui_forms::DropDownButton&);
    void on_breadcrumb_segment_activated(const std::string& id);
    void on_breadcrumb_edit_committed(const std::string& text);
    void on_breadcrumb_edit_started(const std::string& text);
    void on_breadcrumb_edit_cancelled();
    void on_path_box_text_changed(const std::string& text);
    void on_path_box_focus_observed(const bool focused);
    void on_root_pointer_preview(const gui_forms::PointerEvent& event);
    void on_search_box_committed(const std::string&);
    void on_search_box_cancelled();
    void on_criteria_rack_field_committed(const gui_forms::InstrumentFieldChange&);
    void on_criteria_rack_module_toggled(const gui_forms::InstrumentModuleToggle&);
    void on_criteria_rack_remove_requested(const gui_forms::InstrumentModuleRequest& request);
    void on_rename_box_committed(const std::string& basename);
    void on_objects_selection_changed(const gui_forms::ObjectSelectionChange& change);
    void on_objects_item_activated(const std::string& stable_id);
    void on_objects_context_requested(const gui_forms::ObjectContextRequest& request);
    void on_objects_pointer_observed(const gui_forms::PointerEvent& event);
    void on_tree_item_activated(const std::string& stable_id);
    void on_tree_selection_changed(const gui_forms::TreeSelectionChange& change);
    void on_tree_expansion_changed(const gui_forms::TreeExpansionChange& change);
    void on_property_list_value_committed(const gui_forms::PropertyValueChange& change);
    void on_correspondence_selection_changed(const gui_forms::CorrespondenceSelectionChange& change);
    void on_correspondence_item_activated(const std::string& stable_id);
    void on_correspondence_context_requested(const gui_forms::ObjectContextRequest& request);
    void on_workspace_split_splitter_changed(const gui_forms::SplitChangeEvent&);
    void on_selection_split_splitter_changed(const gui_forms::SplitChangeEvent&);
    void update_adaptive_layout(gui_forms::Rect bounds);
    void update_adaptive_preview();
    void toggle_preview();
    bool focus_search_accelerator();
    void focus_search_command();
    void focus_location_command();
    std::shared_ptr<gui_forms::Command> make_command(
        std::string id, std::string text, std::string description,
        std::function<void()> action);
    void show_menu(const std::shared_ptr<gui_forms::ContextMenu>& menu,
                   const gui_forms::Control::Ptr& owner);
    void show_command_shelf_overflow();
    void update_command_state();
    void focus_active_object_surface();
    void rebuild_breadcrumb();
    void show_breadcrumb_overflow();
    void set_path_editing(bool editing);
    void request_path_suggestions(std::string text);
    void enumerate_path_suggestions(
        std::uint64_t generation, std::string requested_text,
        std::vector<std::filesystem::path> admitted_roots,
        std::filesystem::path current_location,
        std::filesystem::path home_root,
        std::filesystem::path candidate);
    void apply_path_suggestions(
        std::uint64_t generation, std::string requested_text,
        std::string preview, std::vector<std::filesystem::path> paths,
        std::string notice = {});
    void show_path_suggestion_popup();
    void close_path_suggestion_popup(bool restore_focus = false);
    void accept_path_suggestion(std::size_t index);
    void accept_active_path_suggestion();
    void apply_object_sort(std::string mode, gui_forms::ObjectSortDirection direction);
    void sort_object_items(std::vector<gui_forms::ObjectViewItem>& items) const;
    void publish_object_items(std::vector<gui_forms::ObjectViewItem> items);
    void on_objects_sort_requested(const gui_forms::ObjectDetailsSort& request);
    void fit_details_columns(const gui_forms::Rect& bounds);
    void on_details_presentation_changed(const gui_forms::PresentationSettings& settings);
    void set_view_mode(gui_forms::ObjectViewMode mode);
    void set_sort_mode(std::string mode);
    void show_properties();
    void toggle_folder_tree();
    void toggle_selection_pane();
    void show_about();
    void show_open_picker();
    void post_worker(std::function<void()> work);
    void post_ui(std::function<void()> work);
    void worker_loop();
    void navigate_breadcrumb(const std::filesystem::path& target, const gui_forms::CommandInvocation&);
    void on_path_suggestion_activated(const std::size_t index);
    void on_path_suggestion_dismissed(const gui_forms::PopupDismissReason reason);
    void on_setting_committed(const gui_forms::PropertyValueChange& change);
    void on_services_refresh(gui_forms::ButtonBase&);
    void on_service_command(const std::string& service_id, const std::string& command_id, const std::optional<std::string>& root, gui_forms::ButtonBase&);
    bool worker_ready();
    void on_worker_failed(const std::string& message);
    void request_bootstrap();
    void request_settings();
    void request_services();
    void show_settings();
    void hide_settings();
    void select_settings_tab(std::string tab, std::string title);
    void update_settings_tab_state();
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
    void request_tree_expansion(std::filesystem::path path);
    void apply_directory(DirectorySnapshot snapshot, bool add_history);
    void navigate_back();
    void navigate_forward();
    void navigate_up();
    void toggle_view_mode();
    void apply_filter();
    void toggle_criteria_mode();
    void show_criteria();
    void prepare_criteria_surface();
    void add_criteria_module();
    void remove_criteria_module(std::string_view module_id);
    void update_criteria_action_state();
    [[nodiscard]] std::optional<
        std::vector<fileman::orchestrator::SearchExactFilter>>
    criteria_filters();
    void request_engine_criteria(bool next_page = false);
    void apply_engine_criteria(
        fileman::orchestrator::SearchPageInfo page,
        std::vector<fileman::orchestrator::SearchExactFilter> filters,
        std::uint64_t generation,
        bool append);
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
    [[nodiscard]] DirectoryEntry terminal_target() const;
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
    void commit_property_name(std::string basename);
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
    void show_tree_root_menu();
    void select_tree_root_mode(const std::filesystem::path& root);
    void rebuild_tree(const DirectorySnapshot& snapshot);
    void append_tree_children(std::vector<gui_forms::TreeViewItem>& items,
                              const std::filesystem::path& parent,
                              std::size_t depth,
                              std::string& selected_id);
    [[nodiscard]] std::string navigation_label(
        const std::filesystem::path& root) const;
    [[nodiscard]] bool mutation_scope_active() const;
    [[nodiscard]] bool engine_search_available() const;
    void set_status(std::string text, std::string summary);
    void update_browsing_status();
    bool focus_location_accelerator();
    bool refresh_accelerator();

    std::filesystem::path protected_root_{};
    std::filesystem::path home_root_{};
    std::optional<std::filesystem::path> volumes_root_{};
    std::vector<std::filesystem::path> navigation_roots_{};
    std::filesystem::path navigation_root_{};
    std::filesystem::path location_{};
    std::string engine_root_id_{};
    std::string filter_{};
    std::atomic_uint64_t requested_generation_{};
    std::atomic_uint64_t search_generation_{};
    std::atomic_uint64_t preview_generation_{};
    std::atomic_uint64_t checksum_generation_{};
    std::atomic_uint64_t path_suggestion_generation_{};
    std::uint64_t applied_generation_{};
    std::vector<std::filesystem::path> history_{};
    std::size_t history_index_{};
    EntryMap entries_{};
    PathMap tree_locations_{};
    TreeEntriesMap tree_directory_entries_{};
    std::unordered_set<std::string> tree_expanded_paths_{};
    std::atomic_uint64_t tree_generation_{};
    std::filesystem::path tree_root_mode_{};
    // UI-owned projection; FileOperationService remains confined to the worker.
    bool undo_available_{};
    std::unique_ptr<FileOperationService> operations_{};
    std::optional<ObjectIdentity> pending_selection_identity_{};
    std::optional<std::string> pending_delete_id_{};
    std::optional<std::string> rename_target_id_{};
    struct PendingTransfer final {
        DirectoryEntry entry{};
        bool move{};
    };
    std::optional<PendingTransfer> pending_transfer_{};
    std::atomic_uint64_t transfer_generation_{};
    bool transfer_in_flight_{};
    bool property_rename_in_flight_{};
    std::string pointer_drag_hover_id_{};
    InternalDragController pointer_drag_{};
    std::optional<fileman::orchestrator::SettingsSchemaInfo> settings_schema_{};
    std::optional<fileman::orchestrator::SettingsSnapshotInfo> settings_snapshot_{};
    std::optional<fileman::orchestrator::ServicesSnapshotInfo> services_snapshot_{};
    SettingsValueMap pending_settings_{};
    std::string settings_tab_{"general"};
    std::optional<gui_forms::ControlStateRecipes> settings_tab_normal_recipes_{};
    std::optional<gui_forms::ControlStateRecipes> settings_tab_selected_recipes_{};
    bool settings_open_{};
    bool settings_loading_{};
    bool settings_apply_in_flight_{};
    bool services_loading_{};
    bool service_command_in_flight_{};
    bool checksum_in_flight_{};
    std::string service_notice_{};
    bool show_hidden_{};
    bool show_extensions_{true};
    bool checksum_visible_{true};
    bool terminal_visible_{true};
    bool tree_model_syncing_{};
    bool builtin_previews_enabled_{true};
    bool search_showing_{};
    bool search_loading_{};
    bool criteria_showing_{};
    bool criteria_loading_{};
    std::optional<fileman::orchestrator::SearchCursorInfo> search_cursor_{};
    std::optional<fileman::orchestrator::SearchCursorInfo> criteria_cursor_{};
    std::vector<std::string> search_order_{};
    std::uint64_t next_host_request_id_{1};

    NativeForm form_{};
    std::shared_ptr<gui_forms::MenuStrip> menu_strip_{};
    std::vector<gui_forms::MenuStripItemSpec> expanded_menu_items_{};
    bool compact_menu_{};
    bool compact_search_active_{};
    bool adapting_layout_{};
    gui_forms::Size adaptive_viewport_{};
    std::shared_ptr<gui_forms::SplitContainer> workspace_split_{};
    std::shared_ptr<gui_forms::SplitContainer> selection_split_{};
    std::shared_ptr<gui_forms::Panel> content_surface_{};
    std::shared_ptr<gui_forms::PropertyList> settings_property_list_{};
    std::shared_ptr<gui_forms::BreadcrumbTrail> breadcrumb_{};
    std::shared_ptr<gui_forms::TextBox> path_box_{};
    std::shared_ptr<gui_forms::TextBox> search_box_{};
    std::shared_ptr<gui_forms::DropDownButton> tree_root_mode_button_{};
    std::shared_ptr<gui_forms::TreeView> tree_{};
    std::shared_ptr<gui_forms::ObjectView> objects_{};
    std::shared_ptr<gui_forms::Panel> criteria_console_{};
    std::shared_ptr<gui_forms::Label> criteria_title_{};
    std::shared_ptr<gui_forms::InstrumentRack> criteria_rack_{};
    std::shared_ptr<gui_forms::Label> criteria_action_state_{};
    std::shared_ptr<gui_forms::Button> criteria_add_button_{};
    std::shared_ptr<gui_forms::ImageList> object_images_{};
    std::shared_ptr<gui_forms::ImageList> tree_images_{};
    std::shared_ptr<gui_forms::ImageList> preview_images_{};
    std::shared_ptr<gui_forms::CorrespondenceView> correspondence_{};
    std::shared_ptr<gui_forms::PropertyList> property_list_{};
    std::shared_ptr<gui_forms::PictureBox> preview_picture_{};
    std::shared_ptr<gui_forms::Label> preview_text_{};
    // UI-thread session preference; unset preserves automatic height adaptation.
    std::optional<bool> preview_expanded_override_{};
    std::shared_ptr<gui_forms::Button> preview_house_icon_{};
    std::shared_ptr<gui_forms::TextBox> expected_checksum_box_{};
    std::shared_ptr<gui_forms::TextBox> rename_box_{};
    std::shared_ptr<gui_forms::DropDownButton> shelf_move_copy_button_{};
    std::shared_ptr<gui_forms::DropDownButton> shelf_view_button_{};
    std::shared_ptr<gui_forms::DropDownButton> shelf_sort_button_{};
    std::shared_ptr<gui_forms::DropDownButton> shelf_overflow_button_{};
    PathMap breadcrumb_paths_{};
    std::shared_ptr<gui_forms::ContextMenu> breadcrumb_overflow_menu_{};
    std::vector<std::shared_ptr<gui_forms::Command>> breadcrumb_overflow_commands_{};
    std::shared_ptr<gui_forms::AnchoredPopupLayer> path_suggestion_layer_{};
    std::shared_ptr<gui_forms::Panel> path_suggestion_content_{};
    std::shared_ptr<gui_forms::Label> path_resolution_preview_{};
    std::shared_ptr<gui_forms::ListBox> path_suggestion_list_{};
    std::vector<std::filesystem::path> path_suggestion_paths_{};
    gui_forms::PopupToken path_suggestion_popup_{};
    std::vector<std::shared_ptr<gui_forms::Command>> commands_{};
    std::vector<std::shared_ptr<gui_forms::ContextMenu>> menus_{};
    std::shared_ptr<gui_forms::ContextMenu> move_copy_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> view_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> sort_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> shelf_overflow_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> tree_root_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> object_menu_{};
    std::shared_ptr<gui_forms::ContextMenu> background_menu_{};
    std::shared_ptr<gui_forms::Command> command_open_{};
    std::shared_ptr<gui_forms::Command> command_focus_search_{};
    std::shared_ptr<gui_forms::Command> command_focus_location_{};
    std::shared_ptr<gui_forms::Command> command_choose_open_{};
    std::shared_ptr<gui_forms::Command> command_new_folder_{};
    std::shared_ptr<gui_forms::Command> command_copy_{};
    std::shared_ptr<gui_forms::Command> command_move_{};
    std::shared_ptr<gui_forms::Command> command_paste_{};
    std::shared_ptr<gui_forms::Command> command_undo_{};
    std::shared_ptr<gui_forms::Command> command_delete_{};
    std::shared_ptr<gui_forms::Command> command_rename_{};
    std::shared_ptr<gui_forms::Command> command_properties_{};
    std::shared_ptr<gui_forms::Command> command_select_all_{};
    std::shared_ptr<gui_forms::Command> command_back_{};
    std::shared_ptr<gui_forms::Command> command_forward_{};
    std::shared_ptr<gui_forms::Command> command_up_{};
    std::shared_ptr<gui_forms::Command> command_root_{};
    std::shared_ptr<gui_forms::Command> command_tree_home_{};
    std::shared_ptr<gui_forms::Command> command_tree_volumes_{};
    std::vector<std::shared_ptr<gui_forms::Command>> command_tree_admitted_roots_{};
    std::vector<std::filesystem::path> tree_admitted_root_paths_{};
    std::shared_ptr<gui_forms::Command> command_icons_{};
    std::shared_ptr<gui_forms::Command> command_details_{};
    std::shared_ptr<gui_forms::Command> command_criteria_{};
    std::shared_ptr<gui_forms::Command> command_sort_name_{};
    std::shared_ptr<gui_forms::Command> command_sort_kind_{};
    std::shared_ptr<gui_forms::Command> command_sort_size_{};
    std::shared_ptr<gui_forms::Command> command_sort_modified_{};
    std::shared_ptr<gui_forms::Command> command_refresh_{};
    std::shared_ptr<gui_forms::Command> command_toggle_tree_{};
    std::shared_ptr<gui_forms::Command> command_toggle_selection_{};
    std::shared_ptr<gui_forms::Command> command_checksum_{};
    std::shared_ptr<gui_forms::Command> command_terminal_{};
    std::shared_ptr<gui_forms::Command> command_copy_path_{};
    std::shared_ptr<gui_forms::Command> command_settings_{};
    std::shared_ptr<gui_forms::Command> command_close_{};
    std::vector<gui_forms::SubscriptionToken> subscriptions_{};
    std::vector<gui_forms::SubscriptionToken> breadcrumb_subscriptions_{};
    std::vector<gui_forms::SubscriptionToken> path_suggestion_subscriptions_{};
    std::vector<gui_forms::SubscriptionToken> settings_subscriptions_{};
    std::vector<gui_forms::AcceleratorToken> accelerator_tokens_{};
    gui_forms::Window* window_{};
    gui_forms::ImageId preview_image_id_{};

    std::thread worker_{};
    std::mutex worker_mutex_{};
    std::condition_variable worker_cv_{};
    std::queue<std::function<void()>> worker_queue_{};
    std::atomic_bool stopping_{};
    bool details_mode_{};

    std::mutex ui_mutex_{};
    std::queue<std::function<void()>> ui_queue_{};
    std::function<void()> wake_{};
    std::function<void()> request_close_{};
    std::function<void(const std::filesystem::path&)> show_open_picker_{};
    std::function<void()> show_about_window_{};
    std::string sort_mode_{"name"};
    gui_forms::ObjectSortDirection sort_direction_{gui_forms::ObjectSortDirection::ascending};
    std::optional<std::array<double, 4>> automatic_details_widths_{};
    bool details_widths_owned_by_user_{false};
};

} // namespace file_manager
