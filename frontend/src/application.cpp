#include "application.hpp"

#include "fileman_orchestrator/client.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace file_manager {
namespace {

gui_forms::ObjectGlyph object_glyph(const EntryKind kind) {
    switch (kind) {
        case EntryKind::folder: return gui_forms::ObjectGlyph::folder;
        case EntryKind::image: return gui_forms::ObjectGlyph::image;
        case EntryKind::archive: return gui_forms::ObjectGlyph::archive;
        case EntryKind::audio: return gui_forms::ObjectGlyph::audio;
        case EntryKind::code: return gui_forms::ObjectGlyph::code;
        case EntryKind::document:
        case EntryKind::symlink:
        case EntryKind::other:
            return gui_forms::ObjectGlyph::document;
    }
    return gui_forms::ObjectGlyph::document;
}

std::string kind_text(const DirectoryEntry& entry) {
    switch (entry.kind) {
        case EntryKind::folder: return "Folder";
        case EntryKind::document: return "Document";
        case EntryKind::image: return "Image";
        case EntryKind::archive: return "Archive";
        case EntryKind::audio: return "Audio";
        case EntryKind::code: return "Source code";
        case EntryKind::symlink: return "Symbolic link · not followed";
        case EntryKind::other: return "Filesystem object";
    }
    return "Filesystem object";
}

std::string leaf_name(const std::filesystem::path& path) {
    auto name = path.filename().string();
    return name.empty() ? path.string() : name;
}

std::string search_stable_id(const std::filesystem::path& path,
                             const ObjectIdentity& identity) {
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offset;
    for (const unsigned char value : path.generic_string()) {
        hash ^= value;
        hash *= prime;
    }
    std::ostringstream stream;
    stream << "engine-result-" << std::hex << identity.device << '-'
           << identity.inode << '-' << hash;
    return stream.str();
}

EntryKind engine_entry_kind(std::string_view kind,
                            const std::filesystem::path& path) {
    if (kind == "directory") return EntryKind::folder;
    if (kind == "symlink") return EntryKind::symlink;
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](const unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
        extension == ".gif" || extension == ".webp") return EntryKind::image;
    if (extension == ".zip" || extension == ".tar" || extension == ".gz" ||
        extension == ".7z") return EntryKind::archive;
    if (extension == ".mp3" || extension == ".wav" || extension == ".flac") {
        return EntryKind::audio;
    }
    if (extension == ".cpp" || extension == ".hpp" || extension == ".c" ||
        extension == ".h" || extension == ".go" || extension == ".rs" ||
        extension == ".py" || extension == ".html" || extension == ".css") {
        return EntryKind::code;
    }
    return kind == "file" ? EntryKind::document : EntryKind::other;
}

std::string setting_title(std::string_view id) {
    const auto separator = id.find_last_of('.');
    std::string title(id.substr(separator == std::string_view::npos
                                    ? 0
                                    : separator + 1));
    bool capitalize = true;
    for (char& character : title) {
        if (character == '_') {
            character = ' ';
            capitalize = true;
        } else if (capitalize) {
            character = static_cast<char>(
                std::toupper(static_cast<unsigned char>(character)));
            capitalize = false;
        }
    }
    return title;
}

std::string settings_tab_description(std::string_view tab) {
    if (tab == "services") {
        return "Lifecycle and availability remain commands and facts; values use typed transactions.";
    }
    if (tab == "search_indexing") {
        return "Search limits and fallback preferences do not widen Engine root authority.";
    }
    if (tab == "previews_extensions" || tab == "privacy_data") {
        return "Unavailable and stubbed providers remain visible but cannot be enabled.";
    }
    return "Typed values use the same optimistic transaction as the CLI.";
}

std::string setting_value_text(
    const fileman::orchestrator::SettingValue& value) {
    if (const auto* boolean = std::get_if<bool>(&value)) {
        return *boolean ? "True" : "False";
    }
    if (const auto* number = std::get_if<std::uint64_t>(&value)) {
        return std::to_string(*number);
    }
    return std::get<std::string>(value);
}

template <typename T>
std::optional<T> setting_as(
    const fileman::orchestrator::SettingsSnapshotInfo& snapshot,
    std::string_view id) {
    const auto* value = snapshot.find(id);
    if (!value) return std::nullopt;
    if (const auto* typed = std::get_if<T>(value)) return *typed;
    return std::nullopt;
}

} // namespace

Application::Application(std::filesystem::path protected_root,
                         std::optional<std::filesystem::path> quarantine_root,
                         const bool mutations_enabled,
                         std::string engine_root_id)
    : protected_root_(canonical_existing_directory(protected_root)),
      location_(protected_root_),
      engine_root_id_(std::move(engine_root_id)),
      form_(web_forms_generated_file_manager_sapphire::make_native_form()) {
    if (engine_root_id_.empty()) {
        throw std::invalid_argument("--engine-root-id must be nonempty");
    }
    if (mutations_enabled) {
        if (!quarantine_root) {
            throw std::invalid_argument(
                "--allow-mutations requires --quarantine DIRECTORY");
        }
        operations_ = std::make_unique<FileOperationService>(
            protected_root_, std::move(*quarantine_root), true);
    }
    install_dynamic_controls();
    install_handlers();
    worker_ = std::thread([this] { worker_loop(); });
}

Application::~Application() {
    stop();
}

std::unique_ptr<gui_forms::Window> Application::make_window() {
    auto window = std::make_unique<gui_forms::Window>(
        form_.root_control(), gui_forms::Size{1340, 850});
    window_ = window.get();
    return window;
}

void Application::install_dynamic_controls() {
    menu_strip_ = std::make_shared<gui_forms::MenuStrip>(
        gui_forms::StableId("fm.application.menu"));
    menu_strip_->set_accessible_name("File Manager application menu");
    menu_strip_->set_requested_bounds({0, 0, 720, 24});
    form_.file_manager_app_shell_menu->clear_children();
    form_.file_manager_app_shell_menu->add_child(menu_strip_);
    form_.file_manager_app_shell_menu->set_flex_grow(*menu_strip_, 1.0);

    path_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.path.editor"), protected_root_.string());
    path_box_->set_requested_bounds({0, 0, 620, 26});
    path_box_->set_placeholder_text("Enter a path inside this launch scope");
    path_box_->set_accessible_name("Current location path");
    path_box_->set_visible(false);
    breadcrumb_ = std::make_shared<gui_forms::FlowLayoutPanel>(
        gui_forms::StableId("fm.path.breadcrumb"));
    breadcrumb_->set_requested_bounds({0, 0, 620, 26});
    breadcrumb_->set_wrap_contents(false);
    breadcrumb_->set_cross_alignment(gui_forms::FlowCrossAlignment::stretch);
    breadcrumb_->set_accessible_name("Current location breadcrumb");
    form_.file_manager_app_shell_location_path_host->clear_children();
    form_.file_manager_app_shell_location_path_host->add_child(breadcrumb_);
    form_.file_manager_app_shell_location_path_host->add_child(path_box_);
    form_.file_manager_app_shell_location_path_host->set_flex_grow(*breadcrumb_, 1.0);
    form_.file_manager_app_shell_location_path_host->set_flex_grow(*path_box_, 1.0);

    search_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.search.current-folder"));
    search_box_->set_requested_bounds({0, 0, 228, 30});
    search_box_->set_placeholder_text("Search this subtree");
    search_box_->set_accessible_name("Search current subtree with Engine");
    form_.file_manager_app_shell_location_search_host->clear_children();
    form_.file_manager_app_shell_location_search_host->add_child(search_box_);
    form_.file_manager_app_shell_location_search_host->set_flex_grow(*search_box_, 1.0);

    tree_ = std::make_shared<gui_forms::TreeView>(
        gui_forms::StableId("fm.navigation.tree"));
    tree_->set_requested_bounds({0, 0, 178, 340});
    tree_->set_accessible_name("Folders in protected root");
    form_.file_manager_app_shell_workspace_sidebar_tree_host->clear_children();
    form_.file_manager_app_shell_workspace_sidebar_tree_host->add_child(tree_);
    form_.file_manager_app_shell_workspace_sidebar_tree_host->set_flex_grow(*tree_, 1.0);

    objects_ = std::make_shared<gui_forms::ObjectView>(
        gui_forms::StableId("fm.objects.current-folder"));
    objects_->set_requested_bounds({0, 0, 736, 455});
    objects_->set_view_mode(gui_forms::ObjectViewMode::icons);
    objects_->set_icon_cell_size({104, 78});
    objects_->set_selection_mode(gui_forms::ObjectSelectionMode::multiple);
    objects_->set_accessible_name("Objects in current folder");

    correspondence_ = std::make_shared<gui_forms::CorrespondenceView>(
        gui_forms::StableId("fm.search.correspondence"));
    correspondence_->set_compact_height(45.0);
    correspondence_->set_expanded_height(116.0);
    correspondence_->set_accessible_name("Factual local search results");
    correspondence_->set_visible(false);

    content_surface_ = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("fm.content.surface"));
    content_surface_->set_background(gui_forms::Color::rgba(255, 255, 255));
    content_surface_->set_dock(gui_forms::DockStyle::fill);
    objects_->set_dock(gui_forms::DockStyle::fill);
    correspondence_->set_dock(gui_forms::DockStyle::fill);
    content_surface_->add_child(objects_);
    content_surface_->add_child(correspondence_);
    form_.file_manager_app_shell_workspace_content_objects->clear_children();
    form_.file_manager_app_shell_workspace_content_objects->add_child(content_surface_);
    form_.file_manager_app_shell_workspace_content_objects->set_flex_grow(
        *content_surface_, 1.0);

    preview_picture_ = std::make_shared<gui_forms::PictureBox>(
        gui_forms::StableId("fm.inspector.preview.image"));
    preview_picture_->set_requested_bounds({0, 0, 194, 112});
    preview_picture_->set_size_mode(gui_forms::PictureBoxSizeMode::zoom);
    preview_picture_->set_accessible_name("Selected image preview");
    preview_picture_->set_visible(false);
    preview_text_ = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("fm.inspector.preview.text"));
    preview_text_->set_requested_bounds({0, 0, 190, 108});
    preview_text_->set_text_style_role(gui_forms::TextStyleRole::monospace);
    preview_text_->set_text_wrapping(gui_forms::TextWrapping::word);
    preview_text_->set_maximum_lines(7);
    preview_text_->set_vertical_alignment(gui_forms::VerticalAlignment::near);
    preview_text_->set_accessible_name("Selected text preview");
    preview_text_->set_visible(false);
    form_.file_manager_app_shell_workspace_inspector_preview_surface->clear_children();
    form_.file_manager_app_shell_workspace_inspector_preview_surface->add_child(
        preview_picture_);
    form_.file_manager_app_shell_workspace_inspector_preview_surface->add_child(
        preview_text_);
    form_.file_manager_app_shell_workspace_inspector_preview_surface->add_child(
        form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph);
    form_.file_manager_app_shell_workspace_inspector_preview->set_minimum_size(
        {0, 224});
    form_.file_manager_app_shell_workspace_inspector_preview_surface->set_minimum_size(
        {0, 174});

    expected_checksum_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.checksum.expected"));
    expected_checksum_box_->set_requested_bounds({0, 0, 190, 28});
    expected_checksum_box_->set_maximum_length(64);
    expected_checksum_box_->set_placeholder_text("Expected SHA-256 (optional)");
    expected_checksum_box_->set_accessible_name("Expected SHA-256 digest");
    expected_checksum_box_->set_accessible_description(
        "Optional exact 64-character hexadecimal SHA-256 value");
    form_.file_manager_app_shell_workspace_inspector_commands_expected_host
        ->clear_children();
    form_.file_manager_app_shell_workspace_inspector_commands_expected_host
        ->add_child(expected_checksum_box_);
    form_.file_manager_app_shell_workspace_inspector_commands_expected_host
        ->set_flex_grow(*expected_checksum_box_, 1.0);

    rename_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.operations.rename"));
    rename_box_->set_requested_bounds({0, 0, 200, 30});
    rename_box_->set_maximum_length(255);
    rename_box_->set_placeholder_text("New name");
    rename_box_->set_accessible_name("Rename selected object");
    rename_box_->set_visible(false);
    form_.file_manager_app_shell_location_path_host->add_child(rename_box_);
    form_.file_manager_app_shell_location_path_host->set_flex_grow(
        *rename_box_, 1.0);

    property_list_ = std::make_shared<gui_forms::PropertyList>(
        gui_forms::StableId("fm.selection.properties"));
    property_list_->set_accessible_name("Selection properties");
    property_list_->set_label_width(72.0);
    property_list_->set_requested_bounds({0, 0, 270, 170});
    property_list_->set_groups({
        {"fm.property.group.identity", "IDENTITY", {
            {"fm.property.kind", "Kind", "—", "Selected object kind"},
            {"fm.property.location", "Location", "—", "Exact local path"},
            {"fm.property.size", "Size", "—", "Observed object size"},
            {"fm.property.modified", "Modified", "—", "Filesystem modification time"},
        }},
    });
    form_.file_manager_app_shell_workspace_inspector_facts->clear_children();
    inspector_surface_ = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("fm.selection.surface"));
    inspector_surface_->set_background(gui_forms::Color::rgba(235, 240, 246));
    inspector_surface_->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace_inspector_label->set_dock(
        gui_forms::DockStyle::top);
    form_.file_manager_app_shell_workspace_inspector_label->set_minimum_size({0, 27});
    form_.file_manager_app_shell_workspace_inspector_preview->set_dock(
        gui_forms::DockStyle::top);
    form_.file_manager_app_shell_workspace_inspector_preview->set_requested_bounds(
        {0, 0, 288, 224});
    property_list_->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace_inspector->clear_children();
    inspector_surface_->add_child(
        form_.file_manager_app_shell_workspace_inspector_label);
    inspector_surface_->add_child(
        form_.file_manager_app_shell_workspace_inspector_preview);
    inspector_surface_->add_child(property_list_);
    form_.file_manager_app_shell_workspace_inspector->add_child(inspector_surface_);
    form_.file_manager_app_shell_workspace_inspector->set_flex_grow(
        *inspector_surface_, 1.0);

    workspace_split_ = std::make_shared<gui_forms::SplitContainer>(
        gui_forms::StableId("fm.workspace.folder-tree"));
    workspace_split_->initialize_control_tree();
    workspace_split_->set_splitter_width(3.0);
    workspace_split_->set_splitter_hit_width(9.0);
    workspace_split_->set_splitter_distance(218.0);
    workspace_split_->set_collapse_panel(gui_forms::SplitFixedPanel::first);
    workspace_split_->set_automatic_collapse_threshold(700.0);
    workspace_split_->set_first_minimum(150.0);
    workspace_split_->set_first_maximum(360.0);
    workspace_split_->set_second_minimum(320.0);

    selection_split_ = std::make_shared<gui_forms::SplitContainer>(
        gui_forms::StableId("fm.workspace.selection"));
    selection_split_->initialize_control_tree();
    selection_split_->set_splitter_width(3.0);
    selection_split_->set_splitter_hit_width(9.0);
    selection_split_->set_splitter_distance(790.0);
    selection_split_->set_fixed_panel(gui_forms::SplitFixedPanel::second);
    selection_split_->set_collapse_panel(gui_forms::SplitFixedPanel::second);
    selection_split_->set_automatic_collapse_threshold(900.0);
    selection_split_->set_first_minimum(260.0);
    selection_split_->set_second_minimum(248.0);
    selection_split_->set_second_maximum(420.0);

    form_.file_manager_app_shell_workspace_sidebar->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace_content->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace_inspector->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace->clear_children();
    workspace_split_->first_panel()->add_child(
        form_.file_manager_app_shell_workspace_sidebar);
    selection_split_->first_panel()->add_child(
        form_.file_manager_app_shell_workspace_content);
    selection_split_->second_panel()->add_child(
        form_.file_manager_app_shell_workspace_inspector);
    selection_split_->set_dock(gui_forms::DockStyle::fill);
    workspace_split_->second_panel()->add_child(selection_split_);
    workspace_split_->set_dock(gui_forms::DockStyle::fill);
    form_.file_manager_app_shell_workspace->add_child(workspace_split_);
    form_.file_manager_app_shell_workspace->set_cell_position(
        *workspace_split_, {0U, 0U});
    form_.file_manager_app_shell_workspace->set_column_span(
        *workspace_split_, 5U);

    form_.file_manager_app_shell_commands_new_folder->set_enabled(
        operations_ != nullptr);
    form_.file_manager_app_shell_commands_rename->set_enabled(false);
    form_.file_manager_app_shell_commands_selection_group_actions_copy->set_enabled(false);
    form_.file_manager_app_shell_commands_move->set_enabled(false);
    form_.file_manager_app_shell_commands_paste->set_enabled(false);
    form_.file_manager_app_shell_commands_selection_group_actions_delete->set_enabled(false);
    form_.file_manager_app_shell_workspace_content_heading_more_results->set_enabled(false);
    form_.file_manager_app_shell_workspace_inspector_commands_open->set_enabled(false);
    form_.file_manager_app_shell_workspace_inspector_commands_checksum->set_enabled(false);
    form_.file_manager_app_shell_settings->set_visible(false);
    form_.file_manager_app_shell_settings_actions_apply->set_enabled(false);
    form_.file_manager_app_shell_settings_actions_cancel->set_enabled(false);
    form_.file_manager_app_shell_settings_actions_reset->set_enabled(false);
    form_.file_manager_app_shell_settings_body_page_host->set_minimum_size({0, 240});
    form_.file_manager_app_shell_settings_body_page_host->set_requested_bounds(
        {0, 0, 760, 330});
    form_.file_manager_app_shell_settings_actions->set_wrap_contents(false);
    form_.file_manager_app_shell_settings_actions_status->set_minimum_size({160, 27});
    auto settings_back =
        form_.file_manager_app_shell_settings_actions->remove_child(
            form_.file_manager_app_shell_settings_actions_back->runtime_id());
    if (!settings_back) {
        throw std::logic_error("Settings Back command is absent from its source host");
    }
    form_.file_manager_app_shell_settings_actions_back->set_requested_bounds(
        {0, 0, 100, 27});
    form_.file_manager_app_shell_settings_heading->add_child(settings_back);
    for (const auto& legacy : {
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_home),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_new_folder),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_rename),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_move),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_paste),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_commands_rule),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_location_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_sidebar_root),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_sidebar_parent),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_sidebar_tree_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_inspector_facts_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_inspector_commands_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_inspector_commands)}) {
        legacy->set_visible(false);
    }
    form_.file_manager_app_shell_commands_arrange_group_actions_settings->set_accessible_description(
        "Open the File Manager-owned tabbed surface for Orchestrator settings and service facts");
    const std::string mutation_description = operations_
        ? "Available only inside the explicit protected mutation profile"
        : "Read-only launch; use an explicit protected mutation profile";
    form_.file_manager_app_shell_commands_new_folder->set_accessible_description(
        mutation_description);
    form_.file_manager_app_shell_commands_rename->set_accessible_description(
        mutation_description);
    form_.file_manager_app_shell_commands_selection_group_actions_copy->set_accessible_description(
        operations_ ? "Capture one selected object for collision-safe staged copy"
                    : mutation_description);
    form_.file_manager_app_shell_commands_move->set_accessible_description(
        operations_ ? "Capture one selected object for same-volume move"
                    : mutation_description);
    form_.file_manager_app_shell_commands_paste->set_accessible_description(
        operations_ ? "Publish the captured transfer in the current folder"
                    : mutation_description);
    form_.file_manager_app_shell_commands_selection_group_actions_delete->set_accessible_description(
        operations_ ? "Two-step recoverable quarantine; never permanent deletion"
                    : mutation_description);

    form_.file_manager_app_shell_workspace_content_heading->set_visible(false);
    rebuild_breadcrumb();
    install_command_surfaces();
}

std::shared_ptr<gui_forms::Command> Application::make_command(
    std::string id, std::string text, std::string description,
    std::function<void()> action) {
    auto command = std::make_shared<gui_forms::Command>(
        std::move(id), std::move(text));
    command->set_description(std::move(description));
    subscriptions_.push_back(command->invoked().subscribe(
        [action = std::move(action)](const gui_forms::CommandInvocation&) {
            action();
        }));
    commands_.push_back(command);
    return command;
}

void Application::show_menu(
    const std::shared_ptr<gui_forms::ContextMenu>& menu,
    const gui_forms::Control::Ptr& owner) {
    if (!menu || !owner) return;
    const auto bounds = owner->rectangle_to_window(owner->client_rectangle());
    menu->show(owner, {bounds.x, bounds.y + bounds.height});
}

void Application::install_command_surfaces() {
    using gui_forms::MenuItemKind;

    command_open_ = make_command(
        "file.open", "Open", "Open the selected local object",
        [this] { request_open(); });
    command_open_->set_shortcut("Enter");
    command_open_->set_default_action(true);
    command_new_folder_ = make_command(
        "file.new-folder", "New folder",
        "Create a collision-safe folder in the current location",
        [this] { request_create_folder(); });
    command_copy_ = make_command(
        "selection.copy", "Copy",
        "Stage the selected object for a collision-safe copy",
        [this] { capture_transfer(false); });
    command_move_ = make_command(
        "selection.move", "Move",
        "Stage the selected object for a same-volume move",
        [this] { capture_transfer(true); });
    command_paste_ = make_command(
        "selection.paste", "Paste",
        "Publish the staged transfer in the current folder",
        [this] { paste_transfer(); });
    command_paste_->set_shortcut("Cmd+V");
    command_undo_ = make_command(
        "edit.undo", "Undo last operation",
        "Undo the last recoverable protected operation",
        [this] { request_undo(); });
    command_delete_ = make_command(
        "selection.delete", "Delete",
        "Move the selected object to the configured recovery quarantine",
        [this] { request_quarantine(); });
    command_delete_->set_shortcut("Delete");
    command_delete_->set_destructive(true);
    command_rename_ = make_command(
        "selection.rename", "Rename",
        "Rename the selected object after no-follow identity validation",
        [this] { begin_rename(); });
    command_rename_->set_shortcut("F2");
    command_properties_ = make_command(
        "view.properties", "Properties",
        "Reveal and focus the factual Selection and Properties pane",
        [this] { show_properties(); });
    command_select_all_ = make_command(
        "edit.select-all", "Select all", "Select all visible objects",
        [this] {
            if (search_showing_) {
                set_status("Search selection is single-row",
                           "correspondence rows retain one focused factual result");
            } else {
                objects_->select_all();
            }
        });
    command_select_all_->set_shortcut("Cmd+A");

    command_back_ = make_command(
        "go.back", "Back", "Return to the previous local location",
        [this] { navigate_back(); });
    command_back_->set_shortcut("Alt+Left");
    command_forward_ = make_command(
        "go.forward", "Forward", "Advance to the next local location",
        [this] { navigate_forward(); });
    command_forward_->set_shortcut("Alt+Right");
    command_up_ = make_command(
        "go.up", "Up", "Navigate to the parent inside this launch scope",
        [this] { navigate_up(); });
    command_up_->set_shortcut("Alt+Up");
    command_root_ = make_command(
        "go.scope-root", "Scope root", "Return to the launch scope root",
        [this] { request_navigation(protected_root_, true); });

    command_icons_ = make_command(
        "view.small-icons", "Small icons", "Use the compact icon object field",
        [this] { set_view_mode(gui_forms::ObjectViewMode::icons); });
    command_details_ = make_command(
        "view.details", "Details", "Use the factual details object field",
        [this] { set_view_mode(gui_forms::ObjectViewMode::details); });
    command_sort_name_ = make_command(
        "sort.name", "Name", "Sort objects by name",
        [this] { set_sort_mode("name"); });
    command_sort_kind_ = make_command(
        "sort.kind", "Kind", "Sort objects by observed kind",
        [this] { set_sort_mode("kind"); });
    command_sort_size_ = make_command(
        "sort.size", "Size", "Sort objects by observed byte size",
        [this] { set_sort_mode("size"); });
    command_sort_modified_ = make_command(
        "sort.modified", "Modified", "Sort objects by observed modification time",
        [this] { set_sort_mode("modified"); });
    command_refresh_ = make_command(
        "view.refresh", "Refresh", "Read the current local folder again",
        [this] { request_navigation(location_, false); });
    command_toggle_tree_ = make_command(
        "view.folder-tree", "Folder tree", "Show or collapse the folder tree",
        [this] { toggle_folder_tree(); });
    command_toggle_selection_ = make_command(
        "view.selection-pane", "Selection and Properties",
        "Show or collapse the factual selection pane",
        [this] { toggle_selection_pane(); });

    command_checksum_ = make_command(
        "commands.sha256", "SHA-256…",
        "Compute a revision-validated SHA-256 digest for the selected file",
        [this] { request_checksum(); });
    command_terminal_ = make_command(
        "commands.terminal", "Terminal here",
        "Open Terminal at the selected folder or current location",
        [this] { request_terminal(); });
    command_copy_path_ = make_command(
        "commands.copy-path", "Copy path",
        "Copy the exact selected or current local path",
        [this] { copy_current_path(); });
    command_settings_ = make_command(
        "file.settings", "Settings…",
        "Open File Manager settings and Orchestrator service controls",
        [this] {
            if (settings_open_) hide_settings();
            else show_settings();
        });
    command_close_ = make_command(
        "file.close", "Close File Manager", "Close this File Manager window",
        [this] {
            if (request_close_) request_close_();
        });
    auto about = make_command(
        "help.about", "About File Manager", "Show build and authority information",
        [this] { show_about(); });

    menu_strip_->set_items({
        {"fm.menu.file", "File", {
            {"file.open", MenuItemKind::command, command_open_},
            {"file.new-folder", MenuItemKind::command, command_new_folder_},
            {"file.separator.settings", MenuItemKind::separator},
            {"file.settings", MenuItemKind::command, command_settings_},
            {"file.separator.close", MenuItemKind::separator},
            {"file.close", MenuItemKind::command, command_close_},
        }},
        {"fm.menu.home", "Home", {
            {"home.open", MenuItemKind::command, command_open_},
            {"home.move-copy", MenuItemKind::submenu, {}, "Move / copy", {
                {"home.copy", MenuItemKind::command, command_copy_},
                {"home.move", MenuItemKind::command, command_move_},
                {"home.paste", MenuItemKind::command, command_paste_},
            }},
            {"home.separator.delete", MenuItemKind::separator},
            {"home.delete", MenuItemKind::command, command_delete_},
            {"home.properties", MenuItemKind::command, command_properties_},
        }},
        {"fm.menu.edit", "Edit", {
            {"edit.select-all", MenuItemKind::command, command_select_all_},
            {"edit.separator.rename", MenuItemKind::separator},
            {"edit.rename", MenuItemKind::command, command_rename_},
            {"edit.copy", MenuItemKind::command, command_copy_},
            {"edit.move", MenuItemKind::command, command_move_},
            {"edit.paste", MenuItemKind::command, command_paste_},
            {"edit.delete", MenuItemKind::command, command_delete_},
            {"edit.separator.undo", MenuItemKind::separator},
            {"edit.undo", MenuItemKind::command, command_undo_},
        }},
        {"fm.menu.view", "View", {
            {"view.icons", MenuItemKind::radio, command_icons_},
            {"view.details", MenuItemKind::radio, command_details_},
            {"view.sort", MenuItemKind::submenu, {}, "Sort", {
                {"view.sort.name", MenuItemKind::radio, command_sort_name_},
                {"view.sort.kind", MenuItemKind::radio, command_sort_kind_},
                {"view.sort.size", MenuItemKind::radio, command_sort_size_},
                {"view.sort.modified", MenuItemKind::radio, command_sort_modified_},
            }},
            {"view.refresh", MenuItemKind::command, command_refresh_},
            {"view.separator.panes", MenuItemKind::separator},
            {"view.folder-tree", MenuItemKind::check, command_toggle_tree_},
            {"view.selection-pane", MenuItemKind::check, command_toggle_selection_},
        }},
        {"fm.menu.go", "Go", {
            {"go.back", MenuItemKind::command, command_back_},
            {"go.forward", MenuItemKind::command, command_forward_},
            {"go.up", MenuItemKind::command, command_up_},
            {"go.scope-root", MenuItemKind::command, command_root_},
        }},
        {"fm.menu.commands", "Commands", {
            {"commands.sha256", MenuItemKind::command, command_checksum_},
            {"commands.terminal", MenuItemKind::command, command_terminal_},
            {"commands.copy-path", MenuItemKind::command, command_copy_path_},
        }},
        {"fm.menu.help", "Help", {
            {"help.about", MenuItemKind::command, about},
        }},
    });

    move_copy_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.move-copy");
    move_copy_menu_->set_items({
        {"copy", MenuItemKind::command, command_copy_},
        {"move", MenuItemKind::command, command_move_},
        {"paste", MenuItemKind::command, command_paste_},
    });
    view_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.view");
    view_menu_->set_items({
        {"small-icons", MenuItemKind::radio, command_icons_},
        {"details", MenuItemKind::radio, command_details_},
    });
    sort_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.sort");
    sort_menu_->set_items({
        {"name", MenuItemKind::radio, command_sort_name_},
        {"kind", MenuItemKind::radio, command_sort_kind_},
        {"size", MenuItemKind::radio, command_sort_size_},
        {"modified", MenuItemKind::radio, command_sort_modified_},
    });
    object_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.context.object");
    object_menu_->set_items({
        {"open", MenuItemKind::command, command_open_},
        {"separator.transfer", MenuItemKind::separator},
        {"copy", MenuItemKind::command, command_copy_},
        {"move", MenuItemKind::command, command_move_},
        {"rename", MenuItemKind::command, command_rename_},
        {"delete", MenuItemKind::command, command_delete_},
        {"separator.commands", MenuItemKind::separator},
        {"sha256", MenuItemKind::command, command_checksum_},
        {"terminal", MenuItemKind::command, command_terminal_},
        {"copy-path", MenuItemKind::command, command_copy_path_},
        {"separator.properties", MenuItemKind::separator},
        {"properties", MenuItemKind::command, command_properties_},
    });
    background_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.context.background");
    background_menu_->set_items({
        {"new-folder", MenuItemKind::command, command_new_folder_},
        {"paste", MenuItemKind::command, command_paste_},
        {"separator.commands", MenuItemKind::separator},
        {"terminal", MenuItemKind::command, command_terminal_},
        {"copy-path", MenuItemKind::command, command_copy_path_},
        {"separator.properties", MenuItemKind::separator},
        {"properties", MenuItemKind::command, command_properties_},
    });
    menus_ = {move_copy_menu_, view_menu_, sort_menu_, object_menu_, background_menu_};
    update_command_state();
}

void Application::rebuild_breadcrumb() {
    breadcrumb_subscriptions_.clear();
    breadcrumb_buttons_.clear();
    breadcrumb_separators_.clear();
    breadcrumb_->clear_children();

    std::vector<std::pair<std::string, std::filesystem::path>> segments;
    std::string root_label = leaf_name(protected_root_);
    if (const char* home = std::getenv("HOME"); home && *home) {
        if (std::filesystem::path(home).lexically_normal() == protected_root_) {
            root_label = "Home";
        }
    }
    segments.emplace_back(root_label, protected_root_);
    const auto relative = location_.lexically_relative(protected_root_);
    auto accumulated = protected_root_;
    if (!relative.empty() && relative != ".") {
        for (const auto& component : relative) {
            accumulated /= component;
            segments.emplace_back(component.string(), accumulated);
        }
    }

    for (std::size_t index = 0; index < segments.size(); ++index) {
        const auto& [text, path] = segments[index];
        auto button = std::make_shared<gui_forms::Button>(
            gui_forms::StableId("fm.path.segment." + std::to_string(index)), text);
        button->set_visual_style(gui_forms::ButtonVisualStyle::flat);
        button->set_requested_bounds(
            {0, 0, std::max(48.0, 15.0 + 6.6 * static_cast<double>(text.size())), 26});
        button->set_accessible_description("Navigate to " + path.string());
        breadcrumb_subscriptions_.push_back(button->clicked().subscribe(
            [this, path](gui_forms::ButtonBase&) {
                request_navigation(path, true);
            }));
        breadcrumb_->add_child(button);
        breadcrumb_buttons_.push_back(std::move(button));
        if (index + 1U < segments.size()) {
            auto separator = std::make_shared<gui_forms::Label>(
                gui_forms::StableId("fm.path.separator." + std::to_string(index)), "›");
            separator->set_requested_bounds({0, 0, 14, 26});
            separator->set_alignment(gui_forms::HorizontalAlignment::center);
            separator->set_vertical_alignment(gui_forms::VerticalAlignment::center);
            breadcrumb_->add_child(separator);
            breadcrumb_separators_.push_back(std::move(separator));
        }
    }
    path_edit_button_ = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("fm.path.edit"), "./");
    path_edit_button_->set_visual_style(gui_forms::ButtonVisualStyle::flat);
    path_edit_button_->set_requested_bounds({0, 0, 34, 26});
    path_edit_button_->set_accessible_name("Edit complete path");
    path_edit_button_->set_accessible_description(
        "Switch this breadcrumb to one inline exact-path editor");
    breadcrumb_subscriptions_.push_back(path_edit_button_->clicked().subscribe(
        [this](gui_forms::ButtonBase&) { set_path_editing(true); }));
    breadcrumb_->add_child(path_edit_button_);
    set_path_editing(false);
}

void Application::set_path_editing(const bool editing) {
    if (rename_box_) rename_box_->set_visible(false);
    breadcrumb_->set_visible(!editing);
    path_box_->set_visible(editing);
    if (!editing) return;
    path_box_->set_text(location_.string());
    path_box_->select_all();
    if (window_) window_->request_focus(path_box_);
}

void Application::set_view_mode(const gui_forms::ObjectViewMode mode) {
    details_mode_ = mode == gui_forms::ObjectViewMode::details;
    objects_->set_view_mode(mode);
    form_.file_manager_app_shell_commands_arrange_group_actions_details->set_text(
        details_mode_ ? "▤ View: details ▾" : "▦ View: small icons ▾");
    update_command_state();
    set_status(details_mode_ ? "Details view" : "Small icons view",
               "current local folder · retained view state");
}

void Application::rebuild_object_order() {
    const std::vector<std::string> selected(
        objects_->selected_ids().begin(), objects_->selected_ids().end());
    std::vector<gui_forms::ObjectViewItem> items(
        objects_->items().begin(), objects_->items().end());
    const auto folded_name = [](const std::string& value) {
        std::string result(value);
        std::transform(result.begin(), result.end(), result.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
        return result;
    };
    std::stable_sort(items.begin(), items.end(), [&](const auto& left,
                                                     const auto& right) {
        const auto left_entry = entries_.find(left.stable_id);
        const auto right_entry = entries_.find(right.stable_id);
        if (left_entry == entries_.end() || right_entry == entries_.end()) {
            return folded_name(left.name) < folded_name(right.name);
        }
        const auto& a = left_entry->second;
        const auto& b = right_entry->second;
        if (a.directory != b.directory) return a.directory > b.directory;
        if (sort_mode_ == "kind" && a.kind != b.kind) return a.kind < b.kind;
        if (sort_mode_ == "size" && a.identity.size != b.identity.size) {
            return a.identity.size < b.identity.size;
        }
        if (sort_mode_ == "modified" &&
            a.identity.modified_nanoseconds != b.identity.modified_nanoseconds) {
            return a.identity.modified_nanoseconds > b.identity.modified_nanoseconds;
        }
        return folded_name(a.name) < folded_name(b.name);
    });
    objects_->set_items(std::move(items));
    if (!selected.empty()) objects_->set_selected_ids(selected);
}

void Application::set_sort_mode(std::string mode) {
    sort_mode_ = std::move(mode);
    rebuild_object_order();
    auto title = sort_mode_;
    title.front() = static_cast<char>(
        std::toupper(static_cast<unsigned char>(title.front())));
    form_.file_manager_app_shell_commands_arrange_group_actions_refresh->set_text(
        "≡ Sort: " + title + " ▾");
    update_command_state();
    set_status("Sorted by " + title,
               "folders first · exact observed fields · stable name fallback");
}

void Application::show_properties() {
    selection_split_->set_second_collapsed(
        false, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    if (window_) window_->request_focus(property_list_);
    set_status("Selection and Properties active",
               "factual local projection · no inferred metadata");
}

void Application::toggle_folder_tree() {
    const bool show = workspace_split_->first_collapsed();
    workspace_split_->set_first_collapsed(
        !show, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    set_status(show ? "Folder tree restored" : "Folder tree collapsed",
               "thin splitter seam remains available");
}

void Application::toggle_selection_pane() {
    const bool show = selection_split_->second_collapsed();
    selection_split_->set_second_collapsed(
        !show, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    set_status(show ? "Selection and Properties restored"
                    : "Selection and Properties collapsed",
               "thin splitter seam remains available");
}

void Application::show_about() {
    const std::string version = FILE_MANAGER_VERSION;
    set_status("File Manager · " + version + " protected-root build",
               "local filesystem authority · GUI.Forms + Web.Forms");
    if (!window_ || !window_->host_services()) return;
    gui_forms::HostMessageDialogRequest message;
    message.title = "About File Manager";
    message.message =
        "File Manager " + version + "\n\n"
        "Protected-root M4 build.\n"
        "Local-machine file navigation and protected operations.\n"
        "Frontend: Web.Forms source compiled to retained GUI.Forms C++.\n"
        "Search and settings cross the admitted Orchestrator contracts.";
    message.buttons = gui_forms::HostMessageButtons::ok;
    message.icon = gui_forms::HostMessageIcon::information;
    gui_forms::HostDialogRequest request;
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.about";
    request.payload = std::move(message);
    (void)window_->host_services()->show_dialog(request);
}

void Application::update_command_state() {
    if (!command_open_) return;
    const bool one = objects_->selected_ids().size() == 1U;
    const auto entry = selected_entry();
    const bool mutation_ready = operations_ && !rename_box_->visible() &&
        !transfer_in_flight_;
    const bool paste_ready = mutation_ready && pending_transfer_ &&
        pending_transfer_->entry.path.parent_path() != location_;
    const std::string mutation_reason = operations_
        ? "Select one object and wait for the active operation"
        : "This launch is read-only; protected mutations were not enabled";

    command_open_->set_enabled(entry && entry->kind != EntryKind::symlink);
    command_new_folder_->set_enabled(mutation_ready);
    command_copy_->set_enabled(mutation_ready && one);
    command_move_->set_enabled(mutation_ready && one);
    command_paste_->set_enabled(paste_ready);
    command_undo_->set_enabled(operations_ && operations_->undo_available());
    command_delete_->set_enabled(mutation_ready && one);
    command_rename_->set_enabled(mutation_ready && one);
    for (const auto& command : {command_new_folder_, command_copy_, command_move_,
                                command_paste_, command_undo_, command_delete_,
                                command_rename_}) {
        if (command && !command->state().enabled) {
            command->set_availability_reason(mutation_reason);
        }
    }
    command_back_->set_enabled(!history_.empty() && history_index_ > 0U);
    command_forward_->set_enabled(
        !history_.empty() && history_index_ + 1U < history_.size());
    command_up_->set_enabled(location_ != protected_root_);
    command_root_->set_enabled(location_ != protected_root_);
    command_icons_->set_checked(!details_mode_);
    command_details_->set_checked(details_mode_);
    command_sort_name_->set_checked(sort_mode_ == "name");
    command_sort_kind_->set_checked(sort_mode_ == "kind");
    command_sort_size_->set_checked(sort_mode_ == "size");
    command_sort_modified_->set_checked(sort_mode_ == "modified");
    command_toggle_tree_->set_checked(!workspace_split_->first_collapsed());
    command_toggle_selection_->set_checked(!selection_split_->second_collapsed());
    command_checksum_->set_visible(checksum_visible_);
    command_checksum_->set_enabled(checksum_in_flight_ ||
        (entry && !entry->directory && entry->kind != EntryKind::symlink));
    command_checksum_->set_text(checksum_in_flight_ ? "Cancel SHA-256" : "SHA-256…");
    command_terminal_->set_visible(terminal_visible_);
    command_terminal_->set_enabled(!entry || entry->directory);

    form_.file_manager_app_shell_commands_selection_group_actions_copy->set_enabled(
        mutation_ready && (one || paste_ready));
    form_.file_manager_app_shell_commands_selection_group_actions_delete->set_enabled(
        mutation_ready && one);
    form_.file_manager_app_shell_location_navigation_back->set_enabled(
        command_back_->state().enabled);
    form_.file_manager_app_shell_location_navigation_forward->set_enabled(
        command_forward_->state().enabled);
    form_.file_manager_app_shell_location_navigation_up->set_enabled(
        command_up_->state().enabled);
}

void Application::install_handlers() {
    auto click = [this](const std::shared_ptr<gui_forms::Button>& button,
                        std::function<void()> action) {
        subscriptions_.push_back(button->clicked().subscribe(
            [action = std::move(action)](gui_forms::ButtonBase&) { action(); }));
    };
    click(form_.file_manager_app_shell_location_navigation_back, [this] { navigate_back(); });
    click(form_.file_manager_app_shell_location_navigation_forward, [this] { navigate_forward(); });
    click(form_.file_manager_app_shell_location_navigation_up, [this] { navigate_up(); });
    click(form_.file_manager_app_shell_commands_home,
          [this] { request_navigation(protected_root_, true); });
    click(form_.file_manager_app_shell_commands_arrange_group_actions_refresh,
          [this] {
              show_menu(sort_menu_,
                  form_.file_manager_app_shell_commands_arrange_group_actions_refresh);
          });
    click(form_.file_manager_app_shell_commands_arrange_group_actions_details,
          [this] {
              show_menu(view_menu_,
                  form_.file_manager_app_shell_commands_arrange_group_actions_details);
          });
    click(form_.file_manager_app_shell_commands_new_folder,
          [this] { request_create_folder(); });
    click(form_.file_manager_app_shell_commands_rename,
          [this] { begin_rename(); });
    click(form_.file_manager_app_shell_commands_selection_group_actions_copy,
          [this] {
              show_menu(move_copy_menu_,
                  form_.file_manager_app_shell_commands_selection_group_actions_copy);
          });
    click(form_.file_manager_app_shell_commands_move,
          [this] { capture_transfer(true); });
    click(form_.file_manager_app_shell_commands_paste,
          [this] { paste_transfer(); });
    click(form_.file_manager_app_shell_commands_selection_group_actions_delete,
          [this] { request_quarantine(); });
    click(form_.file_manager_app_shell_commands_arrange_group_actions_settings,
          [this] { show_properties(); });
    click(form_.file_manager_app_shell_workspace_content_heading_more_results,
          [this] { request_engine_search(true); });
    click(form_.file_manager_app_shell_workspace_inspector_commands_open,
          [this] { request_open(); });
    click(form_.file_manager_app_shell_workspace_inspector_commands_checksum,
          [this] { request_checksum(); });
    click(form_.file_manager_app_shell_workspace_inspector_commands_terminal,
          [this] { request_terminal(); });
    click(form_.file_manager_app_shell_workspace_inspector_commands_copy_path,
          [this] { copy_current_path(); });
    click(form_.file_manager_app_shell_settings_actions_back,
          [this] { hide_settings(); });
    click(form_.file_manager_app_shell_settings_actions_cancel,
          [this] { cancel_settings_edits(); });
    click(form_.file_manager_app_shell_settings_actions_reset,
          [this] { reset_settings_page(); });
    click(form_.file_manager_app_shell_settings_actions_apply,
          [this] { apply_settings_changes(); });

    const auto tab = [this, &click](
                         const std::shared_ptr<gui_forms::Button>& button,
                         std::string id, std::string title) {
        click(button, [this, id = std::move(id), title = std::move(title)] {
            select_settings_tab(id, title);
        });
    };
    tab(form_.file_manager_app_shell_settings_body_tabs_general,
        "general", "General");
    tab(form_.file_manager_app_shell_settings_body_tabs_appearance,
        "appearance_access", "Appearance & Access");
    tab(form_.file_manager_app_shell_settings_body_tabs_navigation,
        "navigation_views", "Navigation & Views");
    tab(form_.file_manager_app_shell_settings_body_tabs_search,
        "search_indexing", "Search & Indexing");
    tab(form_.file_manager_app_shell_settings_body_tabs_services,
        "services", "Services");
    tab(form_.file_manager_app_shell_settings_body_tabs_handlers,
        "handlers_commands", "Handlers & Commands");
    tab(form_.file_manager_app_shell_settings_body_tabs_previews,
        "previews_extensions", "Previews & Extensions");
    tab(form_.file_manager_app_shell_settings_body_tabs_applications,
        "applications", "Applications");
    tab(form_.file_manager_app_shell_settings_body_tabs_privacy,
        "privacy_data", "Privacy & Data");
    tab(form_.file_manager_app_shell_settings_body_tabs_advanced,
        "advanced", "Advanced");
    click(form_.file_manager_app_shell_workspace_sidebar_root,
          [this] { request_navigation(protected_root_, true); });
    click(form_.file_manager_app_shell_workspace_sidebar_parent,
          [this] { navigate_up(); });

    subscriptions_.push_back(path_box_->committed().subscribe(
        [this](const std::string& text) { request_navigation(text, true); }));
    subscriptions_.push_back(path_box_->cancelled().subscribe(
        [this] {
            path_box_->set_text(location_.string());
            set_path_editing(false);
        }));
    subscriptions_.push_back(search_box_->committed().subscribe(
        [this](const std::string&) { apply_filter(); }));
    subscriptions_.push_back(search_box_->cancelled().subscribe([this] {
        search_box_->set_text({});
        apply_filter();
    }));
    subscriptions_.push_back(rename_box_->committed().subscribe(
        [this](const std::string& basename) { commit_rename(basename); }));
    subscriptions_.push_back(rename_box_->cancelled().subscribe(
        [this] { cancel_rename(); }));
    subscriptions_.push_back(objects_->selection_changed().subscribe(
        [this](const gui_forms::ObjectSelectionChange& change) {
            update_selection(change.current_id);
        }));
    subscriptions_.push_back(objects_->item_activated().subscribe(
        [this](const std::string& stable_id) { activate(stable_id); }));
    subscriptions_.push_back(objects_->context_requested().subscribe(
        [this](const gui_forms::ObjectContextRequest& request) {
            if (request.stable_id.empty()) {
                objects_->clear_selection();
                update_selection({});
                background_menu_->show(objects_, request.screen_position);
            } else {
                objects_->set_selected_id(request.stable_id);
                object_menu_->show(objects_, request.screen_position);
            }
        }));
    subscriptions_.push_back(objects_->pointer_observed().subscribe(
        [this](const gui_forms::PointerEvent& event) {
            observe_object_pointer(event);
        }));
    subscriptions_.push_back(tree_->item_activated().subscribe(
        [this](const std::string& stable_id) { activate(stable_id); }));
    subscriptions_.push_back(correspondence_->selection_changed().subscribe(
        [this](const gui_forms::CorrespondenceSelectionChange& change) {
            objects_->set_selected_id(change.current_id);
        }));
    subscriptions_.push_back(correspondence_->item_activated().subscribe(
        [this](const std::string& stable_id) { activate(stable_id); }));
    subscriptions_.push_back(correspondence_->context_requested().subscribe(
        [this](const gui_forms::ObjectContextRequest& request) {
            if (request.stable_id.empty()) {
                objects_->clear_selection();
                update_selection({});
                background_menu_->show(correspondence_, request.screen_position);
            } else {
                correspondence_->set_selected_id(request.stable_id);
                objects_->set_selected_id(request.stable_id);
                object_menu_->show(correspondence_, request.screen_position);
            }
        }));
    subscriptions_.push_back(workspace_split_->splitter_changed().subscribe(
        [this](const gui_forms::SplitChangeEvent&) { update_command_state(); }));
    subscriptions_.push_back(selection_split_->splitter_changed().subscribe(
        [this](const gui_forms::SplitChangeEvent&) { update_command_state(); }));
}

void Application::bind_host(std::function<void()> wake,
                            std::function<void()> request_close) {
    {
        std::lock_guard lock(ui_mutex_);
        wake_ = std::move(wake);
        request_close_ = std::move(request_close);
    }
    request_bootstrap();
    request_settings();
    request_navigation(protected_root_, true);
}

void Application::post_worker(std::function<void()> work) {
    {
        std::lock_guard lock(worker_mutex_);
        if (stopping_.load()) return;
        worker_queue_.push(std::move(work));
    }
    worker_cv_.notify_one();
}

void Application::post_ui(std::function<void()> work) {
    std::function<void()> wake;
    {
        std::lock_guard lock(ui_mutex_);
        if (stopping_.load()) return;
        ui_queue_.push(std::move(work));
        wake = wake_;
    }
    if (wake) wake();
}

void Application::worker_loop() {
    for (;;) {
        std::function<void()> work;
        {
            std::unique_lock lock(worker_mutex_);
            worker_cv_.wait(lock, [this] {
                return stopping_.load() || !worker_queue_.empty();
            });
            if (stopping_.load() && worker_queue_.empty()) return;
            work = std::move(worker_queue_.front());
            worker_queue_.pop();
        }
        try {
            work();
        } catch (const std::exception& error) {
            const std::string message = error.what();
            post_ui([this, message] {
                set_status("Background operation failed", message);
            });
        }
    }
}

void Application::drain_ui() {
    std::queue<std::function<void()>> pending;
    {
        std::lock_guard lock(ui_mutex_);
        pending.swap(ui_queue_);
    }
    while (!pending.empty()) {
        pending.front()();
        pending.pop();
    }
}

void Application::stop() {
    if (stopping_.exchange(true)) return;
    {
        std::scoped_lock lock(worker_mutex_, ui_mutex_);
        wake_ = {};
        request_close_ = {};
        while (!ui_queue_.empty()) ui_queue_.pop();
    }
    worker_cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void Application::request_bootstrap() {
    form_.file_manager_app_shell_title_service->set_text("Orchestrator · connecting");
    post_worker([self = shared_from_this()] {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-frontend-1.0");
            const auto snapshot = client.bootstrap();
            const auto gui_state = snapshot.frontend_opening.gui_forms_gate.state;
            const auto service = snapshot.orchestrator_gate_ready()
                ? "Core ready · GUI.Forms " + gui_state
                : "Core blocked · GUI.Forms " + gui_state;
            const auto summary = "Core " + snapshot.release.target_version +
                " · GUI.Forms " + gui_state;
            self->post_ui([self, service, summary] {
                self->form_.file_manager_app_shell_title_service->set_text(service);
                self->form_.file_manager_app_shell_status_summary->set_text(summary);
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            self->post_ui([self, message] {
                self->form_.file_manager_app_shell_title_service->set_text(
                    "Orchestrator · unavailable");
                self->form_.file_manager_app_shell_status_summary->set_text(
                    "Base navigation remains available · " + message);
            });
        }
    });
}

void Application::request_settings() {
    if (settings_loading_ || settings_apply_in_flight_) return;
    settings_loading_ = true;
    form_.file_manager_app_shell_settings_heading_revision->set_text("connecting…");
    post_worker([self = shared_from_this()] {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-settings-1.0");
            auto schema = client.settings_schema();
            auto snapshot = client.settings_snapshot();
            self->post_ui([self, schema = std::move(schema),
                           snapshot = std::move(snapshot)]() mutable {
                self->settings_loading_ = false;
                self->apply_settings_state(std::move(schema),
                                           std::move(snapshot));
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            self->post_ui([self, message] {
                self->settings_loading_ = false;
                self->form_.file_manager_app_shell_settings_heading_revision->set_text(
                    "service unavailable");
                self->form_.file_manager_app_shell_settings_actions_status->set_text(
                    "Settings unavailable · " + message);
                self->update_settings_actions();
            });
        }
    });
}

void Application::request_services() {
    if (services_loading_ || service_command_in_flight_) return;
    services_loading_ = true;
    if (settings_open_ && settings_tab_ == "services") rebuild_settings_page();
    post_worker([self = shared_from_this()] {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-services-1.0");
            auto snapshot = client.services_snapshot();
            self->post_ui([self, snapshot = std::move(snapshot)]() mutable {
                self->services_loading_ = false;
                self->services_snapshot_ = std::move(snapshot);
                if (self->service_notice_.empty()) {
                    self->service_notice_ =
                        "Immutable ORC-UI-001 service observation";
                }
                if (self->settings_open_ && self->settings_tab_ == "services") {
                    self->rebuild_settings_page();
                }
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            self->post_ui([self, message] {
                self->services_loading_ = false;
                self->services_snapshot_.reset();
                self->service_notice_ = "Services unavailable · " + message;
                if (self->settings_open_ && self->settings_tab_ == "services") {
                    self->rebuild_settings_page();
                }
            });
        }
    });
}

void Application::show_settings() {
    settings_open_ = true;
    form_.file_manager_app_shell_commands->set_visible(false);
    form_.file_manager_app_shell_location->set_visible(false);
    form_.file_manager_app_shell_workspace->set_visible(false);
    form_.file_manager_app_shell_settings->set_visible(true);
    if (!settings_schema_ || !settings_snapshot_) request_settings();
    if (settings_tab_ == "services" && !services_snapshot_) request_services();
    rebuild_settings_page();
    set_status("Settings", "ORC-SET-001 · typed optimistic transactions");
}

void Application::hide_settings() {
    settings_open_ = false;
    form_.file_manager_app_shell_settings->set_visible(false);
    form_.file_manager_app_shell_commands->set_visible(true);
    form_.file_manager_app_shell_location->set_visible(true);
    form_.file_manager_app_shell_workspace->set_visible(true);
    if (window_) window_->request_focus(objects_);
    set_status("Files", "direct filesystem · protected root");
}

void Application::select_settings_tab(std::string tab, std::string title) {
    settings_tab_ = std::move(tab);
    form_.file_manager_app_shell_settings_body_page_title->set_text(
        std::move(title));
    form_.file_manager_app_shell_settings_body_page_description->set_text(
        settings_tab_description(settings_tab_));
    if (settings_tab_ == "services") {
        request_services();
    } else {
        rebuild_settings_page();
    }
}

void Application::apply_settings_state(
    fileman::orchestrator::SettingsSchemaInfo schema,
    fileman::orchestrator::SettingsSnapshotInfo snapshot,
    std::string notice) {
    if (schema.schema_revision != snapshot.schema_revision) {
        form_.file_manager_app_shell_settings_actions_status->set_text(
            "Settings schema/value revision mismatch");
        settings_schema_.reset();
        settings_snapshot_.reset();
        update_settings_actions();
        return;
    }
    settings_schema_ = std::move(schema);
    settings_snapshot_ = std::move(snapshot);
    form_.file_manager_app_shell_settings_heading_revision->set_text(
        "revision " + std::to_string(settings_snapshot_->revision) + " · " +
        settings_snapshot_->recovery_provenance);
    apply_runtime_settings();
    rebuild_settings_page();
    if (!notice.empty()) {
        form_.file_manager_app_shell_settings_actions_status->set_text(
            std::move(notice));
    }
    update_settings_actions();
}

void Application::rebuild_settings_page() {
    settings_subscriptions_.clear();
    const bool services_page = settings_tab_ == "services";
    form_.file_manager_app_shell_settings_actions->set_visible(!services_page);
    form_.file_manager_app_shell_settings_body_page_title->set_visible(
        !services_page);
    form_.file_manager_app_shell_settings_body_page_description->set_visible(
        !services_page);
    auto host = form_.file_manager_app_shell_settings_body_page_host;
    host->clear_children();
    settings_property_list_.reset();
    if (settings_tab_ == "services") {
        rebuild_services_page();
        update_settings_actions();
        return;
    }
    if (!settings_schema_ || !settings_snapshot_) {
        auto message = std::make_shared<gui_forms::Label>(
            gui_forms::StableId("fm.settings.loading"),
            settings_loading_ ? "Reading the typed settings schema…"
                              : "Settings service is unavailable");
        message->set_requested_bounds({0, 0, 620, 34});
        message->set_accessible_name("Settings state");
        host->add_child(message);
        return;
    }

    std::vector<gui_forms::PropertyRowSpec> rows;
    for (const auto& field : settings_schema_->fields) {
        if (field.presentation_tab != settings_tab_) continue;
        const auto pending = pending_settings_.find(field.id);
        const auto* committed = settings_snapshot_->find(field.id);
        const auto value = pending != pending_settings_.end()
            ? pending->second
            : (committed ? *committed : field.default_value);
        const bool available = field.availability == "available";
        auto editor = gui_forms::PropertyEditorKind::text;
        if (!available) {
            editor = gui_forms::PropertyEditorKind::read_only;
        } else if (field.value_type == "boolean") {
            editor = gui_forms::PropertyEditorKind::boolean;
        } else if (!field.choices.empty()) {
            editor = gui_forms::PropertyEditorKind::choice;
        }
        std::string description = available
            ? "Availability: available"
            : "Availability: " + field.availability;
        if (!field.availability_reason.empty()) {
            description += " · " + field.availability_reason;
        }
        description += " · restart effect: " + field.restart_effect;
        const auto display_value = available
            ? setting_value_text(value)
            : field.availability +
                (field.availability_reason.empty()
                     ? std::string{}
                     : " — " + field.availability_reason);
        rows.emplace_back(
            field.id, setting_title(field.id), display_value,
            std::move(description), editor, field.choices, std::string{},
            available && !settings_apply_in_flight_);
    }

    if (rows.empty()) {
        rows.emplace_back(
            "fm.settings.page-empty", "Availability",
            "No admitted settings in this category",
            "Service and provider absence is preserved, not shown as empty success.");
    }

    settings_property_list_ = std::make_shared<gui_forms::PropertyList>(
        gui_forms::StableId("fm.settings.properties"));
    settings_property_list_->set_accessible_name("Settings on this page");
    settings_property_list_->set_label_width(210.0);
    settings_property_list_->set_requested_bounds({0, 0, 720, 250});
    settings_property_list_->set_minimum_size({520, 160});
    settings_property_list_->set_maximum_size({4096, 250});
    settings_property_list_->set_groups({
        {"fm.settings.group." + settings_tab_, "SETTINGS", std::move(rows)},
    });
    host->add_child(settings_property_list_);
    host->set_flex_grow(*settings_property_list_, 1.0);
    settings_subscriptions_.push_back(
        settings_property_list_->value_committed().subscribe(
            [this](const gui_forms::PropertyValueChange& change) {
                if (!settings_schema_ || !settings_property_list_) return;
                const auto field = std::find_if(
                    settings_schema_->fields.begin(),
                    settings_schema_->fields.end(),
                    [&change](const auto& candidate) {
                        return candidate.id == change.row_id;
                    });
                if (field == settings_schema_->fields.end() ||
                    field->availability != "available") {
                    return;
                }
                settings_property_list_->set_validation(change.row_id, {});
                if (field->value_type == "boolean") {
                    edit_setting(change.row_id,
                                 change.current_value == "True" ||
                                     change.current_value == "1");
                    return;
                }
                if (field->value_type == "unsigned_integer") {
                    try {
                        std::size_t parsed = 0;
                        const auto value = std::stoull(change.current_value,
                                                      &parsed, 10);
                        if (parsed != change.current_value.size() ||
                            (field->minimum && value < *field->minimum) ||
                            (field->maximum && value > *field->maximum)) {
                            throw std::out_of_range("setting range");
                        }
                        edit_setting(change.row_id,
                            static_cast<std::uint64_t>(value));
                    } catch (const std::exception&) {
                        settings_property_list_->set_value(
                            change.row_id, change.previous_value);
                        settings_property_list_->set_validation(
                            change.row_id,
                            "Enter an integer within the declared range");
                    }
                    return;
                }
                edit_setting(change.row_id, change.current_value);
            }));
    update_settings_actions();
}

void Application::rebuild_services_page() {
    auto host = form_.file_manager_app_shell_settings_body_page_host;
    std::vector<gui_forms::PropertyGroupSpec> groups;
    groups.push_back({
        "fm.services.observation", "OBSERVATION",
        {{"fm.services.refresh", "Service facts", "Refresh service facts",
          "Read a new immutable ORC-UI-001 service snapshot.",
          gui_forms::PropertyEditorKind::custom, {}, {},
          !services_loading_ && !service_command_in_flight_}},
        false,
    });
    groups.front().rows.push_back({
        "fm.services.notice", "Last result",
        services_loading_ ? "Reading ORC-UI-001 service facts…"
                          : (service_notice_.empty()
                                 ? "No service command has run"
                                 : service_notice_),
        "Command outcomes and observation failures remain explicit.",
    });
    if (settings_schema_ && settings_snapshot_) {
        const auto field = std::find_if(
            settings_schema_->fields.begin(), settings_schema_->fields.end(),
            [](const auto& candidate) {
                return candidate.id == "services.diagnostics_level";
            });
        if (field != settings_schema_->fields.end()) {
            groups.front().rows.push_back({
                field->id, "Diagnostics",
                field->availability +
                    (field->availability_reason.empty()
                         ? std::string{}
                         : " — " + field->availability_reason),
                "Availability: " + field->availability + " · " +
                    field->availability_reason,
                gui_forms::PropertyEditorKind::read_only,
            });
        }
    }
    if (!services_snapshot_) {
        groups.front().rows.push_back({
            "fm.services.state", "State",
            services_loading_ ? "Reading ORC-UI-001 service facts…"
                              : service_notice_,
            "Absence and refusal remain explicit.",
        });
    } else {
        for (const auto& service : services_snapshot_->services) {
            std::vector<gui_forms::PropertyRowSpec> rows;
            std::string lifecycle = service.state + " · " + service.transport +
                " · generation " +
                (service.generation ? std::to_string(*service.generation)
                                    : "not reported");
            std::string roots = service.roots.empty() ? "None admitted" : "";
            for (std::size_t index = 0; index < service.roots.size(); ++index) {
                if (index != 0) roots += ", ";
                roots += service.roots[index];
            }
            lifecycle += " · roots " + roots;
            rows.push_back({"fm.services." + service.id + ".lifecycle",
                            "Lifecycle / roots", std::move(lifecycle),
                            (service.ready ? "Ready" : "Not ready") +
                                std::string(" · ") +
                                service.currentness.value_or(
                                    service.reason.value_or(
                                        "no currentness fact reported"))});
            for (const auto& command : service.commands) {
                const bool root_command = service.id == "engine" &&
                    (command.id == "reconcile" || command.id == "rebuild");
                const bool root_admitted = !root_command ||
                    std::find(service.roots.begin(), service.roots.end(),
                              engine_root_id_) != service.roots.end();
                rows.emplace_back(
                    "fm.services." + service.id + ".command." + command.id,
                    command.title, command.title,
                    root_admitted ? command.effect
                                  : command.effect + " · launch root is not admitted",
                    gui_forms::PropertyEditorKind::custom,
                    std::vector<std::string>{}, std::string{},
                    command.available && root_admitted &&
                        !service_command_in_flight_ && !services_loading_);
            }
            groups.push_back({"fm.services.group." + service.id,
                              service.title, std::move(rows),
                              service.id == "engine"});
        }
    }

    settings_property_list_ = std::make_shared<gui_forms::PropertyList>(
        gui_forms::StableId("fm.services.properties"));
    settings_property_list_->set_accessible_name(
        "Installed service facts and admitted commands");
    settings_property_list_->set_label_width(210.0);
    settings_property_list_->set_requested_bounds({0, 0, 720, 250});
    settings_property_list_->set_minimum_size({520, 160});
    settings_property_list_->set_maximum_size({4096, 250});
    settings_property_list_->set_groups(std::move(groups));
    host->add_child(settings_property_list_);
    host->set_flex_grow(*settings_property_list_, 1.0);

    auto refresh = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("fm.services.refresh.button"),
        services_loading_ ? "Reading services…" : "Refresh service facts");
    refresh->set_requested_bounds({0, 0, 190, 28});
    settings_property_list_->replace_editor("fm.services.refresh", refresh);
    settings_subscriptions_.push_back(refresh->clicked().subscribe(
        [this](gui_forms::ButtonBase&) {
            services_snapshot_.reset();
            service_notice_.clear();
            request_services();
        }));

    if (services_snapshot_) {
        for (const auto& service : services_snapshot_->services) {
            for (const auto& command : service.commands) {
                const bool root_command = service.id == "engine" &&
                    (command.id == "reconcile" || command.id == "rebuild");
                const bool root_admitted = !root_command ||
                    std::find(service.roots.begin(), service.roots.end(),
                              engine_root_id_) != service.roots.end();
                const auto row_id = "fm.services." + service.id +
                    ".command." + command.id;
                auto button = std::make_shared<gui_forms::Button>(
                    gui_forms::StableId(row_id + ".button"), command.title);
                button->set_requested_bounds({0, 0, 180, 28});
                button->set_enabled(command.available && root_admitted &&
                                    !service_command_in_flight_ &&
                                    !services_loading_);
                button->set_accessible_description(command.effect);
                settings_property_list_->replace_editor(row_id, button);
                const auto root = root_command
                    ? std::optional<std::string>(engine_root_id_)
                    : std::nullopt;
                settings_subscriptions_.push_back(button->clicked().subscribe(
                    [this, service_id = service.id, command_id = command.id,
                     root](gui_forms::ButtonBase&) {
                        run_service_command(service_id, command_id, root);
                    }));
            }
        }
    }
    form_.file_manager_app_shell_settings_actions_status->set_text(
        service_notice_);
}

bool Application::confirm_service_command(const std::string_view service_id,
                                          const std::string_view command_id) {
    if (command_id != "rebuild" && command_id != "restart" &&
        command_id != "shutdown") {
        return true;
    }
    if (!window_ || !window_->host_services()) return false;
    gui_forms::HostMessageDialogRequest message;
    message.title = "Confirm service command";
    message.message = std::string("Run ") + std::string(command_id) +
        " for " + std::string(service_id) +
        "? The command is identity-checked immediately before execution.";
    message.buttons = gui_forms::HostMessageButtons::yes_no;
    message.icon = gui_forms::HostMessageIcon::warning;
    message.default_choice = gui_forms::HostDialogChoice::no;
    gui_forms::HostDialogRequest request;
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.services";
    request.payload = std::move(message);
    const auto result = window_->host_services()->show_dialog(request);
    const auto* choice = std::get_if<gui_forms::HostMessageDialogResult>(
        &result.payload);
    return choice && choice->choice == gui_forms::HostDialogChoice::yes;
}

void Application::run_service_command(
    std::string service_id, std::string command_id,
    std::optional<std::string> root_id) {
    if (!services_snapshot_ || service_command_in_flight_ ||
        !confirm_service_command(service_id, command_id)) {
        return;
    }
    const auto* service = services_snapshot_->find(service_id);
    if (!service) return;
    const auto instance = service->instance_id;
    const auto generation = service_id == "orchestrator"
        ? service->generation
        : std::optional<std::uint64_t>{};
    service_command_in_flight_ = true;
    service_notice_ = "Running " + service_id + " " + command_id + "…";
    rebuild_settings_page();
    post_worker([self = shared_from_this(), service_id = std::move(service_id),
                 command_id = std::move(command_id), root_id = std::move(root_id),
                 instance, generation] {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-service-command-1.0");
            auto result = client.service_command(service_id, command_id,
                                                 instance, generation, root_id);
            self->post_ui([self, result = std::move(result)]() mutable {
                self->service_command_in_flight_ = false;
                self->services_snapshot_.reset();
                self->service_notice_ = result.service_id + " " +
                    result.command_id + " · " + result.terminal + " · " +
                    result.effect;
                if (result.service_id == "orchestrator" &&
                    result.command_id == "shutdown") {
                    self->rebuild_settings_page();
                } else {
                    self->request_services();
                }
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            self->post_ui([self, message] {
                self->service_command_in_flight_ = false;
                self->services_snapshot_.reset();
                self->service_notice_ = "Service command refused · " + message;
                self->request_services();
            });
        }
    });
}

void Application::edit_setting(
    std::string id, fileman::orchestrator::SettingValue value) {
    if (!settings_snapshot_ || settings_apply_in_flight_) return;
    const auto* committed = settings_snapshot_->find(id);
    if (committed && *committed == value) {
        pending_settings_.erase(id);
    } else {
        pending_settings_[std::move(id)] = std::move(value);
    }
    update_settings_actions();
}

void Application::cancel_settings_edits() {
    if (settings_apply_in_flight_) return;
    pending_settings_.clear();
    rebuild_settings_page();
    form_.file_manager_app_shell_settings_actions_status->set_text(
        "Pending changes cancelled");
}

void Application::reset_settings_page() {
    if (!settings_schema_ || !settings_snapshot_ || settings_apply_in_flight_) {
        return;
    }
    for (const auto& field : settings_schema_->fields) {
        if (field.presentation_tab == settings_tab_ &&
            field.availability == "available") {
            const auto* committed = settings_snapshot_->find(field.id);
            if (committed && *committed == field.default_value) {
                pending_settings_.erase(field.id);
            } else {
                pending_settings_[field.id] = field.default_value;
            }
        }
    }
    rebuild_settings_page();
    form_.file_manager_app_shell_settings_actions_status->set_text(
        "Page defaults staged · Apply commits them");
}

void Application::apply_settings_changes() {
    if (!settings_snapshot_ || pending_settings_.empty() ||
        settings_apply_in_flight_) {
        return;
    }
    std::vector<fileman::orchestrator::SettingChange> changes;
    changes.reserve(pending_settings_.size());
    for (const auto& [id, value] : pending_settings_) {
        changes.push_back({id, value});
    }
    const auto revision = settings_snapshot_->revision;
    settings_apply_in_flight_ = true;
    update_settings_actions();
    rebuild_settings_page();
    form_.file_manager_app_shell_settings_actions_status->set_text(
        "Applying revision " + std::to_string(revision) + "…");
    post_worker([self = shared_from_this(), revision,
                 changes = std::move(changes)]() mutable {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-settings-1.0");
            auto commit = client.apply_settings(revision, std::move(changes));
            self->post_ui([self, commit = std::move(commit)]() mutable {
                self->apply_settings_commit(std::move(commit));
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            try {
                auto client = fileman::orchestrator::Client::connect_default(
                    "file-manager-settings-refresh-1.0");
                auto schema = client.settings_schema();
                auto snapshot = client.settings_snapshot();
                self->post_ui([self, schema = std::move(schema),
                               snapshot = std::move(snapshot), message]() mutable {
                    self->settings_apply_in_flight_ = false;
                    self->apply_settings_state(
                        std::move(schema), std::move(snapshot),
                        "Apply conflict/refusal · refreshed committed values · " +
                            message);
                });
            } catch (const std::exception& refresh_error) {
                const std::string refresh = refresh_error.what();
                self->post_ui([self, message, refresh] {
                    self->settings_apply_in_flight_ = false;
                    self->form_.file_manager_app_shell_settings_actions_status->set_text(
                        "Apply failed · " + message + " · refresh failed · " + refresh);
                    self->update_settings_actions();
                    self->rebuild_settings_page();
                });
            }
        }
    });
}

void Application::apply_settings_commit(
    fileman::orchestrator::SettingsCommitInfo commit) {
    settings_apply_in_flight_ = false;
    pending_settings_.clear();
    const auto audit = commit.audit_id;
    const bool restart = commit.restart_required;
    if (settings_schema_) {
        apply_settings_state(
            std::move(*settings_schema_), std::move(commit.snapshot),
            "Committed · " + audit +
                (restart ? " · restart required" : " · active now"));
    }
}

void Application::update_settings_actions() {
    const bool ready = settings_schema_.has_value() &&
        settings_snapshot_.has_value() && !settings_loading_ &&
        !settings_apply_in_flight_ && settings_tab_ != "services";
    form_.file_manager_app_shell_settings_actions_apply->set_enabled(
        ready && !pending_settings_.empty());
    form_.file_manager_app_shell_settings_actions_cancel->set_enabled(
        ready && !pending_settings_.empty());
    form_.file_manager_app_shell_settings_actions_reset->set_enabled(ready);
    if (ready && !pending_settings_.empty()) {
        form_.file_manager_app_shell_settings_actions_status->set_text(
            std::to_string(pending_settings_.size()) +
            (pending_settings_.size() == 1 ? " pending change"
                                           : " pending changes"));
    } else if (ready &&
               form_.file_manager_app_shell_settings_actions_status->text().empty()) {
        form_.file_manager_app_shell_settings_actions_status->set_text(
            "No pending changes");
    }
}

void Application::apply_runtime_settings() {
    if (!settings_snapshot_) return;
    const bool old_hidden = show_hidden_;
    show_hidden_ = setting_as<bool>(*settings_snapshot_,
                                    "navigation.show_hidden").value_or(false);
    show_extensions_ = setting_as<bool>(*settings_snapshot_,
        "navigation.show_extensions").value_or(true);
    checksum_visible_ = setting_as<bool>(*settings_snapshot_,
        "commands.checksum_visible").value_or(true);
    terminal_visible_ = setting_as<bool>(*settings_snapshot_,
        "commands.open_terminal_visible").value_or(true);
    builtin_previews_enabled_ = setting_as<bool>(*settings_snapshot_,
        "previews.builtin_enabled").value_or(true);
    const auto density = setting_as<std::string>(
        *settings_snapshot_, "appearance.density").value_or("comfortable");
    const bool compact = density == "compact";
    tree_->set_item_height(compact ? 21.0 : 24.0);
    objects_->set_icon_cell_size(compact ? gui_forms::Size{94.0, 78.0}
                                        : gui_forms::Size{104.0, 78.0});
    objects_->set_details_row_height(compact ? 24.0 : 28.0);
    correspondence_->set_compact_height(compact ? 41.0 : 45.0);
    correspondence_->set_expanded_height(compact ? 103.0 : 118.0);
    if (window_) {
        auto presentation = window_->presentation_settings();
        presentation.text_scale = static_cast<double>(setting_as<std::uint64_t>(
            *settings_snapshot_, "appearance.text_scale_percent").value_or(100)) /
            100.0;
        presentation.reduced_motion = setting_as<bool>(
            *settings_snapshot_, "appearance.reduce_motion").value_or(false);
        window_->set_presentation_settings(presentation);
    }
    form_.file_manager_app_shell_workspace_inspector_commands_checksum
        ->set_visible(checksum_visible_);
    form_.file_manager_app_shell_workspace_inspector_commands_expected_host
        ->set_visible(checksum_visible_);
    form_.file_manager_app_shell_workspace_inspector_commands_terminal
        ->set_visible(terminal_visible_);
    const auto default_view = setting_as<std::string>(
        *settings_snapshot_, "navigation.default_view").value_or("icons");
    set_view_mode(default_view == "details"
                      ? gui_forms::ObjectViewMode::details
                      : gui_forms::ObjectViewMode::icons);
    update_command_state();
    if (old_hidden != show_hidden_ && !history_.empty()) {
        request_navigation(location_, false);
    } else if (const auto entry = selected_entry()) {
        request_preview(*entry);
    }
}

void Application::request_navigation(std::filesystem::path path,
                                     const bool add_history) {
    search_generation_.fetch_add(1);
    search_loading_ = false;
    search_cursor_.reset();
    search_order_.clear();
    form_.file_manager_app_shell_workspace_content_heading_more_results->set_enabled(false);
    if (search_showing_) {
        search_showing_ = false;
        filter_.clear();
        search_box_->set_text({});
    }
    const auto generation = requested_generation_.fetch_add(1) + 1;
    const auto root = protected_root_;
    set_status("Reading " + path.string(), "direct filesystem · protected root");
    post_worker([self = shared_from_this(), root, path = std::move(path),
                 generation, add_history] {
        auto snapshot = read_directory(root, path, {}, generation,
            [self, generation] {
                return self->stopping_.load() ||
                    self->requested_generation_.load() != generation;
            }, self->show_hidden_);
        if (snapshot.cancelled) return;
        self->post_ui([self, snapshot = std::move(snapshot), add_history]() mutable {
            self->apply_directory(std::move(snapshot), add_history);
        });
    });
}

void Application::apply_directory(DirectorySnapshot snapshot,
                                  const bool add_history) {
    if (snapshot.generation != requested_generation_.load() ||
        snapshot.generation < applied_generation_) {
        return;
    }
    applied_generation_ = snapshot.generation;
    if (!snapshot.available()) {
        path_box_->set_text(location_.string());
        set_status("Location unavailable", snapshot.error);
        return;
    }
    location_ = std::move(snapshot.location);
    path_box_->set_text(location_.string());
    rebuild_breadcrumb();
    correspondence_->set_items({});
    correspondence_->set_visible(false);
    objects_->set_visible(true);
    form_.file_manager_app_shell_workspace_content_heading->set_visible(false);
    form_.file_manager_app_shell_workspace_content_heading->set_minimum_size({0, 0});
    form_.file_manager_app_shell_workspace_content_heading_copy_title->set_text(
        leaf_name(location_));
    form_.file_manager_app_shell_workspace_content_heading_count->set_text(
        std::to_string(snapshot.entries.size()) +
        (snapshot.entries.size() == 1 ? " object" : " objects"));

    if (add_history) {
        if (!history_.empty() && history_[history_index_] == location_) {
            // A refresh or repeated root command does not fork history.
        } else {
            if (!history_.empty()) {
                history_.erase(history_.begin() +
                                   static_cast<std::ptrdiff_t>(history_index_ + 1),
                               history_.end());
            }
            history_.push_back(location_);
            history_index_ = history_.size() - 1;
        }
    }

    const std::vector<std::string> previous_selection(
        objects_->selected_ids().begin(), objects_->selected_ids().end());
    entries_.clear();
    std::vector<gui_forms::ObjectViewItem> items;
    items.reserve(snapshot.entries.size());
    std::string pending_selection_id;
    for (const auto& entry : snapshot.entries) {
        if (pending_selection_identity_ &&
            entry.identity == *pending_selection_identity_) {
            pending_selection_id = entry.stable_id;
        }
        entries_.emplace(entry.stable_id, entry);
        auto display_name = entry.name;
        if (!show_extensions_ && !entry.directory &&
            entry.kind != EntryKind::symlink) {
            const auto stem = entry.path.stem().string();
            if (!stem.empty()) display_name = stem;
        }
        items.push_back({entry.stable_id, std::move(display_name), entry.secondary_text,
                         kind_text(entry), object_glyph(entry.kind), true, {}});
    }
    objects_->set_items(std::move(items));
    rebuild_object_order();
    std::vector<std::string> retained_selection;
    for (const auto& stable_id : previous_selection) {
        if (entries_.contains(stable_id)) retained_selection.push_back(stable_id);
    }
    if (!pending_selection_id.empty()) {
        objects_->set_selected_id(pending_selection_id);
        pending_selection_identity_.reset();
    } else if (!retained_selection.empty()) {
        objects_->set_selected_ids(std::move(retained_selection));
    } else {
        objects_->clear_selection();
        update_selection({});
    }
    rebuild_tree(snapshot);

    form_.file_manager_app_shell_location_navigation_back->set_enabled(
        !history_.empty() && history_index_ > 0);
    form_.file_manager_app_shell_location_navigation_forward->set_enabled(
        !history_.empty() && history_index_ + 1 < history_.size());
    form_.file_manager_app_shell_location_navigation_up->set_enabled(location_ != protected_root_);
    form_.file_manager_app_shell_workspace_sidebar_parent->set_enabled(
        location_ != protected_root_);
    update_mutation_controls();
    update_command_state();
    set_status(snapshot.entries.empty() ? "This folder is empty" :
                   std::to_string(snapshot.entries.size()) +
                       (snapshot.entries.size() == 1 ? " object" : " objects"),
               "direct filesystem · protected root");
}

void Application::rebuild_tree(const DirectorySnapshot& snapshot) {
    std::vector<gui_forms::TreeViewItem> items;
    tree_locations_.clear();
    const auto root_id = "fm.location.root";
    std::string root_label = leaf_name(protected_root_);
    if (const char* home = std::getenv("HOME"); home && *home &&
        std::filesystem::path(home).lexically_normal() == protected_root_) {
        root_label = "Home";
    }
    form_.file_manager_app_shell_workspace_sidebar_label->set_text(
        root_label == "Home" ? "FOLDERS · HOME-ROOTED"
                             : "FOLDERS · SCOPE-ROOTED");
    items.push_back({root_id, root_label, 0, true, true, true, {}});
    tree_locations_.emplace(root_id, protected_root_);

    auto accumulated = protected_root_;
    std::size_t depth = 1U;
    std::string selected_id = root_id;
    const auto relative = location_.lexically_relative(protected_root_);
    if (!relative.empty() && relative != ".") {
        std::size_t index{};
        for (const auto& component : relative) {
            accumulated /= component;
            const auto stable_id = "fm.location.path." + std::to_string(index++);
            items.push_back({stable_id, component.string(), depth++, true,
                             true, true, {}});
            tree_locations_.emplace(stable_id, accumulated);
            selected_id = stable_id;
        }
    }
    for (const auto& entry : snapshot.entries) {
        if (!entry.directory) continue;
        items.push_back({entry.stable_id, entry.name, depth, true, false, true, {}});
    }
    const std::filesystem::path volumes("/Volumes");
    const bool volumes_in_scope = path_is_within(protected_root_, volumes);
    items.push_back({"fm.location.volumes",
                     volumes_in_scope ? "Volumes" : "Volumes · outside launch scope",
                     0, volumes_in_scope, false, volumes_in_scope, {}});
    if (volumes_in_scope) {
        tree_locations_.emplace("fm.location.volumes", volumes);
    }
    tree_->set_items(std::move(items));
    tree_->set_selected_id(selected_id);
}

void Application::navigate_back() {
    if (history_.empty() || history_index_ == 0) return;
    --history_index_;
    request_navigation(history_[history_index_], false);
}

void Application::navigate_forward() {
    if (history_.empty() || history_index_ + 1 >= history_.size()) return;
    ++history_index_;
    request_navigation(history_[history_index_], false);
}

void Application::navigate_up() {
    if (location_ == protected_root_) return;
    const auto parent = location_.parent_path();
    request_navigation(parent, true);
}

void Application::toggle_view_mode() {
    set_view_mode(details_mode_ ? gui_forms::ObjectViewMode::icons
                                : gui_forms::ObjectViewMode::details);
}

void Application::apply_filter() {
    filter_ = std::string(search_box_->text());
    if (filter_.empty()) {
        request_navigation(location_, false);
    } else {
        request_engine_search();
    }
}

void Application::request_engine_search(const bool next_page) {
    const auto query = filter_;
    if (query.empty() || (next_page && search_loading_)) return;
    std::optional<fileman::orchestrator::SearchCursorInfo> cursor;
    std::uint64_t generation{};
    if (next_page) {
        if (!search_showing_ || !search_cursor_) return;
        cursor = search_cursor_;
        generation = search_generation_.load();
    } else {
        generation = search_generation_.fetch_add(1) + 1U;
        search_cursor_.reset();
        search_order_.clear();
    }
    search_loading_ = true;
    form_.file_manager_app_shell_workspace_content_heading_more_results->set_enabled(false);
    const auto engine_root_id = engine_root_id_;
    auto relative = std::filesystem::relative(location_, protected_root_).generic_string();
    std::optional<std::string> relative_path;
    if (!relative.empty() && relative != ".") relative_path = std::move(relative);
    std::uint32_t maximum_results = 100;
    if (settings_snapshot_) {
        const auto configured = setting_as<std::uint64_t>(
            *settings_snapshot_, "search.result_limit").value_or(100);
        maximum_results = static_cast<std::uint32_t>(
            std::clamp<std::uint64_t>(configured, 25, 500));
    }
    set_status(next_page ? "Loading more matches…" :
                           "Searching “" + query + "”…",
               "Orchestrator → installed Engine · current subtree · paged");
    post_worker([self = shared_from_this(), engine_root_id, relative_path,
                 query, maximum_results, generation, cursor,
                 append = next_page] {
        try {
            auto client = fileman::orchestrator::Client::connect_default(
                "file-manager-search-1.0");
            auto page = client.search_subtree(
                engine_root_id, relative_path, query, maximum_results, cursor);
            self->post_ui([self, page = std::move(page), query,
                           generation, append]() mutable {
                self->apply_engine_search(
                    std::move(page), std::move(query), generation, append);
            });
        } catch (const std::exception& error) {
            const std::string message = error.what();
            self->post_ui([self, generation, message] {
                if (generation != self->search_generation_.load()) return;
                self->search_loading_ = false;
                self->form_.file_manager_app_shell_workspace_content_heading_more_results
                    ->set_enabled(self->search_cursor_.has_value());
                self->set_status("Search unavailable", message);
            });
        }
    });
}

void Application::apply_engine_search(
    fileman::orchestrator::SearchPageInfo page,
    std::string query,
    const std::uint64_t generation,
    const bool append) {
    if (generation != search_generation_.load() ||
        std::string(search_box_->text()) != query) return;
    search_loading_ = false;
    search_showing_ = true;
    if (!append) {
        entries_.clear();
        search_order_.clear();
    }
    const std::vector<std::string> previous_selection(
        objects_->selected_ids().begin(), objects_->selected_ids().end());
    std::size_t rejected{};
    std::size_t duplicate{};
    for (const auto& result : page.results) {
        auto path = result.path.lexically_normal();
        if (!path_is_within(protected_root_, path)) {
            const auto rebased = rebase_path_from_equivalent_root(
                protected_root_, path);
            if (rebased) path = *rebased;
        }
        if (result.unavailable || !path_is_within(protected_root_, path) ||
            path_route_has_symlink(protected_root_, path.parent_path())) {
            ++rejected;
            continue;
        }
        auto identity = observe_identity(path);
        if (!identity.available()) {
            ++rejected;
            continue;
        }
        const auto kind = engine_entry_kind(result.kind, path);
        const bool directory = kind == EntryKind::folder;
        const auto stable_id = search_stable_id(path, identity);
        if (entries_.contains(stable_id)) {
            ++duplicate;
            continue;
        }
        DirectoryEntry entry{
            stable_id,
            path,
            result.name,
            directory ? "Folder" : format_bytes(result.size),
            "Indexed observation",
            identity,
            kind,
            directory,
        };
        entries_.emplace(stable_id, std::move(entry));
        search_order_.push_back(stable_id);
    }
    std::vector<gui_forms::ObjectViewItem> items;
    std::vector<gui_forms::CorrespondenceItem> correspondence_items;
    items.reserve(search_order_.size());
    correspondence_items.reserve(search_order_.size());
    const std::string evidence_source = page.source == "catalogue"
        ? "Local index"
        : "Live filesystem";
    const std::string generation_detail = page.generation
        ? "Engine generation " + std::to_string(*page.generation)
        : "Live filesystem observation; no catalogue generation claimed";
    for (const auto& stable_id : search_order_) {
        const auto found = entries_.find(stable_id);
        if (found == entries_.end()) continue;
        const auto& entry = found->second;
        auto display_name = entry.name;
        if (!show_extensions_ && !entry.directory &&
            entry.kind != EntryKind::symlink) {
            const auto stem = entry.path.stem().string();
            if (!stem.empty()) display_name = stem;
        }
        items.push_back({stable_id, std::move(display_name), entry.secondary_text,
                         kind_text(entry), object_glyph(entry.kind), true, {}});
        auto relative_path = entry.path.lexically_relative(protected_root_)
                                 .generic_string();
        if (relative_path.empty()) relative_path = entry.path.generic_string();
        gui_forms::CorrespondenceItem correspondence{
            stable_id,
            entry.name,
            relative_path,
            kind_text(entry) + " · " + entry.secondary_text,
            "No content claim. Match is from exact local name or path data.",
            page.source == "catalogue" ? "CATALOGUE" : "LIVE",
            "LOCAL EVIDENCE",
            {evidence_source, "Exact filesystem identity"},
            generation_detail,
            object_glyph(entry.kind),
            true,
            false,
            false,
            {query},
        };
        correspondence_items.push_back(std::move(correspondence));
    }
    objects_->set_items(std::move(items));
    rebuild_object_order();
    correspondence_->set_items(std::move(correspondence_items));
    objects_->set_visible(false);
    correspondence_->set_visible(true);
    form_.file_manager_app_shell_workspace_content_heading->set_minimum_size({0, 27});
    form_.file_manager_app_shell_workspace_content_heading->set_visible(true);
    std::vector<std::string> retained_selection;
    for (const auto& stable_id : previous_selection) {
        if (entries_.contains(stable_id)) retained_selection.push_back(stable_id);
    }
    if (!retained_selection.empty()) {
        objects_->set_selected_ids(std::move(retained_selection));
        correspondence_->set_selected_id(objects_->selected_id());
    } else {
        objects_->clear_selection();
        correspondence_->set_selected_id({});
        update_selection({});
    }
    search_cursor_ = std::move(page.cursor);
    form_.file_manager_app_shell_workspace_content_heading_more_results->set_enabled(
        !page.complete && search_cursor_.has_value());
    form_.file_manager_app_shell_workspace_content_heading_copy_title->set_text(
        "Search · " + query);
    form_.file_manager_app_shell_workspace_content_heading_count->set_text(
        std::to_string(entries_.size()) +
        (entries_.size() == 1 ? " result" : " results"));
    update_mutation_controls();
    update_command_state();
    std::string provenance = page.source == "catalogue"
        ? "installed Engine catalogue"
        : "installed Engine live filesystem";
    if (page.generation) {
        provenance += " · generation " + std::to_string(*page.generation);
    }
    if (!page.complete) provenance += " · more results available";
    if (append) provenance += " · appended page";
    if (rejected != 0) {
        provenance += " · " + std::to_string(rejected) +
            " stale/out-of-root result" + (rejected == 1 ? " refused" : "s refused");
    }
    if (duplicate != 0) {
        provenance += " · " + std::to_string(duplicate) +
            " duplicate" + (duplicate == 1 ? " skipped" : "s skipped");
    }
    set_status(entries_.empty() ? "No matches" :
                   std::to_string(entries_.size()) +
                       (entries_.size() == 1 ? " match" : " matches"),
               std::move(provenance));
}

std::optional<DirectoryEntry> Application::selected_entry() const {
    if (objects_->selected_ids().size() != 1U) return std::nullopt;
    const auto found = entries_.find(
        std::string(objects_->selected_ids().front()));
    if (found == entries_.end()) return std::nullopt;
    return found->second;
}

void Application::reset_preview() {
    if (preview_image_id_.value != 0U && window_) {
        (void)window_->remove_image(preview_image_id_);
        preview_image_id_ = {};
    }
    preview_picture_->clear_image();
    preview_picture_->set_visible(false);
    preview_text_->set_text({});
    preview_text_->set_visible(false);
    form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
        ->set_visible(true);
}

void Application::request_preview(const DirectoryEntry& entry) {
    const auto generation = preview_generation_.fetch_add(1) + 1U;
    reset_preview();
    form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
        ->set_text(entry.directory ? "DIR" : "FILE");
    if (!builtin_previews_enabled_) {
        form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
            kind_text(entry) + " · built-in preview disabled");
        return;
    }
    if (entry.directory || entry.kind == EntryKind::symlink) return;
    form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
        kind_text(entry) + " · loading preview…");
    const auto root = protected_root_;
    post_worker([self = shared_from_this(), root, entry, generation] {
        auto result = load_preview(root, entry.path, entry.identity,
            [self, generation] {
                return self->stopping_.load() ||
                    self->preview_generation_.load() != generation;
            });
        self->post_ui([self, result = std::move(result),
                       stable_id = entry.stable_id, generation]() mutable {
            self->apply_preview(std::move(result), std::move(stable_id),
                                generation);
        });
    });
}

void Application::apply_preview(PreviewResult result, std::string stable_id,
                                const std::uint64_t generation) {
    if (generation != preview_generation_.load()) return;
    const auto selected = selected_entry();
    if (!selected || selected->stable_id != stable_id ||
        (!selected->identity.same_revision(result.identity) &&
         (result.kind == PreviewKind::text ||
          result.kind == PreviewKind::png))) {
        return;
    }
    reset_preview();
    if (result.kind == PreviewKind::text) {
        preview_text_->set_text(std::move(result.text_utf8));
        preview_text_->set_visible(true);
        form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
            ->set_visible(false);
        form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
            kind_text(*selected) + " · bounded UTF-8 preview");
        return;
    }
    if (result.kind == PreviewKind::png && window_) {
        const auto loaded = window_->load_png(result.png_bytes);
        if (loaded) {
            preview_image_id_ = loaded.image;
            preview_picture_->set_image(loaded.image);
            preview_picture_->set_visible(true);
            form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
                ->set_visible(false);
            form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
                "Image · bounded PNG preview");
            return;
        }
        result.code = "png-decode-failed";
        result.message = "GUI.Forms rejected the encoded PNG";
    }
    form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
        ->set_text(selected->directory ? "DIR" : "FILE");
    form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
        kind_text(*selected) + " · " + result.message);
}

void Application::update_selection(const std::string_view stable_id) {
    pending_delete_id_.reset();
    if (rename_box_->visible()) cancel_rename();
    if (checksum_in_flight_) {
        checksum_generation_.fetch_add(1);
        checksum_in_flight_ = false;
        form_.file_manager_app_shell_workspace_inspector_commands_checksum
            ->set_text("SHA-256");
    }
    update_mutation_controls();
    if (objects_->selected_ids().size() > 1U) {
        preview_generation_.fetch_add(1);
        reset_preview();
        const auto count = objects_->selected_ids().size();
        form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
            ->set_text("MULTI");
        form_.file_manager_app_shell_workspace_inspector_preview_name->set_text(
            std::to_string(count) + " objects selected");
        form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
            "Properties with multiple values are not synthesized");
        property_list_->set_value("fm.property.kind", "Multiple kinds");
        property_list_->set_value("fm.property.location", location_.string());
        property_list_->set_value("fm.property.size", "Multiple values");
        property_list_->set_value("fm.property.modified", "Multiple values");
        update_command_state();
        return;
    }
    const auto found = entries_.find(std::string(stable_id));
    if (found == entries_.end()) {
        preview_generation_.fetch_add(1);
        reset_preview();
        form_.file_manager_app_shell_workspace_inspector_preview_surface_glyph
            ->set_text("—");
        form_.file_manager_app_shell_workspace_inspector_preview_name->set_text(
            "Nothing selected");
        form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
            "Choose an item to inspect it");
        form_.file_manager_app_shell_workspace_inspector_facts_path->set_text("Path · —");
        form_.file_manager_app_shell_workspace_inspector_facts_size->set_text("Size · —");
        form_.file_manager_app_shell_workspace_inspector_facts_modified->set_text(
            "Modified · —");
        property_list_->set_value("fm.property.kind", "—");
        property_list_->set_value("fm.property.location", "—");
        property_list_->set_value("fm.property.size", "—");
        property_list_->set_value("fm.property.modified", "—");
        update_command_state();
        return;
    }
    const auto& entry = found->second;
    form_.file_manager_app_shell_workspace_inspector_preview_name->set_text(entry.name);
    form_.file_manager_app_shell_workspace_inspector_preview_kind->set_text(
        kind_text(entry));
    form_.file_manager_app_shell_workspace_inspector_facts_path->set_text(
        "Path · " + entry.path.string());
    form_.file_manager_app_shell_workspace_inspector_facts_size->set_text(
        "Size · " + entry.secondary_text);
    form_.file_manager_app_shell_workspace_inspector_facts_modified->set_text(
        "Modified · " + entry.modified_text);
    property_list_->set_value("fm.property.kind", kind_text(entry));
    property_list_->set_value("fm.property.location", entry.path.string());
    property_list_->set_value("fm.property.size", entry.secondary_text);
    property_list_->set_value("fm.property.modified", entry.modified_text);
    update_command_state();
    request_preview(entry);
}

void Application::update_mutation_controls() {
    const bool one_selected = objects_->selected_ids().size() == 1;
    const auto entry = selected_entry();
    const bool available = operations_ != nullptr && !rename_box_->visible() &&
        !transfer_in_flight_;
    form_.file_manager_app_shell_commands_rename->set_enabled(
        available && one_selected);
    form_.file_manager_app_shell_commands_selection_group_actions_copy->set_enabled(
        available && one_selected);
    form_.file_manager_app_shell_commands_move->set_enabled(
        available && one_selected);
    form_.file_manager_app_shell_commands_paste->set_enabled(
        available && pending_transfer_.has_value() &&
        pending_transfer_->entry.path.parent_path() != location_);
    form_.file_manager_app_shell_commands_selection_group_actions_delete->set_enabled(
        available && one_selected);
    form_.file_manager_app_shell_workspace_inspector_commands_open->set_enabled(
        entry.has_value() && entry->kind != EntryKind::symlink);
    form_.file_manager_app_shell_workspace_inspector_commands_checksum->set_enabled(
        checksum_visible_ && (checksum_in_flight_ ||
        (entry.has_value() && !entry->directory &&
         entry->kind != EntryKind::symlink)));
    form_.file_manager_app_shell_workspace_inspector_commands_terminal->set_enabled(
        terminal_visible_ && (!entry.has_value() || entry->directory));
    form_.file_manager_app_shell_workspace_inspector_commands_copy_path->set_enabled(
        true);
    update_command_state();
}

void Application::request_checksum() {
    if (checksum_in_flight_) {
        checksum_generation_.fetch_add(1);
        checksum_in_flight_ = false;
        form_.file_manager_app_shell_workspace_inspector_commands_checksum
            ->set_text("SHA-256");
        update_mutation_controls();
        set_status("Checksum cancellation requested",
                   "no partial digest will be published");
        return;
    }
    const auto entry = selected_entry();
    if (!entry || entry->directory || entry->kind == EntryKind::symlink) return;
    const auto expected = std::string(expected_checksum_box_->text());
    const auto generation = checksum_generation_.fetch_add(1) + 1U;
    checksum_in_flight_ = true;
    form_.file_manager_app_shell_workspace_inspector_commands_checksum
        ->set_text("Cancel hash");
    update_mutation_controls();
    set_status("Computing SHA-256 · " + entry->name,
               "bounded 256 KiB stream · stable revision required");
    const auto root = protected_root_;
    post_worker([self = shared_from_this(), root, entry = *entry, expected,
                 generation] {
        std::uint64_t last_report{};
        auto result = checksum_sha256(
            root, entry.path, entry.identity,
            [self, generation] {
                return self->stopping_.load() ||
                    self->checksum_generation_.load() != generation;
            },
            [self, generation, &last_report](const ChecksumProgress progress) {
                constexpr std::uint64_t report_stride = 8U * 1024U * 1024U;
                if (progress.bytes_read != progress.total_bytes &&
                    progress.bytes_read - last_report < report_stride) {
                    return;
                }
                last_report = progress.bytes_read;
                self->post_ui([self, generation, progress] {
                    if (generation != self->checksum_generation_.load()) return;
                    self->set_status(
                        "Computing SHA-256 · " +
                            format_bytes(progress.bytes_read) + " / " +
                            format_bytes(progress.total_bytes),
                        "Cancel hash stops before publishing a digest");
                });
            });
        self->post_ui([self, result = std::move(result), expected,
                       generation]() mutable {
            self->apply_checksum(std::move(result), std::move(expected),
                                 generation);
        });
    });
}

void Application::apply_checksum(ChecksumResult result, std::string expected,
                                 const std::uint64_t generation) {
    if (generation != checksum_generation_.load()) return;
    checksum_in_flight_ = false;
    form_.file_manager_app_shell_workspace_inspector_commands_checksum
        ->set_text("SHA-256");
    update_mutation_controls();
    if (!result.succeeded()) {
        set_status(result.terminal == ChecksumTerminal::cancelled
                       ? "Checksum cancelled"
                       : "Checksum unavailable · " + result.code,
                   result.message);
        if (result.terminal != ChecksumTerminal::cancelled && window_ &&
            window_->host_services()) {
            gui_forms::HostMessageDialogRequest message;
            message.title = "SHA-256 unavailable";
            message.message = result.message + "\n\nCode: " + result.code +
                "\nObject: " + result.path.string();
            message.buttons = gui_forms::HostMessageButtons::ok;
            message.icon = gui_forms::HostMessageIcon::warning;
            gui_forms::HostDialogRequest request;
            request.request_id = next_host_request_id_++;
            request.owner_id = "file-manager.checksum";
            request.payload = std::move(message);
            (void)window_->host_services()->show_dialog(request);
        }
        return;
    }

    expected.erase(std::remove_if(expected.begin(), expected.end(),
        [](const unsigned char character) { return std::isspace(character); }),
        expected.end());
    std::transform(expected.begin(), expected.end(), expected.begin(),
                   [](const unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    const bool expected_valid = expected.empty() ||
        (expected.size() == 64U &&
         std::all_of(expected.begin(), expected.end(), [](const char character) {
             return std::isxdigit(static_cast<unsigned char>(character)) != 0;
         }));
    std::string comparison;
    if (!expected_valid) {
        comparison = "\n\nExpected value is not 64 hexadecimal characters.";
    } else if (!expected.empty()) {
        comparison = expected == result.digest_hex
            ? "\n\nMATCH · expected digest is identical."
            : "\n\nMISMATCH · expected digest is different.";
    }
    std::string checksum_detail = "stable file revision · digest ready";
    if (!expected_valid) {
        checksum_detail = "expected digest is not 64 hexadecimal characters";
    } else if (!expected.empty()) {
        checksum_detail = expected == result.digest_hex
            ? "expected digest matched"
            : "expected digest did not match";
    }
    set_status("SHA-256 complete · " + format_bytes(result.bytes_read),
               checksum_detail);
    if (!window_ || !window_->host_services()) return;
    gui_forms::HostMessageDialogRequest message;
    message.title = "SHA-256 · " + result.path.filename().string();
    message.message = "Object: " + result.path.string() +
        "\nBytes read: " + std::to_string(result.bytes_read) +
        "\nAlgorithm: SHA-256\n\n" + result.digest_hex + comparison +
        "\n\nCopy this digest to the clipboard?";
    message.buttons = gui_forms::HostMessageButtons::yes_no;
    message.icon = expected_valid && !expected.empty() &&
            expected != result.digest_hex
        ? gui_forms::HostMessageIcon::warning
        : gui_forms::HostMessageIcon::information;
    message.default_choice = gui_forms::HostDialogChoice::yes;
    gui_forms::HostDialogRequest request;
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.checksum";
    request.payload = std::move(message);
    const auto dialog = window_->host_services()->show_dialog(request);
    const auto* answer = std::get_if<gui_forms::HostMessageDialogResult>(
        &dialog.payload);
    if (answer && answer->outcome == gui_forms::HostDialogOutcome::accepted &&
        answer->choice == gui_forms::HostDialogChoice::yes) {
        const auto copied = window_->host_services()->write_clipboard_text(
            result.digest_hex);
        set_status(copied.accepted() ? "SHA-256 copied" :
                                      "Clipboard unavailable",
                   copied.accepted() ? "64 lowercase hexadecimal characters"
                                     : "digest remains visible in the dialog");
    }
}

void Application::request_open() {
    const auto entry = selected_entry();
    if (!entry || entry->kind == EntryKind::symlink) return;
    if (entry->directory) {
        request_navigation(entry->path, true);
        return;
    }
    run_platform_command(PlatformCommandKind::open_default, *entry);
}

void Application::request_terminal() {
    auto entry = selected_entry();
    if (entry && !entry->directory) return;
    if (!entry) {
        entry = DirectoryEntry{"fm.location.current", location_,
            leaf_name(location_), "Folder", "Current location",
            observe_identity(location_), EntryKind::folder, true};
    }
    run_platform_command(PlatformCommandKind::open_terminal_here, *entry);
}

void Application::copy_current_path() {
    const auto entry = selected_entry();
    const auto path = entry ? entry->path : location_;
    if (!window_ || !window_->host_services()) {
        set_status("Clipboard unavailable", path.string());
        return;
    }
    const auto copied = window_->host_services()->write_clipboard_text(
        path.string());
    set_status(copied.accepted() ? "Path copied" : "Clipboard unavailable",
               path.string());
}

void Application::run_platform_command(const PlatformCommandKind kind,
                                       const DirectoryEntry& entry) {
    PlatformCommandPlan plan;
    auto planned = make_platform_command_plan(
        kind, protected_root_, entry.path, entry.identity, plan);
    if (planned.code != "ok") {
        apply_platform_command(kind, std::move(planned), entry.path);
        return;
    }
    set_status(kind == PlatformCommandKind::open_terminal_here
                   ? "Opening Terminal…" : "Opening " + entry.name + "…",
               "macOS argv-only launcher · identity revalidation");
    post_worker([self = shared_from_this(), kind, plan = std::move(plan),
                 path = entry.path]() mutable {
        auto result = execute_platform_command(plan);
        self->post_ui([self, kind, result = std::move(result),
                       path = std::move(path)]() mutable {
            self->apply_platform_command(kind, std::move(result),
                                         std::move(path));
        });
    });
}

void Application::apply_platform_command(const PlatformCommandKind kind,
                                         PlatformCommandResult result,
                                         std::filesystem::path path) {
    set_status(result.launched ? result.message :
                   "Native launch refused · " + result.code,
               result.launched ? "no shell · no elevation · fixed argv"
                               : result.message);
    if (result.launched || !window_ || !window_->host_services()) return;
    gui_forms::HostMessageDialogRequest message;
    message.title = kind == PlatformCommandKind::open_terminal_here
        ? "Terminal unavailable" : "Open unavailable";
    message.message = result.message + "\n\nCode: " + result.code +
        "\nPath: " + path.string();
    message.buttons = gui_forms::HostMessageButtons::ok;
    message.icon = gui_forms::HostMessageIcon::warning;
    gui_forms::HostDialogRequest request;
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.platform-command";
    request.payload = std::move(message);
    (void)window_->host_services()->show_dialog(request);
}

void Application::request_create_folder() {
    if (!operations_) return;
    const auto parent = location_;
    set_status("Creating a folder", "protected operation · collision-safe");
    post_worker([self = shared_from_this(), parent] {
        auto result = self->operations_->create_folder(parent);
        self->post_ui([self, result = std::move(result)]() mutable {
            self->apply_operation(std::move(result));
        });
    });
}

void Application::begin_rename() {
    if (!operations_ || objects_->selected_ids().size() != 1) return;
    const auto stable_id = std::string(objects_->selected_ids().front());
    const auto found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    rename_target_id_ = stable_id;
    pending_delete_id_.reset();
    rename_box_->set_text(found->second.name);
    breadcrumb_->set_visible(false);
    path_box_->set_visible(false);
    rename_box_->set_visible(true);
    rename_box_->select_all();
    update_mutation_controls();
    if (window_) window_->request_focus(rename_box_);
    set_status("Rename " + found->second.name,
               "Enter commits · Escape cancels · collision-safe");
}

void Application::commit_rename(std::string basename) {
    if (!operations_ || !rename_target_id_) return;
    const auto found = entries_.find(*rename_target_id_);
    if (found == entries_.end()) {
        cancel_rename();
        return;
    }
    const auto entry = found->second;
    rename_target_id_.reset();
    rename_box_->set_visible(false);
    set_path_editing(false);
    update_mutation_controls();
    if (window_) window_->request_focus(objects_);
    set_status("Renaming " + entry.name,
               "revalidating no-follow filesystem identity");
    post_worker([self = shared_from_this(), entry,
                 basename = std::move(basename)]() mutable {
        auto result = self->operations_->rename_object(
            entry.path, entry.identity, basename);
        self->post_ui([self, result = std::move(result)]() mutable {
            self->apply_operation(std::move(result));
        });
    });
}

void Application::cancel_rename() {
    rename_target_id_.reset();
    rename_box_->set_visible(false);
    set_path_editing(false);
    update_mutation_controls();
    if (window_) window_->request_focus(objects_);
}

void Application::capture_transfer(const bool move) {
    if (!operations_ || transfer_in_flight_ ||
        objects_->selected_ids().size() != 1) {
        return;
    }
    const auto stable_id = std::string(objects_->selected_ids().front());
    const auto found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    transfer_generation_.fetch_add(1);
    pending_transfer_ = PendingTransfer{found->second, move};
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status((move ? "Move" : "Copy") + std::string(" captured · ") +
                   found->second.name,
               "navigate to a different folder and press Paste");
}

void Application::paste_transfer() {
    if (!operations_ || transfer_in_flight_ || !pending_transfer_ ||
        pending_transfer_->entry.path.parent_path() == location_) {
        return;
    }
    const auto transfer = *pending_transfer_;
    const auto destination = location_;
    const auto generation = transfer_generation_.fetch_add(1) + 1;
    transfer_in_flight_ = true;
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status(std::string(transfer.move ? "Moving " : "Copying ") +
                   transfer.entry.name,
               transfer.move ? "same-volume identity-preserving publication"
                             : "staged no-follow copy · no overwrite");
    post_worker([self = shared_from_this(), transfer, destination, generation] {
        OperationResult result;
        if (transfer.move) {
            result = self->operations_->move_object(
                transfer.entry.path, transfer.entry.identity, destination);
        } else {
            result = self->operations_->copy_object(
                transfer.entry.path, transfer.entry.identity, destination,
                [self, generation] {
                    return self->stopping_.load() ||
                        self->transfer_generation_.load() != generation;
                });
        }
        self->post_ui([self, result = std::move(result), generation]() mutable {
            self->apply_transfer(std::move(result), generation);
        });
    });
}

void Application::observe_object_pointer(const gui_forms::PointerEvent& event) {
    if (!operations_ || transfer_in_flight_) return;
    const auto item_id = objects_->item_id_at(event.position);
    if (event.action == gui_forms::PointerAction::down &&
        event.button == gui_forms::PointerButton::primary) {
        (void)pointer_drag_.observe({DragPointerPhase::down, event.position.x,
                                    event.position.y, item_id, false});
        pointer_drag_hover_id_.clear();
        return;
    }
    if (pointer_drag_.source_id().empty()) return;
    if (event.action == gui_forms::PointerAction::move) {
        (void)pointer_drag_.observe({DragPointerPhase::move, event.position.x,
                                    event.position.y, item_id, false});
        if (!pointer_drag_.active()) return;
        const auto target_id = item_id;
        if (target_id == pointer_drag_hover_id_) return;
        pointer_drag_hover_id_ = std::string(target_id);
        const auto source = entries_.find(std::string(pointer_drag_.source_id()));
        const auto target = entries_.find(pointer_drag_hover_id_);
        if (source != entries_.end() && target != entries_.end() &&
            target->second.directory && target->first != source->first) {
            set_status("Drop " + source->second.name + " on " +
                           target->second.name,
                       "ordinary drag moves · hold Option while dropping to copy");
        } else if (source != entries_.end()) {
            set_status("Dragging " + source->second.name,
                       "drop on a visible folder · Option copies");
        }
        return;
    }
    if (event.action != gui_forms::PointerAction::up ||
        event.button != gui_forms::PointerButton::primary) {
        return;
    }

    const auto intent = pointer_drag_.observe({
        DragPointerPhase::up, event.position.x, event.position.y, item_id,
        gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::alt)});
    pointer_drag_hover_id_.clear();
    if (!intent) return;
    const auto source = entries_.find(intent->source_id);
    const auto target = entries_.find(intent->destination_id);
    if (source == entries_.end() || target == entries_.end() ||
        !target->second.directory || target->first == source->first) {
        set_status("Drag cancelled", "no destination folder accepted the object");
        return;
    }
    request_internal_drop(
        source->second, target->second,
        intent->copy);
}

void Application::request_internal_drop(const DirectoryEntry& source,
                                        const DirectoryEntry& destination,
                                        const bool copy) {
    if (!operations_ || transfer_in_flight_ || !destination.directory) return;
    pending_transfer_.reset();
    const auto generation = transfer_generation_.fetch_add(1) + 1;
    transfer_in_flight_ = true;
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status(std::string(copy ? "Copying " : "Moving ") + source.name +
                   " to " + destination.name,
               copy ? "drag copy · staged no-overwrite publication"
                    : "drag move · same-volume identity publication");
    post_worker([self = shared_from_this(), source, destination, copy,
                 generation] {
        OperationResult result;
        if (copy) {
            result = self->operations_->copy_object(
                source.path, source.identity, destination.path,
                [self, generation] {
                    return self->stopping_.load() ||
                        self->transfer_generation_.load() != generation;
                });
        } else {
            result = self->operations_->move_object(
                source.path, source.identity, destination.path);
        }
        self->post_ui([self, result = std::move(result), generation]() mutable {
            self->apply_transfer(std::move(result), generation);
        });
    });
}

void Application::request_quarantine() {
    if (!operations_ || objects_->selected_ids().size() != 1) return;
    const auto stable_id = std::string(objects_->selected_ids().front());
    const auto found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    if (!pending_delete_id_ || *pending_delete_id_ != stable_id) {
        pending_delete_id_ = stable_id;
        set_status("Delete armed · press Delete again for " + found->second.name,
                   "recoverable quarantine · no permanent removal");
        return;
    }
    pending_delete_id_.reset();
    const auto entry = found->second;
    set_status("Moving " + entry.name + " to quarantine",
               "revalidating no-follow filesystem identity");
    post_worker([self = shared_from_this(), entry] {
        auto result = self->operations_->quarantine_object(
            entry.path, entry.identity);
        self->post_ui([self, result = std::move(result)]() mutable {
            self->apply_operation(std::move(result));
        });
    });
}

void Application::request_undo() {
    if (!operations_) return;
    set_status("Undoing the last operation",
               "identity and destination revalidation");
    post_worker([self = shared_from_this()] {
        auto result = self->operations_->undo_last();
        self->post_ui([self, result = std::move(result)]() mutable {
            self->apply_operation(std::move(result));
        });
    });
}

void Application::apply_operation(OperationResult result) {
    if (!result.succeeded()) {
        set_status("Operation refused · " + result.code, result.message);
        update_mutation_controls();
        show_operation_failure(result);
        return;
    }
    if (result.identity.available() && !result.resulting_path.empty() &&
        path_is_within(protected_root_, result.resulting_path)) {
        pending_selection_identity_ = result.identity;
    } else {
        pending_selection_identity_.reset();
    }
    set_status(result.message,
               result.operation_id + (result.undo_available ? " · undo available"
                                                             : " · committed"));
    request_navigation(location_, false);
}

void Application::apply_transfer(OperationResult result,
                                 const std::uint64_t generation) {
    transfer_in_flight_ = false;
    if (generation == transfer_generation_.load() && result.succeeded()) {
        pending_transfer_.reset();
    }
    update_mutation_controls();
    apply_operation(std::move(result));
}

void Application::show_operation_failure(const OperationResult& result) {
    if (result.terminal == OperationTerminal::cancelled || !window_ ||
        !window_->host_services()) {
        return;
    }
    gui_forms::HostMessageDialogRequest message;
    message.title = "File operation refused";
    message.message = result.message + "\n\nCode: " + result.code;
    if (!result.original_path.empty()) {
        message.message += "\nObject: " + result.original_path.string();
    }
    if (!result.resulting_path.empty()) {
        message.message += "\nDestination: " + result.resulting_path.string();
    }
    message.buttons = gui_forms::HostMessageButtons::ok;
    message.icon = result.terminal == OperationTerminal::conflict
        ? gui_forms::HostMessageIcon::warning
        : gui_forms::HostMessageIcon::error;
    message.default_choice = gui_forms::HostDialogChoice::ok;

    gui_forms::HostDialogRequest request;
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.operations";
    request.payload = std::move(message);
    (void)window_->host_services()->show_dialog(request);
}

void Application::activate(const std::string_view stable_id) {
    const auto tree_location = tree_locations_.find(std::string(stable_id));
    if (tree_location != tree_locations_.end()) {
        request_navigation(tree_location->second, true);
        return;
    }
    const auto found = entries_.find(std::string(stable_id));
    if (found == entries_.end()) return;
    if (found->second.directory) {
        request_navigation(found->second.path, true);
    } else {
        run_platform_command(PlatformCommandKind::open_default, found->second);
    }
}

void Application::set_status(std::string text, std::string summary) {
    form_.file_manager_app_shell_status_ready->set_text(std::move(text));
    form_.file_manager_app_shell_status_summary->set_text(std::move(summary));
}

} // namespace file_manager
