#include "file_manager/platform_paths.hpp"
#include "application.hpp"
#include "application_jobs.hpp"

#include "fileman_orchestrator/client.hpp"
#include "house_art.hpp"
#include "object_order.hpp"
#include "details_projection.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace file_manager {
namespace {

// These predicates borrow their keys only during synchronous library calls.
struct BreadcrumbId final {
    std::string_view id{};
    bool operator()(const gui_forms::BreadcrumbSegment& segment) const {
        const bool matches = segment.stable_id == id;
        return matches;
    }
};
struct SettingId final {
    std::string_view id{};
    bool operator()(const fileman::orchestrator::SettingSchemaFieldInfo& field) const {
        const bool matches = field.id == id;
        return matches;
    }
};
struct ModuleId final {
    std::string_view id{};
    bool operator()(const gui_forms::InstrumentModuleSpec& module) const {
        const bool matches = module.stable_id == id;
        return matches;
    }
};
bool module_enabled(const gui_forms::InstrumentModuleSpec& module) {
    return module.enabled;
}
bool contains_module(const std::vector<gui_forms::InstrumentModuleSpec>& modules,
                     const std::string_view id) {
    const bool found = std::any_of(modules.begin(), modules.end(), ModuleId{id});
    return found;
}
std::string_view criteria_field_value(const gui_forms::InstrumentModuleSpec& module,
                                     const std::string_view id) {
    for (const gui_forms::InstrumentFieldSpec& field : module.fields) {
        if (field.stable_id == id) return field.value;
    }
    return {};
}
bool selection_group_collapsed(const gui_forms::CommandOverflowGroupResult& group) {
    const bool collapsed = group.stable_id ==
        "file-manager-app.shell.commands.selection-group" && group.collapsed;
    return collapsed;
}
std::size_t rename_basename_extent(const DirectoryEntry& entry) {
    const std::size_t full_extent = entry.name.size();
    if (entry.directory) return full_extent;
    const std::size_t dot = entry.name.find_last_of('.');
    // A leading dot alone is a filename; an empty trailing suffix is not an
    // extension. Otherwise retain the final extension, including its dot.
    if (dot == std::string::npos || dot == 0U || dot + 1U == full_extent) return full_extent;
    // The ASCII separator is a UTF-8 boundary; no byte/code-point conversion.
    return dot;
}
int decimal_digit(const char character) {
    const int value = character >= '0' && character <= '9' ? character - '0' : -1;
    return value;
}
char lowercase_byte(const unsigned char character) {
    const char lower = static_cast<char>(std::tolower(character));
    return lower;
}
bool whitespace_byte(const unsigned char character) {
    const bool space = std::isspace(character) != 0;
    return space;
}
bool hexadecimal_byte(const char character) {
    const bool hexadecimal = std::isxdigit(static_cast<unsigned char>(character)) != 0;
    return hexadecimal;
}
struct BreadcrumbPath final {
    std::string text{};
    std::filesystem::path path{};
};
struct SettingsTab final {
    std::shared_ptr<gui_forms::Button> button{};
    std::string_view id{};
};

// The subscription owns the action; Application owns and revokes the connection.
// Bound member targets borrow Application, which outlives its connections.
struct ButtonAction final {
    std::function<void()> action{};
    void operator()(gui_forms::ButtonBase&) const { action(); }
};

struct CommandExecution final {
    std::shared_ptr<gui_forms::Command> command{};
    std::string source{};
    void operator()() const { static_cast<void>((*command).execute(source)); }
};

// The accelerator observes its command and borrows the Application predicate.
// Its token is revoked before Application teardown; a vanished command is inert.
struct CommandAccelerator final {
    std::weak_ptr<gui_forms::Command> command{};
    std::string source{};
    std::function<bool()> predicate{};
    bool operator()() const {
        if (predicate && !predicate()) return false;
        const std::shared_ptr<gui_forms::Command> target = command.lock();
        if (!target) return false;
        const bool handled = (*target).execute(source);
        return handled;
    }
};

struct MenuOpenProjection final {
    std::shared_ptr<gui_forms::DropDownButton> button{};
    void operator()(const bool open) const { (*button).set_drop_down_open(open); }
};

struct CommandAction final {
    std::function<void()> action{};
    void operator()(const gui_forms::CommandInvocation&) const { action(); }
};

class HouseImage final : public gui_forms::Button {
public:
    HouseImage(gui_forms::StableId stable_id, std::string accessible_name)
        : gui_forms::Button(std::move(stable_id)) {
        set_focusable(false);
        set_accessible_name(std::move(accessible_name));
    }

    [[nodiscard]] gui_forms::SemanticDescriptor semantic_descriptor()
        const override {
        gui_forms::SemanticDescriptor descriptor{};
        descriptor.role = gui_forms::SemanticRole::image;
        descriptor.name = accessible_name();
        descriptor.exposed = true;
        return descriptor;
    }

    bool on_semantic_action(gui_forms::SemanticAction,
                            std::string_view) override {
        return false;
    }

    [[nodiscard]] bool hit_test_local(gui_forms::Point) const override {
        return false;
    }
};

[[nodiscard]] gui_forms::BasicControlStyle transparent_image_style() {
    gui_forms::BasicControlStyle style{};
    const gui_forms::Color clear = gui_forms::Color::rgba(0, 0, 0, 0);
    style.face = clear;
    style.face_light = clear;
    style.paper = clear;
    style.highlight = clear;
    style.border = clear;
    style.dark_border = clear;
    style.text = clear;
    style.disabled_text = clear;
    style.accent = clear;
    style.accent_light = clear;
    style.link = clear;
    style.visited_link = clear;
    return style;
}

[[nodiscard]] gui_forms::BasicControlStyle tree_mode_style() {
    gui_forms::BasicControlStyle style = transparent_image_style();
    style.text = gui_forms::Color::rgba(75, 101, 129);
    style.disabled_text = gui_forms::Color::rgba(116, 130, 145);
    return style;
}

[[nodiscard]] gui_forms::BasicControlStyle breadcrumb_style() {
    gui_forms::BasicControlStyle style{};
    style.face = gui_forms::Color::rgba(231, 237, 246);
    style.face_light = gui_forms::Color::rgba(250, 253, 255);
    style.paper = gui_forms::Color::rgba(255, 255, 255);
    style.highlight = gui_forms::Color::rgba(255, 255, 255);
    style.border = gui_forms::Color::rgba(119, 140, 171);
    style.dark_border = gui_forms::Color::rgba(23, 45, 105);
    style.text = gui_forms::Color::rgba(29, 45, 75);
    style.disabled_text = gui_forms::Color::rgba(132, 143, 153);
    style.accent = gui_forms::Color::rgba(58, 104, 203);
    style.accent_light = gui_forms::Color::rgba(105, 143, 222);
    style.link = gui_forms::Color::rgba(25, 82, 139);
    style.visited_link = gui_forms::Color::rgba(93, 65, 145);
    return style;
}

class CriteriaConsolePanel final : public gui_forms::Panel {
public:
    CriteriaConsolePanel()
        : gui_forms::Panel(gui_forms::StableId("fm.criteria.console")) {
        set_background(gui_forms::Color::rgba(228, 237, 243));
        set_accessible_name("Criteria virtual folder instrument rack");
        set_auto_size(true);
        set_auto_size_mode(gui_forms::AutoSizeMode::grow_and_shrink);
    }

    std::shared_ptr<gui_forms::Label> title{};
    std::shared_ptr<gui_forms::InstrumentRack> rack{};

    [[nodiscard]] gui_forms::Size measure(
        const gui_forms::Size available) override {
        const double scale = effective_text_scale();
        const double rack_width = std::max(1.0, available.width - 16.0 * scale);
        const double desired = 29.0 * scale +
            (rack ? (*rack).preferred_height(rack_width) : 0.0) + 7.0 * scale;
        const gui_forms::Size result{available.width, std::min(available.height, desired)};
        return result;
    }

    void arrange(const gui_forms::Rect final_bounds) override {
        arrange_self(final_bounds);
        const double scale = effective_text_scale();
        if (title) {
            set_child_layout(title, {8.0 * scale, 4.0 * scale,
                                     std::max(0.0, final_bounds.width -
                                                       16.0 * scale),
                                     21.0 * scale});
        }
        if (rack) {
            set_child_layout(rack, {8.0 * scale, 28.0 * scale,
                                    std::max(0.0, final_bounds.width -
                                                      16.0 * scale),
                                    std::max(0.0, final_bounds.height -
                                                      35.0 * scale)});
        }
    }

    void on_paint(gui_forms::Painter& painter,
                  const gui_forms::Rect damage) override {
        gui_forms::Panel::on_paint(painter, damage);
        const double scale = effective_text_scale();
        painter.fill_rect({0.0, 0.0, committed_arranged_bounds().width,
                           1.0 * scale},
                          gui_forms::Color::rgba(250, 253, 255));
        painter.draw_line(
            {0.0, committed_arranged_bounds().height - 0.5 * scale},
            {committed_arranged_bounds().width,
             committed_arranged_bounds().height - 0.5 * scale},
            gui_forms::Color::rgba(92, 119, 140), scale);
    }
};

gui_forms::InstrumentFieldSpec criteria_choice_field(
    std::string id, std::string name, std::string value,
    std::vector<std::string> choices, const double weight = 1.0) {
    gui_forms::InstrumentFieldSpec result{std::move(id), std::move(name), std::move(value),
            gui_forms::InstrumentFieldEditor::choice, std::move(choices), {},
            weight, true};
    return result;
}

gui_forms::InstrumentFieldSpec criteria_text_field(
    std::string id, std::string name, std::string value,
    const double weight = 1.0) {
    gui_forms::InstrumentFieldSpec result{std::move(id), std::move(name), std::move(value),
            gui_forms::InstrumentFieldEditor::text, {}, {}, weight, true};
    return result;
}

gui_forms::InstrumentModuleSpec kind_criteria_module() {
    gui_forms::InstrumentModuleSpec result{"fm.criteria.kind", "Kind",
            {criteria_choice_field("field", "Field", "Kind", {"Kind"}, 0.75),
             criteria_choice_field("operator", "Operator", "is", {"is"}, 0.5),
             criteria_choice_field("value", "Value", "Files",
                                   {"Files", "Folders", "Symbolic links",
                                    "Other"},
                                   1.25)},
            "committed exact filter", gui_forms::InstrumentModuleState::live,
            10, true, true};
    return result;
}

gui_forms::InstrumentModuleSpec modified_criteria_module() {
    gui_forms::InstrumentModuleSpec result{"fm.criteria.modified", "Modified",
            {criteria_choice_field("field", "Field", "Modified",
                                   {"Modified"}, 0.85),
             criteria_choice_field("operator", "Operator", "after",
                                   {"after", "before"}, 0.75),
             criteria_text_field("value", "Date (YYYY-MM-DD)", "2026-01-01",
                                 1.4)},
            "committed exact filter", gui_forms::InstrumentModuleState::live,
            20, true, true};
    return result;
}

gui_forms::InstrumentModuleSpec size_criteria_module() {
    gui_forms::InstrumentModuleSpec result{"fm.criteria.size", "Size",
            {criteria_choice_field("field", "Field", "Size", {"Size"}, 0.7),
             criteria_choice_field("operator", "Operator", "at least",
                                   {"at least", "at most"}, 0.95),
             criteria_text_field("value", "Bytes", "0", 1.35)},
            "committed exact filter", gui_forms::InstrumentModuleState::live,
            30, true, true};
    return result;
}

std::optional<std::int64_t> parse_criteria_date(std::string_view value) {
    if (value.size() != 10U || value[4] != '-' || value[7] != '-') return {};
    int parts[8]{};
    for (const std::pair<unsigned, unsigned>& indices :
         {std::pair{0U, 0U}, {1U, 1U}, {2U, 2U}, {3U, 3U},
          {5U, 4U}, {6U, 5U}, {8U, 6U}, {9U, 7U}}) {
        const unsigned source = indices.first;
        const unsigned target = indices.second;
        parts[target] = decimal_digit(value[source]);
        if (parts[target] < 0) return {};
    }
    const int year_value = parts[0] * 1000 + parts[1] * 100 +
        parts[2] * 10 + parts[3];
    const unsigned month_value = static_cast<unsigned>(parts[4] * 10 + parts[5]);
    const unsigned day_value = static_cast<unsigned>(parts[6] * 10 + parts[7]);
    if (year_value < 1970 || year_value > 2261) return {};
    const std::chrono::year_month_day date{
        std::chrono::year{year_value}, std::chrono::month{month_value},
        std::chrono::day{day_value}};
    if (!date.ok()) return {};
    const std::chrono::sys_days midnight{date};
    const std::chrono::days elapsed = midnight.time_since_epoch();
    const std::chrono::days::rep days = elapsed.count();
    constexpr std::int64_t nanoseconds_per_day = 86'400'000'000'000LL;
    const std::int64_t nanoseconds = static_cast<std::int64_t>(days) * nanoseconds_per_day;
    return nanoseconds;
}

std::optional<std::int64_t> parse_criteria_size(std::string_view value) {
    if (value.empty()) return {};
    std::int64_t result{};
    const std::from_chars_result parsed = std::from_chars(value.data(), value.data() + value.size(),
                                        result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() ||
        result < 0) {
        return {};
    }
    return result;
}

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

std::string_view sort_title(const std::string_view mode) {
    if (mode == "name") return "Name";
    if (mode == "kind") return "Type";
    if (mode == "size") return "Size";
    if (mode == "modified") return "Date modified";
    throw std::invalid_argument("Unknown object sort column");
}

std::string leaf_name(const std::filesystem::path& path) {
    std::string name = path_utf8(path.filename());
    if (name.empty()) name = path_utf8(path);
    return name;
}

std::string breadcrumb_stable_id(const std::filesystem::path& path) {
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offset;
    for (const unsigned char value : path_generic_utf8(path)) {
        hash ^= value;
        hash *= prime;
    }
    std::ostringstream stream{};
    stream << "fm.path.segment." << std::hex << std::setw(16)
           << std::setfill('0') << hash;
    const std::string result = stream.str();
    return result;
}

std::string setting_title(std::string_view id) {
    const std::size_t separator = id.find_last_of('.');
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
    if (const bool* const boolean = std::get_if<bool>(&value)) {
        const std::string text = *boolean ? "True" : "False";
        return text;
    }
    if (const std::uint64_t* const number = std::get_if<std::uint64_t>(&value)) {
        const std::string text = std::to_string(*number);
        return text;
    }
    const std::string& text = std::get<std::string>(value);
    return text;
}

template <typename T>
std::optional<T> setting_as(
    const fileman::orchestrator::SettingsSnapshotInfo& snapshot,
    std::string_view id) {
    const fileman::orchestrator::SettingValue* const value = snapshot.find(id);
    if (!value) return std::nullopt;
    if (const T* const typed = std::get_if<T>(value)) return *typed;
    return std::nullopt;
}

} // namespace

Application::Application(std::filesystem::path protected_root,
                         std::optional<std::filesystem::path> quarantine_root,
                         const bool mutations_enabled,
                         std::string engine_root_id)
    : protected_root_(canonical_existing_directory(protected_root)),
      home_root_(protected_root_),
      navigation_root_(protected_root_),
      location_(protected_root_),
      engine_root_id_(std::move(engine_root_id)),
      form_(web_forms_generated_file_manager_sapphire::make_native_form()) {
    install_command_shelf_controls();
    home_root_ = canonical_existing_directory(user_home_directory());
    navigation_roots_.push_back(home_root_);
    const std::vector<std::filesystem::path> volumes = local_volume_roots();
    for (const std::filesystem::path& volume : volumes) {
        if (volume != home_root_) navigation_roots_.push_back(volume);
#if defined(__APPLE__)
        volumes_root_ = volume;
#endif
    }
    const bool launch_root_admitted = std::find(
        navigation_roots_.begin(), navigation_roots_.end(), protected_root_) !=
        navigation_roots_.end();
    if (!launch_root_admitted) navigation_roots_.push_back(protected_root_);
    const std::optional<NavigationTarget> initial_target = resolve_navigation_target(
        navigation_roots_, protected_root_, home_root_, protected_root_);
    if (!initial_target) {
        throw std::logic_error("launch root was not admitted for navigation");
    }
    navigation_root_ = (*initial_target).root;
    tree_root_mode_ = navigation_root_;
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
    worker_ = std::thread(&Application::worker_loop, this);
}

Application::~Application() {
    stop();
    // Revoke every borrowed UI target while Application and its controls live.
    accelerator_tokens_.clear();
    settings_subscriptions_.clear();
    path_suggestion_subscriptions_.clear();
    breadcrumb_subscriptions_.clear();
    subscriptions_.clear();
}

std::unique_ptr<gui_forms::Window> Application::make_window() {
    std::unique_ptr<gui_forms::Window> window = std::make_unique<gui_forms::Window>(
        form_.root_control(), gui_forms::Size{1340, 850});
    // Image-list attachment invalidates live controls and can synchronously
    // enter installed application callbacks; publish the owning window first.
    window_ = window.get();
    subscriptions_.push_back((*window_).presentation_changed().subscribe(
        std::bind_front(&Application::on_details_presentation_changed, this)));
    web_forms_generated_file_manager_sapphire::bind_native_resources(
        form_, *window_);
    install_house_art();
    install_accelerators();
    return window;
}

void Application::install_command_shelf_controls() {
    shelf_move_copy_button_ =
        form_.file_manager_app_shell_commands_selection_group_actions_copy;
    shelf_view_button_ =
        form_.file_manager_app_shell_commands_arrange_group_actions_details;
    shelf_sort_button_ =
        form_.file_manager_app_shell_commands_arrange_group_actions_refresh;
    shelf_overflow_button_ = form_.file_manager_app_shell_commands_overflow;

    (*shelf_view_button_).set_accessible_description(
        "Current presentation: Small icons. Choose the object presentation");
    (*shelf_overflow_button_).set_accessible_name("More command shelf actions");
    (*shelf_overflow_button_).set_accessible_description(
        "Shows shelf commands hidden by the current window width");
    (*form_.file_manager_app_shell_commands_selection_group).set_accessible_name(
        "Selection commands");
    (*form_.file_manager_app_shell_commands_arrange_group).set_accessible_name(
        "Arrange and inspect commands");

}

void Application::install_house_art() {
    if (!window_) return;
    object_images_ = house_art::make_image_list(*window_, 42.0);
    tree_images_ = house_art::make_image_list(*window_, 17.0);
    preview_images_ = house_art::make_image_list(*window_, 72.0);
    (*objects_).set_image_list(object_images_);
    (*tree_).set_image_list(tree_images_);
    (*preview_house_icon_).set_image_list(preview_images_);
}

void Application::install_dynamic_controls() {
    menu_strip_ = std::make_shared<gui_forms::MenuStrip>(
        gui_forms::StableId("fm.application.menu"));
    (*menu_strip_).set_font({gui_forms::FontRole::control, 13.0, 400, false});
    (*menu_strip_).set_accessible_name("File Manager application menu");
    (*menu_strip_).set_requested_bounds({0, 0, 720, 28});
    (*form_.file_manager_app_shell_menu).clear_children();
    (*form_.file_manager_app_shell_menu).add_child(menu_strip_);
    (*form_.file_manager_app_shell_menu).set_flex_grow(*menu_strip_, 1.0);

    breadcrumb_ = gui_forms::make_control<gui_forms::BreadcrumbTrail>(
        gui_forms::StableId("fm.path.breadcrumb"), "fm.path.editor");
    (*breadcrumb_).set_requested_bounds({0, 0, 620, 26});
    (*breadcrumb_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*breadcrumb_).set_appearance(gui_forms::BreadcrumbAppearance::raised);
    (*breadcrumb_).set_border_style(gui_forms::BorderStyle::none);
    (*breadcrumb_).set_style(breadcrumb_style());
    (*breadcrumb_).set_accessible_name("Current location breadcrumb");
    (*breadcrumb_).set_accessible_description(
        "Navigate stable path segments or switch to exact path entry");
    path_box_ = (*breadcrumb_).editor();
    (*path_box_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*path_box_).set_text(path_utf8(protected_root_));
    (*path_box_).set_placeholder_text("Enter a path in Home or Volumes");
    (*path_box_).set_accessible_name("Current location path");
    (*form_.file_manager_app_shell_location_path_host).clear_children();
    (*form_.file_manager_app_shell_location_path_host).add_child(breadcrumb_);
    (*form_.file_manager_app_shell_location_path_host).set_flex_grow(*breadcrumb_, 1.0);

    search_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.search.current-folder"));
    (*search_box_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*search_box_).set_requested_bounds({0, 0, 228, 30});
    (*search_box_).set_placeholder_text("Search this subtree");
    (*search_box_).set_accessible_name("Search current subtree");
    (*form_.file_manager_app_shell_location_search_host).clear_children();
    (*form_.file_manager_app_shell_location_search_host).add_child(search_box_);
    (*form_.file_manager_app_shell_location_search_host).set_flex_grow(*search_box_, 1.0);

    tree_root_mode_button_ = std::make_shared<gui_forms::DropDownButton>(
        gui_forms::StableId("fm.navigation.root-mode"), "Home",
        gui_forms::DropDownButtonMode::menu);
    (*tree_root_mode_button_).set_visual_style(gui_forms::ButtonVisualStyle::flat);
    (*tree_root_mode_button_).set_flat_border_width(0.0);
    (*tree_root_mode_button_).set_style(tree_mode_style());
    (*tree_root_mode_button_).set_drop_down_width(14.0);
    (*tree_root_mode_button_).set_font({gui_forms::FontRole::control, 12.0, 400, false});
    (*tree_root_mode_button_).set_requested_bounds({0, 0, 112, 27});
    (*tree_root_mode_button_).set_minimum_size({112, 27});
    (*tree_root_mode_button_).set_maximum_size({112, 27});
    (*tree_root_mode_button_).set_accessible_name("Folder tree root mode");
    (*tree_root_mode_button_).set_accessible_description(
        "Switch immediately between the honest Home and Volumes trees");
    (*tree_root_mode_button_).set_dock(gui_forms::DockStyle::fill);
    (*tree_root_mode_button_).set_margin({});
    (*form_.file_manager_app_shell_workspace_sidebar_header_root_mode_host).clear_children();
    (*form_.file_manager_app_shell_workspace_sidebar_header_root_mode_host).add_child(tree_root_mode_button_);
    (*form_.file_manager_app_shell_workspace_sidebar_header_root_mode_host).set_flex_grow(*tree_root_mode_button_, 1.0);
    (*form_.file_manager_app_shell_workspace_sidebar).set_flex_grow(
        *form_.file_manager_app_shell_workspace_sidebar_tree_host, 1.0);

    tree_ = std::make_shared<gui_forms::TreeView>(
        gui_forms::StableId("fm.navigation.tree"));
    (*tree_).set_requested_bounds({0, 0, 178, 340});
    (*tree_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*tree_).set_item_height(26.0);
    (*tree_).set_accessible_name("Folders in the selected honest root mode");
    (*form_.file_manager_app_shell_workspace_sidebar_tree_host).clear_children();
    (*form_.file_manager_app_shell_workspace_sidebar_tree_host).set_flow_direction(
        gui_forms::FlowDirection::top_down);
    (*form_.file_manager_app_shell_workspace_sidebar_tree_host).add_child(tree_);
    (*form_.file_manager_app_shell_workspace_sidebar_tree_host).set_flex_grow(*tree_, 1.0);

    objects_ = std::make_shared<gui_forms::ObjectView>(
        gui_forms::StableId("fm.objects.current-folder"));
    (*objects_).set_requested_bounds({0, 0, 736, 455});
    (*objects_).set_view_mode(gui_forms::ObjectViewMode::icons);
    (*objects_).set_icon_cell_size({104, 86});
    (*objects_).set_details_row_height(28.0);
    (*objects_).set_show_secondary_text(false);
    (*objects_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*objects_).set_selection_mode(gui_forms::ObjectSelectionMode::multiple);
    (*objects_).set_accessible_name("Objects in current folder");

    std::shared_ptr<CriteriaConsolePanel> criteria_console = std::make_shared<CriteriaConsolePanel>();
    criteria_console_ = criteria_console;
    criteria_title_ = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("fm.criteria.title"),
        "CURRENT SUBTREE   ·   EXACT VIRTUAL FOLDER   ·   COMMITTED CATALOGUE");
    (*criteria_title_).set_font(
        {gui_forms::FontRole::control, 8.5, 700, false, 0.32});
    (*criteria_title_).set_accessible_name("Criteria virtual folder derivation");
    criteria_rack_ = std::make_shared<gui_forms::InstrumentRack>(
        gui_forms::StableId("fm.criteria.rack"));
    (*criteria_rack_).set_accessible_name("Exact metadata predicate rack");
    (*criteria_rack_).set_accessible_description(
        "Committed intrinsic metadata filters over one Engine catalogue generation");
    (*criteria_rack_).set_modules(
        {kind_criteria_module(), modified_criteria_module()});

    std::shared_ptr<gui_forms::TableLayoutPanel> criteria_actions = std::make_shared<gui_forms::TableLayoutPanel>(
        gui_forms::StableId("fm.criteria.actions"));
    (*criteria_actions).set_column_count(1);
    (*criteria_actions).set_row_count(2);
    (*criteria_actions).set_column_style(
        0, {gui_forms::TableSizeMode::percent, 100.0});
    (*criteria_actions).set_row_style(
        0, {gui_forms::TableSizeMode::percent, 100.0});
    (*criteria_actions).set_row_style(
        1, {gui_forms::TableSizeMode::absolute, 28.0});
    (*criteria_actions).set_grow_style(
        gui_forms::TableLayoutGrowStyle::fixed_size);
    criteria_action_state_ = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("fm.criteria.actions.state"),
        "2 exact filters · catalogue only");
    (*criteria_action_state_).set_font(
        {gui_forms::FontRole::control, 8.0, 600, false});
    (*criteria_action_state_).set_margin({5.0, 2.0, 5.0, 1.0});
    (*criteria_action_state_).set_dock(gui_forms::DockStyle::fill);
    (*criteria_actions).add_child(criteria_action_state_);
    (*criteria_actions).set_cell_position(*criteria_action_state_, {0, 0});
    criteria_add_button_ = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("fm.criteria.add"), "+ module");
    (*criteria_add_button_).set_visual_style(gui_forms::ButtonVisualStyle::standard);
    (*criteria_add_button_).set_accessible_name("Add an admitted criterion module");
    (*criteria_add_button_).set_margin({3.0, 1.0, 3.0, 1.0});
    (*criteria_add_button_).set_dock(gui_forms::DockStyle::fill);
    (*criteria_actions).add_child(criteria_add_button_);
    (*criteria_actions).set_cell_position(*criteria_add_button_, {0, 1});
    (*criteria_rack_).set_action_content(criteria_actions, 154.0);
    (*criteria_console).title = criteria_title_;
    (*criteria_console).rack = criteria_rack_;
    (*criteria_console).add_child(criteria_title_);
    (*criteria_console).add_child(criteria_rack_);
    (*criteria_console_).set_dock(gui_forms::DockStyle::top);
    (*criteria_console_).set_visible(false);

    correspondence_ = std::make_shared<gui_forms::CorrespondenceView>(
        gui_forms::StableId("fm.search.correspondence"));
    (*correspondence_).set_compact_height(45.0);
    (*correspondence_).set_expanded_height(116.0);
    (*correspondence_).set_accessible_name("Factual local search results");
    (*correspondence_).set_visible(false);

    content_surface_ = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("fm.content.surface"));
    (*content_surface_).set_background(gui_forms::Color::rgba(255, 255, 255));
    (*content_surface_).set_dock(gui_forms::DockStyle::fill);
    (*objects_).set_dock(gui_forms::DockStyle::fill);
    (*correspondence_).set_dock(gui_forms::DockStyle::fill);
    (*content_surface_).add_child(criteria_console_);
    (*content_surface_).add_child(objects_);
    (*content_surface_).add_child(correspondence_);
    (*form_.file_manager_app_shell_workspace_selection_content_objects).clear_children();
    (*form_.file_manager_app_shell_workspace_selection_content_objects).add_child(content_surface_);
    (*form_.file_manager_app_shell_workspace_selection_content_objects).set_flex_grow(
        *content_surface_, 1.0);

    preview_picture_ = std::make_shared<gui_forms::PictureBox>(
        gui_forms::StableId("fm.inspector.preview.image"));
    // The authored preview surface paints in the control plane. PictureBox
    // inherits Panel's backplane default, which would put its image beneath
    // that opaque parent surface during the window's ordered plane replay.
    (*preview_picture_).set_paint_plane(gui_forms::PaintPlane::control);
    (*preview_picture_).set_background(gui_forms::Color::rgba(0, 0, 0, 0));
    (*preview_picture_).set_requested_bounds({0, 0, 194, 112});
    (*preview_picture_).set_size_mode(gui_forms::PictureBoxSizeMode::zoom);
    (*preview_picture_).set_accessible_name("Selected image preview");
    (*preview_picture_).set_visible(false);
    preview_text_ = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("fm.inspector.preview.text"));
    (*preview_text_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*preview_text_).set_foreground(gui_forms::Color::rgba(219, 232, 239));
    (*preview_text_).set_requested_bounds({0, 0, 190, 108});
    (*preview_text_).set_text_style_role(gui_forms::TextStyleRole::monospace);
    (*preview_text_).set_text_wrapping(gui_forms::TextWrapping::word);
    (*preview_text_).set_use_mnemonic(false);
    (*preview_text_).set_maximum_lines(7);
    (*preview_text_).set_vertical_alignment(gui_forms::VerticalAlignment::near);
    (*preview_text_).set_accessible_name("Selected text preview");
    (*preview_text_).set_visible(false);
    preview_house_icon_ = std::make_shared<HouseImage>(
        gui_forms::StableId("fm.inspector.preview.house-icon"),
        "Selected object material icon");
    (*preview_house_icon_).set_requested_bounds({0, 0, 72, 72});
    (*preview_house_icon_).set_minimum_size({72, 72});
    (*preview_house_icon_).set_maximum_size({72, 72});
    (*preview_house_icon_).set_image_alignment(
        gui_forms::ContentAlignment::middle_center);
    (*preview_house_icon_).set_text_image_relation(
        gui_forms::TextImageRelation::overlay);
    (*preview_house_icon_).set_content_padding({0, 0, 0, 0});
    (*preview_house_icon_).set_visual_style(gui_forms::ButtonVisualStyle::flat);
    (*preview_house_icon_).set_flat_border_width(0.0);
    (*preview_house_icon_).set_style(transparent_image_style());
    (*preview_house_icon_).set_visible(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).clear_children();
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).add_child(
        preview_picture_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).add_child(
        preview_text_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).add_child(
        preview_house_icon_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).add_child(
        form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).set_minimum_size(
        {0, 260});
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).set_minimum_size(
        {0, 174});

    rename_box_ = std::make_shared<gui_forms::TextBox>(
        gui_forms::StableId("fm.operations.rename"));
    (*rename_box_).set_requested_bounds({0, 0, 200, 30});
    (*rename_box_).set_maximum_length(255);
    (*rename_box_).set_placeholder_text("New name");
    (*rename_box_).set_accessible_name("Rename selected object");
    (*rename_box_).set_visible(false);
    (*form_.file_manager_app_shell_location_path_host).add_child(rename_box_);
    (*form_.file_manager_app_shell_location_path_host).set_flex_grow(
        *rename_box_, 1.0);

    property_list_ = std::make_shared<gui_forms::PropertyList>(
        gui_forms::StableId("fm.selection.properties"));
    (*property_list_).set_font({gui_forms::FontRole::content, 13.0, 400, false});
    (*property_list_).set_row_height(30.0);
    (*property_list_).set_accessible_name("Selection properties");
    (*property_list_).set_label_width(104.0);
    (*property_list_).set_requested_bounds({0, 0, 270, 170});
    (*property_list_).set_groups({
        {"fm.property.group.identity", "IDENTITY", {
            {"fm.property.name", "Name", "—",
             "Rename this exact filesystem object inside the protected mutation scope",
             gui_forms::PropertyEditorKind::text},
            {"fm.property.kind", "Kind", "—", "Selected object kind"},
            {"fm.property.location", "Location", "—", "Exact local path"},
            {"fm.property.size", "Size", "—", "Observed object size"},
            {"fm.property.modified", "Modified", "—", "Filesystem modification time"},
        }},
        {"fm.property.group.verification", "VERIFICATION", {
            {"fm.property.expected-sha256", "Expected SHA-256", "",
             "Optional exact 64-character hexadecimal digest compared with the next completed SHA-256 result",
             gui_forms::PropertyEditorKind::text},
        }},
    });
    if (const std::shared_ptr<gui_forms::TextBox> name_editor = std::dynamic_pointer_cast<gui_forms::TextBox>(
            (*property_list_).editor("fm.property.name"))) {
        (*name_editor).set_maximum_length(255);
        (*name_editor).set_enabled(false);
    }
    expected_checksum_box_ = std::dynamic_pointer_cast<gui_forms::TextBox>(
        (*property_list_).editor("fm.property.expected-sha256"));
    if (!expected_checksum_box_) {
        throw std::logic_error(
            "Selection PropertyList did not create the expected SHA-256 editor");
    }
    (*expected_checksum_box_).set_maximum_length(64);
    (*expected_checksum_box_).set_placeholder_text("64 hexadecimal characters (optional)");
    (*expected_checksum_box_).set_enabled(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts).clear_children();
    (*form_.file_manager_app_shell_workspace_selection_inspector_label).set_dock(
        gui_forms::DockStyle::top);
    (*form_.file_manager_app_shell_workspace_selection_inspector_label).set_minimum_size({0, 27});
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).set_dock(
        gui_forms::DockStyle::top);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).set_requested_bounds(
        {0, 0, 288, 260});
    (*property_list_).set_dock(gui_forms::DockStyle::fill);
    (*property_list_).set_header_content(
        form_.file_manager_app_shell_workspace_selection_inspector_facts_preview, 260.0);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts).add_child(
        property_list_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts).set_flex_grow(
        *property_list_, 1.0);
    (*form_.file_manager_app_shell_workspace_selection_inspector).set_flex_grow(
        *form_.file_manager_app_shell_workspace_selection_inspector_facts, 1.0);

    workspace_split_ = form_.file_manager_app_shell_workspace;
    selection_split_ = form_.file_manager_app_shell_workspace_selection;

    (*form_.file_manager_app_shell_commands_new_folder).set_enabled(
        operations_ != nullptr);
    (*form_.file_manager_app_shell_commands_rename).set_enabled(false);
    (*form_.file_manager_app_shell_commands_selection_group_actions_copy).set_enabled(false);
    (*form_.file_manager_app_shell_commands_move).set_enabled(false);
    (*form_.file_manager_app_shell_commands_paste).set_enabled(false);
    (*form_.file_manager_app_shell_commands_selection_group_actions_delete).set_enabled(false);
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_open).set_enabled(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_enabled(false);
    (*form_.file_manager_app_shell_settings).set_visible(false);
    (*form_.file_manager_app_shell_settings_actions_apply).set_enabled(false);
    (*form_.file_manager_app_shell_settings_actions_cancel).set_enabled(false);
    (*form_.file_manager_app_shell_settings_actions_reset).set_enabled(false);
    (*form_.file_manager_app_shell_settings_body_page_host).set_minimum_size({0, 240});
    (*form_.file_manager_app_shell_settings_body_page_host).set_requested_bounds(
        {0, 0, 760, 330});
    (*form_.file_manager_app_shell_settings_actions).set_wrap_contents(false);
    settings_tab_selected_recipes_ =
        (*form_.file_manager_app_shell_settings_body_tabs_general).visual_recipes_override();
    settings_tab_normal_recipes_ =
        (*form_.file_manager_app_shell_settings_body_tabs_appearance).visual_recipes_override();
    update_settings_tab_state();
    (*form_.file_manager_app_shell_settings_actions_status).set_minimum_size({160, 27});
    gui_forms::Control::Ptr settings_back =
        (*form_.file_manager_app_shell_settings_actions).remove_child(
            (*form_.file_manager_app_shell_settings_actions_back).runtime_id());
    if (!settings_back) {
        throw std::logic_error("Settings Back command is absent from its source host");
    }
    (*form_.file_manager_app_shell_settings_actions_back).set_requested_bounds(
        {0, 0, 100, 27});
    (*form_.file_manager_app_shell_settings_heading).add_child(settings_back);
    for (const gui_forms::Control::Ptr& legacy : {
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
                 form_.file_manager_app_shell_workspace_selection_inspector_facts_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_label),
             std::static_pointer_cast<gui_forms::Control>(
                 form_.file_manager_app_shell_workspace_selection_inspector_facts_commands)}) {
        (*legacy).set_visible(false);
    }
    (*form_.file_manager_app_shell_commands_arrange_group_actions_settings).set_accessible_description(
        "Reveal and focus the factual Selection and Properties pane");
    const std::string mutation_description = operations_
        ? "Available only inside the explicit protected mutation profile"
        : "Read-only launch; use an explicit protected mutation profile";
    (*form_.file_manager_app_shell_commands_new_folder).set_accessible_description(
        mutation_description);
    (*form_.file_manager_app_shell_commands_rename).set_accessible_description(
        mutation_description);
    (*form_.file_manager_app_shell_commands_selection_group_actions_copy).set_accessible_description(
        operations_ ? "Capture one selected object for collision-safe staged copy"
                    : mutation_description);
    (*form_.file_manager_app_shell_commands_move).set_accessible_description(
        operations_ ? "Capture one selected object for same-volume move"
                    : mutation_description);
    (*form_.file_manager_app_shell_commands_paste).set_accessible_description(
        operations_ ? "Publish the captured transfer in the current folder"
                    : mutation_description);
    (*form_.file_manager_app_shell_commands_selection_group_actions_delete).set_accessible_description(
        operations_ ? "Two-step recoverable quarantine; never permanent deletion"
                    : mutation_description);

    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_visible(false);
    rebuild_breadcrumb();
    install_command_surfaces();
}

std::shared_ptr<gui_forms::Command> Application::make_command(
    std::string id, std::string text, std::string description,
    std::function<void()> action) {
    std::shared_ptr<gui_forms::Command> command = std::make_shared<gui_forms::Command>(
        std::move(id), std::move(text));
    (*command).set_description(std::move(description));
    subscriptions_.push_back((*command).invoked().subscribe(
        CommandAction{std::move(action)}));
    commands_.push_back(command);
    return command;
}

void Application::show_menu(
    const std::shared_ptr<gui_forms::ContextMenu>& menu,
    const gui_forms::Control::Ptr& owner) {
    if (!menu || !owner) return;
    const gui_forms::Rect bounds = (*owner).rectangle_to_window((*owner).client_rectangle());
    (*menu).show(owner, {bounds.x, bounds.y + bounds.height});
}

void Application::show_command_shelf_overflow() {
    using gui_forms::MenuItemKind;
    std::vector<gui_forms::MenuItemSpec> items{};
    const gui_forms::CommandOverflowSnapshot projection =
        (*form_.file_manager_app_shell_commands).layout_snapshot();
    const bool selection_collapsed = std::any_of(
        projection.groups.begin(), projection.groups.end(),
        selection_group_collapsed);
    if (selection_collapsed) {
        items.push_back({"move-copy", MenuItemKind::submenu, {}, "Move / copy", {
            {"copy", MenuItemKind::command, command_copy_},
            {"move", MenuItemKind::command, command_move_},
        }});
        items.push_back({"delete", MenuItemKind::command, command_delete_});
        items.push_back({"separator.arrange", MenuItemKind::separator});
    }
    items.push_back({"view", MenuItemKind::submenu, {}, "View", {
        {"small-icons", MenuItemKind::radio, command_icons_},
        {"details", MenuItemKind::radio, command_details_},
        {"criteria", MenuItemKind::check, command_criteria_},
    }});
    items.push_back({"sort", MenuItemKind::submenu, {}, "Sort", {
        {"name", MenuItemKind::radio, command_sort_name_},
        {"kind", MenuItemKind::radio, command_sort_kind_},
        {"size", MenuItemKind::radio, command_sort_size_},
        {"modified", MenuItemKind::radio, command_sort_modified_},
    }});
    items.push_back({"properties", MenuItemKind::command, command_properties_});
    (*shelf_overflow_menu_).set_items(std::move(items));
    show_menu(shelf_overflow_menu_, shelf_overflow_button_);
}

void Application::select_all_objects() { (*objects_).select_all(); }
void Application::navigate_home() { request_navigation(home_root_, true); }
void Application::select_home_tree() { select_tree_root_mode(home_root_); }
void Application::select_volumes_tree() {
    if (volumes_root_) select_tree_root_mode(*volumes_root_);
}
void Application::refresh_view() {
    if (criteria_showing_) request_engine_criteria();
    else if (search_showing_) request_engine_search();
    else request_navigation(location_, false);
}
void Application::toggle_settings() {
    if (settings_open_) hide_settings();
    else show_settings();
}
void Application::close_window() {
    if (request_close_) request_close_();
}

void Application::install_command_surfaces() {
    using gui_forms::MenuItemKind;

    command_open_ = make_command(
        "file.open", "Open", "Open the selected local object",
        std::bind_front(&Application::request_open, this));
    (*command_open_).set_shortcut("Enter");
    (*command_open_).set_default_action(true);
    command_choose_open_ = make_command(
        "file.choose-open", "Open…",
        "Choose a local file in the File Manager document picker",
        std::bind_front(&Application::show_open_picker, this));
    (*command_choose_open_).set_shortcut("Cmd+O");
    command_new_folder_ = make_command(
        "file.new-folder", "New folder",
        "Create a collision-safe folder in the current location",
        std::bind_front(&Application::request_create_folder, this));
    command_copy_ = make_command(
        "selection.copy", "Copy",
        "Stage the selected object for a collision-safe copy",
        std::bind_front(&Application::capture_transfer, this, false));
    command_move_ = make_command(
        "selection.move", "Move",
        "Stage the selected object for a same-volume move",
        std::bind_front(&Application::capture_transfer, this, true));
    command_paste_ = make_command(
        "selection.paste", "Paste",
        "Publish the staged transfer in the current folder",
        std::bind_front(&Application::paste_transfer, this));
    (*command_paste_).set_shortcut("Cmd+V");
    command_cancel_copy_ = make_command(
        "transfer.cancel", "Cancel copy",
        "Request cancellation of the active copy before publication",
        std::bind_front(&Application::cancel_transfer, this));
    command_undo_ = make_command(
        "edit.undo", "Undo last operation",
        "Undo the last recoverable protected operation",
        std::bind_front(&Application::request_undo, this));
    command_delete_ = make_command(
        "selection.delete", "Delete",
        "Move the selected object to the configured recovery quarantine",
        std::bind_front(&Application::request_quarantine, this));
    (*command_delete_).set_shortcut("Delete");
    (*command_delete_).set_destructive(true);
    command_rename_ = make_command(
        "selection.rename", "Rename",
        "Rename the selected object after no-follow identity validation",
        std::bind_front(&Application::begin_rename, this));
    (*command_rename_).set_shortcut("F2");
    command_properties_ = make_command(
        "view.properties", "Properties",
        "Reveal and focus the factual Selection and Properties pane",
        std::bind_front(&Application::show_properties, this));
    command_select_all_ = make_command(
        "edit.select-all", "Select all", "Select all visible folder objects",
        std::bind_front(&Application::select_all_objects, this));
    (*command_select_all_).set_shortcut("Cmd+A");

    command_back_ = make_command(
        "go.back", "Back", "Return to the previous local location",
        std::bind_front(&Application::navigate_back, this));
    (*command_back_).set_shortcut("Alt+Left");
    command_forward_ = make_command(
        "go.forward", "Forward", "Advance to the next local location",
        std::bind_front(&Application::navigate_forward, this));
    (*command_forward_).set_shortcut("Alt+Right");
    command_up_ = make_command(
        "go.up", "Up", "Navigate to the parent inside the current admitted root",
        std::bind_front(&Application::navigate_up, this));
    (*command_up_).set_shortcut("Alt+Up");
    command_root_ = make_command(
        "go.home", "Home", "Return to the user Home folder",
        std::bind_front(&Application::navigate_home, this));
    command_tree_home_ = make_command(
        "tree-root.home", "Home",
        "Show the honest user Home hierarchy in the folder tree",
        std::bind_front(&Application::select_home_tree, this));
    if (volumes_root_) {
        command_tree_volumes_ = make_command(
            "tree-root.volumes", "Volumes",
            "Show the honest mounted Volumes hierarchy in the folder tree",
            std::bind_front(&Application::select_volumes_tree, this));
    }
    for (std::size_t index = 0; index < navigation_roots_.size(); ++index) {
        const std::filesystem::path& root = navigation_roots_[index];
        if (root == home_root_ || (volumes_root_ && root == *volumes_root_)) {
            continue;
        }
        std::shared_ptr<gui_forms::Command> command = make_command(
            "tree-root.admitted." + std::to_string(index),
            leaf_name(root),
            "Show the explicit admitted hierarchy in the folder tree",
            std::bind_front(&Application::select_tree_root_mode, this, root));
        command_tree_admitted_roots_.push_back(std::move(command));
        tree_admitted_root_paths_.push_back(root);
    }

    command_icons_ = make_command(
        "view.small-icons", "Small icons", "Use the compact icon object field",
        std::bind_front(&Application::set_view_mode, this, gui_forms::ObjectViewMode::icons));
    command_details_ = make_command(
        "view.details", "Details", "Use the factual details object field",
        std::bind_front(&Application::set_view_mode, this, gui_forms::ObjectViewMode::details));
    command_criteria_ = make_command(
        "view.criteria", "Criteria",
        "Build an exact metadata virtual folder over the current indexed subtree",
        std::bind_front(&Application::toggle_criteria_mode, this));
    command_sort_name_ = make_command(
        "sort.name", "Name", "Sort objects by name",
        std::bind_front(&Application::set_sort_mode, this, std::string("name")));
    command_sort_kind_ = make_command(
        "sort.kind", "Type", "Sort objects by observed kind",
        std::bind_front(&Application::set_sort_mode, this, std::string("kind")));
    command_sort_size_ = make_command(
        "sort.size", "Size", "Sort objects by observed byte size",
        std::bind_front(&Application::set_sort_mode, this, std::string("size")));
    command_sort_modified_ = make_command(
        "sort.modified", "Modified", "Sort objects by observed modification time",
        std::bind_front(&Application::set_sort_mode, this, std::string("modified")));
    command_refresh_ = make_command(
        "view.refresh", "Refresh", "Refresh the visible local content",
        std::bind_front(&Application::refresh_view, this));
    command_toggle_tree_ = make_command(
        "view.folder-tree", "Folder tree", "Show or collapse the folder tree",
        std::bind_front(&Application::toggle_folder_tree, this));
    command_toggle_selection_ = make_command(
        "view.selection-pane", "Selection and Properties",
        "Show or collapse the factual selection pane",
        std::bind_front(&Application::toggle_selection_pane, this));

    command_checksum_ = make_command(
        "commands.sha256", "SHA-256…",
        "Compute a revision-validated SHA-256 digest for the selected file",
        std::bind_front(&Application::request_checksum, this));
    command_terminal_ = make_command(
        "commands.terminal", "Terminal here",
        "Open Terminal at the selected folder or current location",
        std::bind_front(&Application::request_terminal, this));
    command_copy_path_ = make_command(
        "commands.copy-path", "Copy path",
        "Copy the exact selected or current local path",
        std::bind_front(&Application::copy_current_path, this));
    command_settings_ = make_command(
        "file.settings", "Settings…",
        "Open File Manager settings and Orchestrator service controls",
        std::bind_front(&Application::toggle_settings, this));
    command_close_ = make_command(
        "file.close", "Close File Manager", "Close this File Manager window",
        std::bind_front(&Application::close_window, this));
    std::shared_ptr<gui_forms::Command> about = make_command(
        "help.about", "About File Manager", "Show build and authority information",
        std::bind_front(&Application::show_about, this));

    command_focus_search_ = make_command("go.search", "Search this subtree…",
        "Focus the existing subtree search field", std::bind_front(&Application::focus_search_command, this));
    command_focus_location_ = make_command("go.location", "Enter location…",
        "Edit the exact current folder path", std::bind_front(&Application::focus_location_command, this));
    (*menu_strip_).set_items({
        {"fm.menu.file", "File", {
            {"file.choose-open", MenuItemKind::command, command_choose_open_},
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
            {"edit.cancel-copy", MenuItemKind::command, command_cancel_copy_},
            {"edit.delete", MenuItemKind::command, command_delete_},
            {"edit.separator.undo", MenuItemKind::separator},
            {"edit.undo", MenuItemKind::command, command_undo_},
        }},
        {"fm.menu.view", "View", {
            {"view.icons", MenuItemKind::radio, command_icons_},
            {"view.details", MenuItemKind::radio, command_details_},
            {"view.criteria", MenuItemKind::check, command_criteria_},
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
            {"go.home", MenuItemKind::command, command_root_},
            {"go.location", MenuItemKind::command, command_focus_location_},
            {"go.search", MenuItemKind::command, command_focus_search_},
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
    expanded_menu_items_ = (*menu_strip_).items();
    (*menu_strip_).set_selected_item_id("fm.menu.home");

    move_copy_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.move-copy");
    (*move_copy_menu_).set_items({
        {"copy", MenuItemKind::command, command_copy_},
        {"move", MenuItemKind::command, command_move_},
    });
    view_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.view");
    (*view_menu_).set_items({
        {"small-icons", MenuItemKind::radio, command_icons_},
        {"details", MenuItemKind::radio, command_details_},
        {"criteria", MenuItemKind::check, command_criteria_},
    });
    sort_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.shelf.sort");
    (*sort_menu_).set_items({
        {"name", MenuItemKind::radio, command_sort_name_},
        {"kind", MenuItemKind::radio, command_sort_kind_},
        {"size", MenuItemKind::radio, command_sort_size_},
        {"modified", MenuItemKind::radio, command_sort_modified_},
    });
    shelf_overflow_menu_ = std::make_shared<gui_forms::ContextMenu>(
        "fm.shelf.more-menu");
    bind_menu_open(move_copy_menu_, shelf_move_copy_button_);
    bind_menu_open(view_menu_, shelf_view_button_);
    bind_menu_open(sort_menu_, shelf_sort_button_);
    bind_menu_open(shelf_overflow_menu_, shelf_overflow_button_);
    tree_root_menu_ = std::make_shared<gui_forms::ContextMenu>(
        "fm.navigation.root-mode-menu");
    std::vector<gui_forms::MenuItemSpec> tree_root_items{
        {"home", MenuItemKind::radio, command_tree_home_},
    };
    if (command_tree_volumes_) {
        tree_root_items.push_back(
            {"volumes", MenuItemKind::radio, command_tree_volumes_});
    }
    for (std::size_t index = 0;
         index < command_tree_admitted_roots_.size(); ++index) {
        tree_root_items.push_back({
            "admitted." + std::to_string(index), MenuItemKind::radio,
            command_tree_admitted_roots_[index]});
    }
    (*tree_root_menu_).set_items(std::move(tree_root_items));
    bind_menu_open(tree_root_menu_, tree_root_mode_button_);
    object_menu_ = std::make_shared<gui_forms::ContextMenu>("fm.context.object");
    (*object_menu_).set_items({
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
    (*background_menu_).set_items({
        {"new-folder", MenuItemKind::command, command_new_folder_},
        {"paste", MenuItemKind::command, command_paste_},
        {"separator.commands", MenuItemKind::separator},
        {"terminal", MenuItemKind::command, command_terminal_},
        {"copy-path", MenuItemKind::command, command_copy_path_},
        {"separator.properties", MenuItemKind::separator},
        {"properties", MenuItemKind::command, command_properties_},
    });
    breadcrumb_overflow_menu_ = std::make_shared<gui_forms::ContextMenu>(
        "fm.path.overflow-menu");
    menus_ = {move_copy_menu_, view_menu_, sort_menu_, shelf_overflow_menu_,
              tree_root_menu_,
              object_menu_, background_menu_, breadcrumb_overflow_menu_};
    update_command_state();
}

void Application::bind_menu_open(const std::shared_ptr<gui_forms::ContextMenu>& menu,
                                  const std::shared_ptr<gui_forms::DropDownButton>& button) {
    gui_forms::SubscriptionToken connection = (*menu).open_changed().subscribe(MenuOpenProjection{button});
    subscriptions_.push_back(std::move(connection));
}

void Application::bind_command_accelerator(const std::shared_ptr<gui_forms::Command>& command,
                                          const gui_forms::KeyGesture gesture,
                                          const bool before_focused_route,
                                          std::function<bool()> predicate) {
    const std::string source = "fm.accelerator." + (*command).stable_id();
    CommandAccelerator action{command, source, std::move(predicate)};
    gui_forms::AcceleratorToken connection = (*window_).register_accelerator(
        *command, gesture, std::move(action), gui_forms::AcceleratorOptions{before_focused_route});
    accelerator_tokens_.push_back(std::move(connection));
}

bool Application::focused_is_object_surface() const {
    if (!window_) return false;
    const gui_forms::Control::Ptr focused = (*window_).focused_control();
    const bool matches = focused == objects_ || focused == correspondence_;
    return matches;
}

bool Application::focused_is_navigation_surface() const {
    if (!window_) return false;
    const gui_forms::Control::Ptr focused = (*window_).focused_control();
    const bool matches = focused == objects_ || focused == correspondence_ || focused == tree_;
    return matches;
}

bool Application::focused_is_not_text_editor() const {
    if (!window_) return false;
    const gui_forms::Control::Ptr focused = (*window_).focused_control();
    const std::shared_ptr<gui_forms::TextBox> editor = std::dynamic_pointer_cast<gui_forms::TextBox>(focused);
    const bool allowed = !editor;
    return allowed;
}

bool Application::focused_is_objects() const {
    if (!window_) return false;
    const bool matches = (*window_).focused_control() == objects_;
    return matches;
}

void Application::bind_navigation_accelerator(const std::shared_ptr<gui_forms::Command>& command,
                                             const std::uint32_t key) {
    bind_command_accelerator(command, {key, gui_forms::Modifier::alt}, true,
        std::bind_front(&Application::focused_is_navigation_surface, this));
    bind_command_accelerator(command, {key, gui_forms::Modifier::alt}, false,
        std::bind_front(&Application::focused_is_not_text_editor, this));
}

void Application::install_accelerators() {
    if (!window_) return;
    accelerator_tokens_.clear();

#if defined(__APPLE__)
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::meta;
    constexpr std::string_view primary_text = "Cmd";
#else
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::control;
    constexpr std::string_view primary_text = "Ctrl";
#endif

    (*command_paste_).set_shortcut(std::string(primary_text) + "+V");
    (*command_select_all_).set_shortcut(std::string(primary_text) + "+A");

    bind_command_accelerator(command_choose_open_, {gui_forms::PhysicalKey::o, primary}, true);
    bind_command_accelerator(command_open_, {gui_forms::PhysicalKey::enter,
                         gui_forms::Modifier::none}, false,
         std::bind_front(&Application::focused_is_object_surface, this));
    bind_command_accelerator(command_paste_, {gui_forms::PhysicalKey::v, primary}, false,
         std::bind_front(&Application::focused_is_not_text_editor, this));
#if defined(__APPLE__)
    bind_command_accelerator(command_delete_, {gui_forms::PhysicalKey::backspace,
                           gui_forms::Modifier::none}, false,
         std::bind_front(&Application::focused_is_not_text_editor, this));
#endif
    bind_command_accelerator(command_delete_, {gui_forms::PhysicalKey::delete_forward,
                           gui_forms::Modifier::none}, false,
         std::bind_front(&Application::focused_is_not_text_editor, this));
    bind_command_accelerator(command_rename_, {gui_forms::PhysicalKey::f2,
                           gui_forms::Modifier::none}, false,
         std::bind_front(&Application::focused_is_not_text_editor, this));
    bind_command_accelerator(command_select_all_, {gui_forms::PhysicalKey::a, primary}, true,
         std::bind_front(&Application::focused_is_objects, this));

    bind_navigation_accelerator(command_back_, gui_forms::PhysicalKey::left);
    bind_navigation_accelerator(command_forward_, gui_forms::PhysicalKey::right);
    bind_navigation_accelerator(command_up_, gui_forms::PhysicalKey::up);
    accelerator_tokens_.push_back((*window_).register_accelerator(
        *command_focus_location_, {gui_forms::PhysicalKey::l, primary},
        std::bind_front(&Application::focus_location_accelerator, this)));
    (*command_focus_location_).set_shortcut(std::string(primary_text) + "+L");
    (*command_focus_search_).set_shortcut(std::string(primary_text) + "+F");
    accelerator_tokens_.push_back((*window_).register_accelerator(
        *command_focus_search_, {gui_forms::PhysicalKey::f, primary},
        std::bind_front(&Application::focus_search_accelerator, this)));
    (*command_refresh_).set_shortcut("F5");
    accelerator_tokens_.push_back((*window_).register_accelerator(
        *command_refresh_, {gui_forms::PhysicalKey::f5, gui_forms::Modifier::none},
        std::bind_front(&Application::refresh_accelerator, this)));
}

bool Application::focus_location_accelerator() {
    if (!window_ || settings_open_ || (*rename_box_).visible()) return false;
    const std::shared_ptr<gui_forms::Control> focused = (*window_).focused_control();
    const std::shared_ptr<gui_forms::TextBox> editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>(focused);
    if (editor && editor != path_box_ && editor != search_box_) return false;
    compact_search_active_ = false;
    update_adaptive_layout((*form_.file_manager_app_shell).committed_arranged_bounds());
    if ((*breadcrumb_).editing()) {
        (*path_box_).select_all();
        const bool focused = (*window_).request_focus(path_box_);
        return focused;
    }
    set_path_editing(true);
    return true;
}

bool Application::refresh_accelerator() {
    if (!window_ || settings_open_ || (*rename_box_).visible()) return false;
    if (std::dynamic_pointer_cast<gui_forms::TextBox>((*window_).focused_control())) return false;
    const bool executed = (*command_refresh_).execute("fm.accelerator.view.refresh");
    return executed;
}

void Application::rebuild_breadcrumb() {
    breadcrumb_paths_.clear();
    breadcrumb_overflow_commands_.clear();
    if (breadcrumb_overflow_menu_) (*breadcrumb_overflow_menu_).set_items({});

    std::vector<BreadcrumbPath> segments{};
    segments.emplace_back(navigation_label(navigation_root_), navigation_root_);
    const std::filesystem::path relative = location_.lexically_relative(navigation_root_);
    std::filesystem::path accumulated = navigation_root_;
    if (!relative.empty() && relative != ".") {
        for (const std::filesystem::path& component : relative) {
            accumulated /= component;
            segments.emplace_back(path_utf8(component), accumulated);
        }
    }

    std::vector<gui_forms::BreadcrumbSegment> model{};
    model.reserve(segments.size());
    for (std::size_t index = 0; index < segments.size(); ++index) {
        const BreadcrumbPath& segment = segments[index];
        const std::string& text = segment.text;
        const std::filesystem::path& path = segment.path;
        const std::string stable_id = breadcrumb_stable_id(path);
        breadcrumb_paths_.insert_or_assign(stable_id, path);
        model.push_back({
            stable_id, text,
            index + 1U == segments.size()
                ? "Current exact location " + path_utf8(path)
                : "Navigate to " + path_utf8(path),
            true});
    }
    (*breadcrumb_).set_segments(std::move(model));
    (*path_box_).set_text(path_utf8(location_));
    set_path_editing(false);
}

void Application::show_breadcrumb_overflow() {
    breadcrumb_subscriptions_.clear();
    breadcrumb_overflow_commands_.clear();
    std::vector<gui_forms::MenuItemSpec> items{};
    for (const std::string& id : (*breadcrumb_).hidden_segment_ids()) {
        const PathMap::const_iterator path = breadcrumb_paths_.find(id);
        const std::span<const gui_forms::BreadcrumbSegment>::iterator segment = std::find_if(
            (*breadcrumb_).segments().begin(), (*breadcrumb_).segments().end(),
            BreadcrumbId{id});
        if (path == breadcrumb_paths_.end() ||
            segment == (*breadcrumb_).segments().end()) {
            continue;
        }
        std::shared_ptr<gui_forms::Command> command = std::make_shared<gui_forms::Command>(
            id + ".navigate", (*segment).text);
        (*command).set_description("Navigate to " + path_utf8((*path).second));
        breadcrumb_subscriptions_.push_back((*command).invoked().subscribe(
            std::bind_front(&Application::navigate_breadcrumb, this, (*path).second)));
        items.push_back({id, gui_forms::MenuItemKind::command, command});
        breadcrumb_overflow_commands_.push_back(std::move(command));
    }
    if (items.empty()) return;
    (*breadcrumb_overflow_menu_).set_items(std::move(items));
    show_menu(breadcrumb_overflow_menu_, breadcrumb_);
}

void Application::set_path_editing(const bool editing) {
    if (rename_box_) (*rename_box_).set_visible(false);
    (*breadcrumb_).set_visible(true);
    if (!editing) {
        path_suggestion_generation_.fetch_add(1);
        close_path_suggestion_popup();
        (*breadcrumb_).set_editing(false);
        if (window_ && (*window_).focused_control() == path_box_) {
            static_cast<void>((*window_).request_focus(breadcrumb_));
        }
        return;
    }
    const std::string location_text = path_utf8(location_);
    (*breadcrumb_).begin_edit(location_text);
}

void Application::request_path_suggestions(std::string text) {
    if (!(*breadcrumb_).editing()) return;
    const std::uint64_t generation =
        path_suggestion_generation_.fetch_add(1) + 1U;
    (*breadcrumb_).set_tab_completion_available(false);
    close_path_suggestion_popup();
    if (text.empty()) {
        apply_path_suggestions(generation, {}, {}, {},
                               "Enter a path in Home or Volumes");
        return;
    }

    std::filesystem::path expanded = path_from_utf8(text);
    if (text == "~") expanded = home_root_;
    else if (text.starts_with("~/")) expanded = home_root_ / path_from_utf8(text.substr(2));
    else if (!expanded.is_absolute()) expanded = location_ / expanded;
    expanded = expanded.lexically_normal();

    const std::string requested_text = text;
    apply_path_suggestions(
        generation, requested_text,
        "Checking: " + path_utf8(expanded), {},
        "Reading matching local folders…");
    enumerate_path_suggestions(
        generation, std::move(text), navigation_roots_, location_, home_root_,
        std::move(expanded));
}

void Application::enumerate_path_suggestions(
    const std::uint64_t generation, std::string requested_text,
    std::vector<std::filesystem::path> admitted_roots,
    std::filesystem::path current_location,
    std::filesystem::path home_root,
    std::filesystem::path candidate) {
    post_worker(SuggestionWork{shared_from_this(), generation, std::move(requested_text), std::move(admitted_roots), std::move(current_location), std::move(home_root), std::move(candidate), show_hidden_});
}

void Application::apply_path_suggestions(
    const std::uint64_t generation, std::string requested_text,
    std::string preview, std::vector<std::filesystem::path> paths,
    std::string notice) {
    if (generation != path_suggestion_generation_.load() ||
        !(*breadcrumb_).editing() || (*path_box_).text() != requested_text) {
        return;
    }
    close_path_suggestion_popup();
    path_suggestion_paths_ = std::move(paths);
    path_suggestion_subscriptions_.clear();

    path_suggestion_content_ = std::make_shared<gui_forms::Panel>(
        gui_forms::StableId("fm.path.suggestions.content"));
    (*path_suggestion_content_).set_background(gui_forms::Color::rgba(247, 249, 244));
    (*path_suggestion_content_).set_border_style(gui_forms::BorderStyle::line);

    path_resolution_preview_ = std::make_shared<gui_forms::Label>(
        gui_forms::StableId("fm.path.resolution-preview"),
        preview.empty() ? notice : preview +
            (notice.empty() ? std::string{} : " · " + notice));
    (*path_resolution_preview_).set_requested_bounds({0, 0, 420, 28});
    (*path_resolution_preview_).set_padding({8, 0, 8, 0});
    (*path_resolution_preview_).set_dock(gui_forms::DockStyle::top);
    (*path_resolution_preview_).set_accessible_name(
        "Canonical path resolution preview");
    (*path_suggestion_content_).add_child(path_resolution_preview_);

    if (!path_suggestion_paths_.empty()) {
        std::vector<std::string> labels{};
        std::vector<std::string> identities{};
        labels.reserve(path_suggestion_paths_.size());
        identities.reserve(path_suggestion_paths_.size());
        for (const std::filesystem::path& path : path_suggestion_paths_) {
            labels.push_back(path_utf8(path.filename()));
            identities.push_back(breadcrumb_stable_id(path) + ".suggestion");
        }
        path_suggestion_list_ = std::make_shared<gui_forms::ListBox>(
            gui_forms::StableId("fm.path.suggestions.list"));
        (*path_suggestion_list_).set_focusable(false);
        (*path_suggestion_list_).set_items(std::move(labels));
        (*path_suggestion_list_).set_item_stable_ids(std::move(identities));
        (*path_suggestion_list_).set_item_height(24.0);
        (*path_suggestion_list_).set_requested_bounds(
            {0, 0, 420,
             4.0 + 24.0 * static_cast<double>(
                 std::min<std::size_t>(path_suggestion_paths_.size(), 6U))});
        (*path_suggestion_list_).set_dock(gui_forms::DockStyle::fill);
        (*path_suggestion_list_).select_index(0U);
        (*path_suggestion_content_).add_child(path_suggestion_list_);
        path_suggestion_subscriptions_.push_back(
            (*path_suggestion_list_).item_activated().subscribe(
                std::bind_front(&Application::on_path_suggestion_activated, this)));
    } else {
        path_suggestion_list_.reset();
    }
    (*breadcrumb_).set_tab_completion_available(
        !path_suggestion_paths_.empty());
    show_path_suggestion_popup();
}

void Application::show_path_suggestion_popup() {
    if (!window_ || !(*breadcrumb_).editing() || !path_suggestion_content_) return;
    gui_forms::AnchoredPopupPlacement placement{};
    placement.preferred_size = {
        std::clamp((*breadcrumb_).absolute_bounds().width, 280.0, 620.0),
        28.0 + (path_suggestion_list_
            ? (*path_suggestion_list_).requested_bounds().height : 0.0)};
    placement.gap = 2.0;
    path_suggestion_layer_ = gui_forms::make_control<gui_forms::AnchoredPopupLayer>(
        gui_forms::StableId("fm.path.suggestions.layer"), breadcrumb_, placement);
    (*path_suggestion_layer_).set_content(path_suggestion_content_);
    (*path_suggestion_layer_).set_accessible_name("Path completion suggestions");
    (*path_suggestion_layer_).set_requested_bounds(
        {0, 0, (*window_).client_size().width, (*window_).client_size().height});
    path_suggestion_subscriptions_.push_back(
        (*path_suggestion_layer_).dismiss_requested().subscribe(
            std::bind_front(&Application::on_path_suggestion_dismissed, this)));
    path_suggestion_popup_ = (*window_).open_popup(
        breadcrumb_, path_suggestion_layer_);
}

void Application::close_path_suggestion_popup(const bool restore_focus) {
    path_suggestion_popup_.disconnect();
    path_suggestion_subscriptions_.clear();
    path_suggestion_layer_.reset();
    path_suggestion_content_.reset();
    path_resolution_preview_.reset();
    path_suggestion_list_.reset();
    if (breadcrumb_) (*breadcrumb_).set_tab_completion_available(false);
    if (restore_focus && (*breadcrumb_).editing() && window_) {
        static_cast<void>((*window_).request_focus(path_box_));
    }
}

void Application::accept_path_suggestion(const std::size_t index) {
    if (index >= path_suggestion_paths_.size() || !(*breadcrumb_).editing()) return;
    (*path_box_).set_text(path_utf8(path_suggestion_paths_[index]));
    (*path_box_).select_all();
    if (window_) static_cast<void>((*window_).request_focus(path_box_));
}

void Application::accept_active_path_suggestion() {
    if (path_suggestion_list_ && (*path_suggestion_list_).selected_index()) {
        accept_path_suggestion(*(*path_suggestion_list_).selected_index());
    }
}

void Application::set_view_mode(const gui_forms::ObjectViewMode mode) {
    details_mode_ = mode == gui_forms::ObjectViewMode::details;
    (*objects_).set_view_mode(mode);
    (*objects_).set_show_secondary_text(details_mode_);
    (*form_.file_manager_app_shell_commands_arrange_group_actions_details).set_text(
        "View");
    (*form_.file_manager_app_shell_commands_arrange_group_actions_details).set_accessible_description(
            details_mode_
                ? "Current presentation: Details. Choose the object presentation"
                : "Current presentation: Small icons. Choose the object presentation");
    update_command_state();
    set_status(details_mode_ ? "Details view" : "Small icons view",
               criteria_showing_
                   ? "exact virtual folder · retained presentation state"
                   : "current local folder · retained presentation state");
}

void Application::sort_object_items(std::vector<gui_forms::ObjectViewItem>& items) const {
    const bool descending = sort_direction_ == gui_forms::ObjectSortDirection::descending;
    if (sort_mode_ == "kind") {
        std::stable_sort(items.begin(), items.end(), detail::ObjectOrder<detail::ObjectSort::kind>{entries_, descending});
    } else if (sort_mode_ == "size") {
        std::stable_sort(items.begin(), items.end(), detail::ObjectOrder<detail::ObjectSort::size>{entries_, descending});
    } else if (sort_mode_ == "modified") {
        std::stable_sort(items.begin(), items.end(), detail::ObjectOrder<detail::ObjectSort::modified>{entries_, descending});
    } else {
        std::stable_sort(items.begin(), items.end(), detail::ObjectOrder<detail::ObjectSort::name>{entries_, descending});
    }
}

void Application::publish_object_items(std::vector<gui_forms::ObjectViewItem> items) {
    for (gui_forms::ObjectViewItem& item : items) {
        const EntryMap::const_iterator found = entries_.find(item.stable_id);
        if (found == entries_.end()) throw std::logic_error("Object row has no observation");
        const DirectoryEntry& entry = (*found).second;
        const std::string type_text = kind_text(entry);
        item.cells = detail::details_cells(entry, item.name, type_text);
    }
    sort_object_items(items);
    std::vector<gui_forms::ObjectDetailsColumn> columns(
        (*objects_).details_columns().begin(), (*objects_).details_columns().end());
    if (columns.empty()) columns = detail::default_details_columns();
    gui_forms::ObjectDetailsSort accepted{{sort_mode_}, sort_direction_};
    (*objects_).set_details_model(std::move(columns), std::move(items), std::move(accepted));
    fit_details_columns((*objects_).committed_arranged_bounds());
}

void Application::on_details_presentation_changed(const gui_forms::PresentationSettings&) {
    fit_details_columns((*objects_).committed_arranged_bounds());
}

void Application::fit_details_columns(const gui_forms::Rect& bounds) {
    if (details_widths_owned_by_user_ || bounds.width <= 8.0) return;
    const std::span<const gui_forms::ObjectDetailsColumn> columns = (*objects_).details_columns();
    constexpr std::array<std::string_view, 4> identities{"name", "kind", "size", "modified"};
    constexpr std::array<double, 4> minimum{120.0, 90.0, 72.0, 132.0};
    constexpr std::array<double, 4> preferred{260.0, 120.0, 104.0, 172.0};
    if (columns.size() != identities.size()) return;
    for (std::size_t index = 0U; index < identities.size(); ++index) {
        if (columns[index].id.value != identities[index]) return;
        if (automatic_details_widths_ && columns[index].width != (*automatic_details_widths_)[index]) {
            details_widths_owned_by_user_ = true;
            return;
        }
    }
    const double available = (bounds.width - 8.0) / (*objects_).effective_text_scale();
    const double fraction = std::clamp((available - 414.0) / 242.0, 0.0, 1.0);
    std::array<double, 4> widths{};
    for (std::size_t index = 0U; index < widths.size(); ++index) {
        widths[index] = minimum[index] + fraction * (preferred[index] - minimum[index]);
    }
    widths[0] = std::min(4096.0, widths[0] + std::max(0.0, available - 656.0));
    // Width setters invalidate retained paint; they do not publish callbacks or
    // replace columns, so the span remains valid throughout this bounded loop.
    try {
        for (std::size_t index = 0U; index < widths.size(); ++index) {
            (*objects_).set_details_column_width(columns[index].id, widths[index]);
        }
    } catch (...) {
        // Retained damage allocation can fail after a setter commits its width.
        // Keep that actual automatic state so a retry does not mistake it for
        // manual resizing. Reading these four fields and assigning the array
        // allocate nothing and cannot replace the original exception.
        for (std::size_t index = 0U; index < widths.size(); ++index) {
            widths[index] = columns[index].width;
        }
        automatic_details_widths_ = widths;
        throw;
    }
    automatic_details_widths_ = widths;
}

void Application::apply_object_sort(std::string mode, const gui_forms::ObjectSortDirection direction) {
    std::vector<gui_forms::ObjectViewItem> items(
        (*objects_).items().begin(), (*objects_).items().end());
    std::vector<gui_forms::ObjectDetailsColumn> columns(
        (*objects_).details_columns().begin(), (*objects_).details_columns().end());
    if (columns.empty()) columns = detail::default_details_columns();
    const gui_forms::ObjectSortDirection previous_direction = sort_direction_;
    // Swap retains the old intent without allocating during failure recovery.
    sort_mode_.swap(mode);
    sort_direction_ = direction;
    try {
        sort_object_items(items);
        gui_forms::ObjectDetailsSort accepted{{sort_mode_}, sort_direction_};
        (*objects_).set_details_model(std::move(columns), std::move(items), std::move(accepted));
    } catch (...) {
        // Publication can throw after commit. The provider commits its order
        // and indicator together, so retain new intent only if it was accepted.
        const gui_forms::ObjectDetailsSort& accepted = (*objects_).details_sort();
        if (accepted.column.value != sort_mode_ || accepted.direction != sort_direction_) {
            sort_mode_.swap(mode);
            sort_direction_ = previous_direction;
        }
        throw;
    }
}

void Application::on_objects_sort_requested(const gui_forms::ObjectDetailsSort& request) {
    if (request.column.value != "name" && request.column.value != "kind" &&
        request.column.value != "size" && request.column.value != "modified") return;
    if (request.direction != gui_forms::ObjectSortDirection::ascending &&
        request.direction != gui_forms::ObjectSortDirection::descending) return;
    apply_object_sort(request.column.value, request.direction);
    const std::string title(sort_title(sort_mode_));
    (*form_.file_manager_app_shell_commands_arrange_group_actions_refresh).set_text("Sort: " + title);
    update_command_state();
    set_status("Sorted by " + title, sort_direction_ == gui_forms::ObjectSortDirection::ascending
        ? "ascending · folders first · unavailable values last"
        : "descending · folders first · unavailable values last");
}

void Application::set_sort_mode(std::string mode) {
    const std::string title(sort_title(mode));
    const gui_forms::ObjectSortDirection direction = mode == "modified" ? gui_forms::ObjectSortDirection::descending
                                                                       : gui_forms::ObjectSortDirection::ascending;
    apply_object_sort(std::move(mode), direction);
    (*form_.file_manager_app_shell_commands_arrange_group_actions_refresh).set_text(
        "Sort: " + title);
    update_command_state();
    set_status("Sorted by " + title,
               "folders first · exact observed fields · stable name fallback");
}

void Application::show_properties() {
    (*selection_split_).set_second_collapsed(
        false, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    if (window_) (*window_).request_focus(property_list_);
    set_status("Selection and Properties active",
               "factual local projection · no inferred metadata");
}

void Application::toggle_folder_tree() {
    const bool show = (*workspace_split_).first_collapsed();
    (*workspace_split_).set_first_collapsed(
        !show, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    set_status(show ? "Folder tree restored" : "Folder tree collapsed",
               "thin splitter seam remains available");
}

void Application::toggle_selection_pane() {
    const bool show = (*selection_split_).second_collapsed();
    (*selection_split_).set_second_collapsed(
        !show, gui_forms::SplitCollapseOrigin::user);
    update_command_state();
    set_status(show ? "Selection and Properties restored"
                    : "Selection and Properties collapsed",
               "thin splitter seam remains available");
}

void Application::show_about() {
    const std::string version = FILE_MANAGER_VERSION;
    set_status("File Manager · " + version + " development build",
               "local filesystem authority · GUI.Forms + Web.Forms");
    if (show_about_window_) show_about_window_();
}

void Application::show_open_picker() {
    set_status("Choose a file", "File Manager document picker · current root authority");
    if (show_open_picker_) show_open_picker_(location_);
}

void Application::bind_secondary_surfaces(
    std::function<void(const std::filesystem::path&)> show_open_picker,
    std::function<void()> show_about_window) {
    show_open_picker_ = std::move(show_open_picker);
    show_about_window_ = std::move(show_about_window);
}

void Application::document_picker_completed(
    const DocumentPickerResult& result) {
    if (!result.accepted() || result.selections.empty()) {
        if (result.terminal == DocumentPickerTerminal::cancelled) {
            set_status("Open cancelled", "No filesystem selection changed");
        }
        return;
    }
    const DocumentSelectionObservation& selection = result.selections.front();
    if (!selection.existing || !selection.identity.available()) {
        set_status("Open refused", "Picker selection no longer has an exact identity");
        return;
    }
    pending_selection_identity_ = selection.identity;
    set_status("Opening " + path_utf8(selection.path.filename()),
               "picker result revalidated · revealing in current location surface");
    request_navigation(selection.path.parent_path(), true);
}

void Application::update_command_state() {
    if (!command_open_) return;
    const bool files_active = !settings_open_;
    const bool correspondence_active = search_showing_ ||
        (*correspondence_).visible();
    const bool folder_presentation_active = files_active && (*objects_).visible() &&
        !(*correspondence_).visible();
    const bool one = (*objects_).selected_ids().size() == 1U;
    const std::optional<DirectoryEntry> entry = selected_entry();
    const bool mutation_ready = files_active && mutation_scope_active() &&
        !(*rename_box_).visible() &&
        !transfer_in_flight_ && !property_rename_in_flight_;
    const bool paste_ready = mutation_ready && pending_transfer_ &&
        (*pending_transfer_).entry.path.parent_path() != location_;
    const std::string hidden_workspace_reason =
        "Return to Files before using file workspace commands";
    const std::string mutation_authority_reason = !files_active
        ? hidden_workspace_reason
        : !operations_
        ? "This launch is read-only; protected mutations were not enabled"
        : !mutation_scope_active()
            ? "Mutations are available only inside the explicitly protected root"
            : (*rename_box_).visible()
                ? "Finish or cancel the active rename first"
                : transfer_in_flight_ || property_rename_in_flight_
                    ? "Wait for the active protected operation"
                    : std::string{};

    (*command_open_).set_enabled(files_active && entry &&
                               (*entry).kind != EntryKind::symlink);
    (*command_choose_open_).set_enabled(files_active);
    (*command_choose_open_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
    (*command_open_).set_availability_reason(!files_active
        ? hidden_workspace_reason : !entry
            ? "Select one visible object" : (*entry).kind == EntryKind::symlink
                ? "Symbolic-link activation is not admitted" : std::string{});
    (*command_new_folder_).set_enabled(mutation_ready);
    (*command_copy_).set_enabled(mutation_ready && one);
    (*command_move_).set_enabled(mutation_ready && one);
    (*command_paste_).set_enabled(paste_ready);
    const bool copy_active = transfer_in_flight_ && transfer_cancellable_;
    const bool copy_cancel_requested = copy_active &&
        cancelled_transfer_generation_.load() == transfer_generation_.load();
    const bool copy_cancel_ready = copy_active && !copy_cancel_requested;
    const std::string copy_cancel_text = copy_cancel_requested ? "Stopping…" : "Cancel copy";
    (*command_cancel_copy_).set_enabled(copy_cancel_ready);
    (*command_cancel_copy_).set_text(copy_cancel_text);
    (*command_cancel_copy_).set_availability_reason(copy_cancel_ready ? std::string{}
        : copy_cancel_requested ? "Waiting for the copy worker to finish cancellation"
                                : "No cancellable copy is active");
    (*form_.file_manager_app_shell_status_cancel_copy).set_visible(copy_active);
    (*form_.file_manager_app_shell_status_cancel_copy).set_enabled(copy_cancel_ready);
    (*form_.file_manager_app_shell_status_cancel_copy).set_text(copy_cancel_text);
    (*command_undo_).set_enabled(files_active && operations_ &&
                               undo_available_);
    (*command_delete_).set_enabled(mutation_ready && one);
    (*command_rename_).set_enabled(mutation_ready && one);
    (*command_new_folder_).set_availability_reason((*command_new_folder_).state().enabled
        ? std::string{} : mutation_authority_reason);
    const std::string selection_mutation_reason =
        !mutation_authority_reason.empty()
            ? mutation_authority_reason : "Select one object";
    for (const std::shared_ptr<gui_forms::Command>& command : {command_copy_, command_move_, command_delete_,
                                command_rename_}) {
        (*command).set_availability_reason((*command).state().enabled
            ? std::string{} : selection_mutation_reason);
    }
    (*command_paste_).set_availability_reason((*command_paste_).state().enabled
        ? std::string{} : !mutation_authority_reason.empty()
            ? mutation_authority_reason : !pending_transfer_
                ? "Copy or move one object first"
                : "Navigate to a different destination folder");
    (*command_undo_).set_availability_reason((*command_undo_).state().enabled
        ? std::string{} : !files_active
            ? hidden_workspace_reason : !operations_
                ? "This launch is read-only; protected mutations were not enabled"
                : "No recoverable operation is available to undo");
    (*command_properties_).set_enabled(files_active);
    (*command_properties_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
    (*command_select_all_).set_enabled(
        folder_presentation_active && !(*objects_).items().empty());
    (*command_select_all_).set_availability_reason(!files_active
        ? hidden_workspace_reason : correspondence_active
            ? "Search results support one factual selection at a time"
            : (*objects_).items().empty()
                ? "The current folder has no visible objects" : std::string{});

    (*command_back_).set_enabled(files_active && !history_.empty() &&
                               history_index_ > 0U);
    (*command_forward_).set_enabled(
        files_active && !history_.empty() &&
        history_index_ + 1U < history_.size());
    (*command_up_).set_enabled(files_active && location_ != navigation_root_);
    (*command_root_).set_enabled(files_active && location_ != home_root_);
    (*command_back_).set_availability_reason((*command_back_).state().enabled
        ? std::string{} : !files_active ? hidden_workspace_reason
                                        : "No previous location");
    (*command_forward_).set_availability_reason((*command_forward_).state().enabled
        ? std::string{} : !files_active ? hidden_workspace_reason
                                        : "No forward location");
    (*command_up_).set_availability_reason((*command_up_).state().enabled
        ? std::string{} : !files_active ? hidden_workspace_reason
                                        : "Already at the admitted root");
    (*command_root_).set_availability_reason((*command_root_).state().enabled
        ? std::string{} : !files_active ? hidden_workspace_reason
                                        : "Already at Home");
    for (const std::shared_ptr<gui_forms::Command>& command : {command_icons_, command_details_,
                                command_sort_name_, command_sort_kind_,
                                command_sort_size_, command_sort_modified_}) {
        (*command).set_enabled(folder_presentation_active);
        (*command).set_availability_reason(folder_presentation_active
            ? std::string{} : !files_active
                ? hidden_workspace_reason
                : "Folder presentation is unavailable for correspondence results");
    }
    (*command_icons_).set_checked(!details_mode_);
    (*command_details_).set_checked(details_mode_);
    (*command_criteria_).set_enabled(files_active && engine_search_available());
    (*command_criteria_).set_checked(criteria_showing_);
    (*command_criteria_).set_availability_reason((*command_criteria_).state().enabled
        ? std::string{} : !files_active ? hidden_workspace_reason
        : "Criteria requires an admitted Engine catalogue root");
    (*command_sort_name_).set_checked(sort_mode_ == "name");
    (*command_sort_kind_).set_checked(sort_mode_ == "kind");
    (*command_sort_size_).set_checked(sort_mode_ == "size");
    (*command_sort_modified_).set_checked(sort_mode_ == "modified");
    (*command_refresh_).set_enabled(
        files_active && !search_loading_ && !criteria_loading_);
    (*command_refresh_).set_description(criteria_showing_
        ? "Repeat the visible exact Criteria query"
        : search_showing_ ? "Repeat the visible Engine search query"
                          : "Read the current local folder again");
    (*command_refresh_).set_availability_reason(!files_active
        ? hidden_workspace_reason : search_loading_ || criteria_loading_
            ? "The current Engine query is still loading" : std::string{});
    (*command_toggle_tree_).set_enabled(files_active);
    (*command_toggle_selection_).set_enabled(files_active);
    (*command_toggle_tree_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
    (*command_toggle_selection_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
    (*command_toggle_tree_).set_checked(!(*workspace_split_).first_collapsed());
    (*command_toggle_selection_).set_checked(!(*selection_split_).second_collapsed());
    (*command_checksum_).set_visible(checksum_visible_);
    (*command_checksum_).set_enabled(files_active && (checksum_in_flight_ ||
        (entry && !(*entry).directory && (*entry).kind != EntryKind::symlink)));
    (*command_checksum_).set_availability_reason((*command_checksum_).state().enabled
        ? std::string{} : !files_active
            ? hidden_workspace_reason
            : "Select one regular file to compute SHA-256");
    (*command_checksum_).set_text(checksum_in_flight_ ? "Cancel SHA-256" : "SHA-256…");
    (*expected_checksum_box_).set_enabled(
        checksum_visible_ && files_active && !checksum_in_flight_ && entry &&
        !(*entry).directory && (*entry).kind != EntryKind::symlink);
    (*command_terminal_).set_visible(terminal_visible_);
#if defined(__linux__)
    (*command_terminal_).set_enabled(false);
    (*command_terminal_).set_availability_reason(
        "A Linux terminal launcher has not been configured");
#else
    (*command_terminal_).set_enabled(files_active);
    (*command_terminal_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
#endif
    (*command_copy_path_).set_enabled(files_active);
    (*command_copy_path_).set_availability_reason(
        files_active ? std::string{} : hidden_workspace_reason);
    (*command_settings_).set_text(settings_open_ ? "Back to files" : "Settings…");
    (*command_settings_).set_description(settings_open_
        ? "Return to the File Manager workspace"
        : "Open File Manager settings and Orchestrator service controls");
    (*command_close_).set_enabled(static_cast<bool>(request_close_));
    (*command_close_).set_availability_reason(request_close_
        ? std::string{} : "The native window close route is not bound yet");

    (*shelf_view_button_).set_enabled(folder_presentation_active);
    (*shelf_sort_button_).set_enabled(folder_presentation_active);
    (*shelf_view_button_).set_accessible_description(folder_presentation_active
        ? details_mode_
            ? "Current presentation: Details. Choose the object presentation"
            : "Current presentation: Small icons. Choose the object presentation"
        : correspondence_active
            ? "Folder presentation is unavailable for correspondence results"
            : hidden_workspace_reason);
    (*shelf_sort_button_).set_accessible_description(folder_presentation_active
        ? "Sort visible folder objects by one factual field"
        : correspondence_active
            ? "Folder sorting is unavailable for correspondence results"
            : hidden_workspace_reason);

    (*form_.file_manager_app_shell_commands_selection_group_actions_copy).set_enabled(
        mutation_ready && one);
    (*form_.file_manager_app_shell_commands_selection_group_actions_delete).set_enabled(
        mutation_ready && one);
    (*form_.file_manager_app_shell_location_navigation_back).set_enabled(
        (*command_back_).state().enabled);
    (*form_.file_manager_app_shell_location_navigation_forward).set_enabled(
        (*command_forward_).state().enabled);
    (*form_.file_manager_app_shell_location_navigation_up).set_enabled(
        (*command_up_).state().enabled);
    (*command_focus_location_).set_enabled(files_active && !(*rename_box_).visible());
    (*command_focus_search_).set_enabled(files_active && engine_search_available() && !(*rename_box_).visible());
    (*command_focus_search_).set_availability_reason(engine_search_available()
        ? (files_active ? std::string{} : hidden_workspace_reason)
        : "Search unavailable for this root");
    (*search_box_).set_enabled(files_active && engine_search_available());
    (*search_box_).set_placeholder_text(engine_search_available()
        ? "Search this subtree"
        : "Search not configured here");
}

void Application::focus_active_object_surface() {
    if (!window_) return;
    const gui_forms::Control::Ptr target = (*correspondence_).effectively_visible()
        ? std::static_pointer_cast<gui_forms::Control>(correspondence_)
        : std::static_pointer_cast<gui_forms::Control>(objects_);
    static_cast<void>((*window_).request_focus(target));
}

void Application::update_adaptive_preview() {
    const bool selected = !(*objects_).selected_ids().empty();
    const double height = (*form_.file_manager_app_shell).committed_arranged_bounds().height;
    const bool automatic_expansion = height == 0.0 || height >= 560.0;
    const bool expanded = selected && preview_expanded_override_.value_or(automatic_expansion);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).set_visible(expanded);
    gui_forms::Button& toggle = *form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_toggle;
    toggle.set_enabled(selected);
    toggle.set_text(expanded ? "Hide preview" : "Show preview");
    toggle.set_accessible_name(expanded ? "Hide selected file preview" : "Show selected file preview");
    const double preview_height = expanded ? 260.0 : 81.0;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).set_minimum_size({0.0, preview_height});
    gui_forms::Rect requested = (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).requested_bounds();
    requested.height = preview_height;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview).set_requested_bounds(requested);
    if (property_list_) (*property_list_).set_header_height(preview_height);
}

void Application::toggle_preview() {
    if ((*objects_).selected_ids().empty()) return;
    const bool expanded = (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface).visible();
    preview_expanded_override_ = !expanded;
    update_adaptive_preview();
}

void Application::update_adaptive_layout(const gui_forms::Rect bounds) {
    if (adapting_layout_ || bounds.width <= 0.0 || bounds.height <= 0.0) return;
    adapting_layout_ = true;
    try {
        const bool stacked = bounds.width < 720.0 && bounds.width >= 420.0 && bounds.height >= 360.0;
        const bool single_field = bounds.width < 420.0 || (bounds.width < 720.0 && bounds.height < 360.0);
        if (adaptive_viewport_ != gui_forms::Size{bounds.width, bounds.height} &&
            window_ && (*window_).focused_control() == search_box_) compact_search_active_ = true;
        adaptive_viewport_ = {bounds.width, bounds.height};
        (*form_.file_manager_app_shell_location_navigation).set_visible(bounds.width >= 720.0);
        (*form_.file_manager_app_shell_location_path_host).set_visible(!single_field || !compact_search_active_);
        (*form_.file_manager_app_shell_location_search_host).set_visible(!single_field || compact_search_active_);
        (*form_.file_manager_app_shell_location).set_flow_direction(stacked
            ? gui_forms::FlowDirection::top_down : gui_forms::FlowDirection::left_to_right);
        (*form_.file_manager_app_shell_location).set_wrap_contents(false);
        (*form_.file_manager_app_shell_location).set_flex_grow(*form_.file_manager_app_shell_location_path_host, 0.0);
        (*form_.file_manager_app_shell_location).set_flex_grow(*form_.file_manager_app_shell_location_search_host, 0.0);
        const double field_space = std::max(0.0, bounds.width - (bounds.width >= 720.0 ? 118.0 : 14.0));
        const double search_width = stacked || single_field ? field_space : std::min(465.0, field_space / 3.0);
        const double path_width = stacked || single_field ? field_space : field_space - search_width;
        (*form_.file_manager_app_shell_location_path_host).set_requested_bounds({0, 0, path_width, 30});
        (*form_.file_manager_app_shell_location_path_host).set_maximum_size({path_width, 30});
        (*form_.file_manager_app_shell_location_search_host).set_requested_bounds({0, 0, search_width, 30});
        (*form_.file_manager_app_shell_location_search_host).set_maximum_size({search_width, 30});
        const double location_height = stacked ? 79.0 : 42.0;
        (*form_.file_manager_app_shell_location).set_minimum_size({0, location_height});
        gui_forms::Rect location_bounds = (*form_.file_manager_app_shell_location).requested_bounds();
        location_bounds.height = location_height;
        (*form_.file_manager_app_shell_location).set_requested_bounds(location_bounds);
        gui_forms::ResponsiveTrackSpec location_track = (*form_.file_manager_app_shell).track_specs()[3];
        location_track.minimum = location_height;
        location_track.preferred = location_height;
        (*form_.file_manager_app_shell).set_track_spec(3, location_track);

        double menu_width = 24.0;
        for (const gui_forms::MenuStripItemSpec& item : expanded_menu_items_) {
            menu_width += (window_ ? (*window_).resolve_text_layout_utf8(item.text, (*menu_strip_).font()).logical_size.width
                                  : static_cast<double>(item.text.size()) * 8.0) + 2.0 * (*menu_strip_).item_padding();
        }
        const bool compact = bounds.width < menu_width;
        if (compact != compact_menu_) {
            compact_menu_ = compact;
            if (compact) {
                std::vector<gui_forms::MenuItemSpec> categories{};
                for (const gui_forms::MenuStripItemSpec& item : expanded_menu_items_) {
                    categories.push_back({item.stable_id, gui_forms::MenuItemKind::submenu, {}, item.text, item.items});
                }
                (*menu_strip_).set_items({{"fm.menu.compact", "Menu", std::move(categories)}});
                (*menu_strip_).set_selected_item_id("fm.menu.compact");
            } else {
                (*menu_strip_).set_items(expanded_menu_items_);
                (*menu_strip_).set_selected_item_id("fm.menu.home");
            }
        }
        update_adaptive_preview();
    } catch (...) {
        adapting_layout_ = false;
        throw;
    }
    adapting_layout_ = false;
}

void Application::focus_search_command() { static_cast<void>(focus_search_accelerator()); }
void Application::focus_location_command() { static_cast<void>(focus_location_accelerator()); }

bool Application::focus_search_accelerator() {
    if (!window_ || settings_open_ || (*rename_box_).visible() || !(*search_box_).enabled()) return false;
    const std::shared_ptr<gui_forms::TextBox> editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window_).focused_control());
    if (editor && editor != path_box_ && editor != search_box_) return false;
    compact_search_active_ = true;
    update_adaptive_layout((*form_.file_manager_app_shell).committed_arranged_bounds());
    (*search_box_).select_all();
    const bool focused = (*window_).request_focus(search_box_);
    return focused;
}

void Application::bind_button_action(const std::shared_ptr<gui_forms::Button>& button,
                                     std::function<void()> action) {
    gui_forms::SubscriptionToken connection = (*button).clicked().subscribe(
        ButtonAction{std::move(action)});
    subscriptions_.push_back(std::move(connection));
}

void Application::bind_settings_tab(const std::shared_ptr<gui_forms::Button>& button,
                                    std::string id, std::string title) {
    bind_button_action(button, std::bind_front(&Application::select_settings_tab,
        this, std::move(id), std::move(title)));
}

void Application::request_more_results() {
    if (criteria_showing_) request_engine_criteria(true);
    else request_engine_search(true);
}

void Application::on_shelf_sort_button_drop_down_requested(gui_forms::DropDownButton&) {
    show_menu(sort_menu_, shelf_sort_button_);
}

void Application::on_shelf_sort_button_drop_down_close_requested(gui_forms::DropDownButton&) { (*sort_menu_).close(); }

void Application::on_shelf_view_button_drop_down_requested(gui_forms::DropDownButton&) {
    show_menu(view_menu_, shelf_view_button_);
}

void Application::on_shelf_view_button_drop_down_close_requested(gui_forms::DropDownButton&) { (*view_menu_).close(); }

void Application::on_shelf_move_copy_button_drop_down_requested(gui_forms::DropDownButton&) {
    show_menu(move_copy_menu_, shelf_move_copy_button_);
}

void Application::on_shelf_move_copy_button_drop_down_close_requested(gui_forms::DropDownButton&) { (*move_copy_menu_).close(); }

void Application::on_shelf_overflow_button_drop_down_requested(gui_forms::DropDownButton&) {
    show_command_shelf_overflow();
}

void Application::on_shelf_overflow_button_drop_down_close_requested(gui_forms::DropDownButton&) {
    (*shelf_overflow_menu_).close();
}

void Application::on_tree_root_mode_button_drop_down_requested(gui_forms::DropDownButton&) {
    show_tree_root_menu();
}

void Application::on_tree_root_mode_button_drop_down_close_requested(gui_forms::DropDownButton&) {
    (*tree_root_menu_).close();
}

void Application::on_breadcrumb_segment_activated(const std::string& id) {
    const PathMap::const_iterator path = breadcrumb_paths_.find(id);
    if (path != breadcrumb_paths_.end()) {
        request_navigation((*path).second, true);
    }
}

void Application::on_breadcrumb_edit_committed(const std::string& text) { request_navigation(path_from_utf8(text), true); }

void Application::on_breadcrumb_edit_started(const std::string& text) { request_path_suggestions(text); }

void Application::on_breadcrumb_edit_cancelled() {
    (*path_box_).set_text(path_utf8(location_));
    set_path_editing(false);
}

void Application::on_path_box_text_changed(const std::string& text) {
    if ((*breadcrumb_).editing()) request_path_suggestions(text);
}

void Application::on_path_box_focus_observed(const bool focused) {
    if (!focused && (*breadcrumb_).editing()) {
        (*path_box_).set_text(path_utf8(location_));
        set_path_editing(false);
    }
}

void Application::on_root_pointer_preview(const gui_forms::PointerEvent& event) {
    if (!(*breadcrumb_).editing() ||
        event.action != gui_forms::PointerAction::down ||
        event.button != gui_forms::PointerButton::primary ||
        (*breadcrumb_).absolute_bounds().contains(event.position)) {
        return;
    }
    (*path_box_).set_text(path_utf8(location_));
    set_path_editing(false);
}

void Application::on_search_box_committed(const std::string&) { apply_filter(); }

void Application::on_search_box_cancelled() {
    (*search_box_).set_text({});
    apply_filter();
}

void Application::on_criteria_rack_field_committed(const gui_forms::InstrumentFieldChange&) {
    if (criteria_showing_) request_engine_criteria();
}

void Application::on_criteria_rack_module_toggled(const gui_forms::InstrumentModuleToggle&) {
    update_criteria_action_state();
    if (criteria_showing_) request_engine_criteria();
}

void Application::on_criteria_rack_remove_requested(const gui_forms::InstrumentModuleRequest& request) {
    remove_criteria_module(request.module_id);
}

void Application::on_rename_box_committed(const std::string& basename) { commit_rename(basename); }

void Application::on_objects_selection_changed(const gui_forms::ObjectSelectionChange& change) {
    update_selection(change.current_id);
}

void Application::on_objects_item_activated(const std::string& stable_id) {
    (*objects_).set_selected_id(stable_id);
    static_cast<void>((*command_open_).execute("fm.objects.activation"));
}

void Application::on_objects_context_requested(const gui_forms::ObjectContextRequest& request) {
    if (request.stable_id.empty()) {
        (*objects_).clear_selection();
        update_selection({});
        (*background_menu_).show(objects_, request.screen_position);
    } else {
        (*objects_).set_selected_id(request.stable_id);
        (*object_menu_).show(objects_, request.screen_position);
    }
}

void Application::on_objects_pointer_observed(const gui_forms::PointerEvent& event) {
    observe_object_pointer(event);
}

void Application::on_tree_item_activated(const std::string& stable_id) { activate(stable_id); }

void Application::on_tree_selection_changed(const gui_forms::TreeSelectionChange& change) {
    if (!tree_model_syncing_ && !change.current_id.empty()) {
        activate(change.current_id);
    }
}

void Application::on_tree_expansion_changed(const gui_forms::TreeExpansionChange& change) {
    if (tree_model_syncing_) return;
    const PathMap::const_iterator found = tree_locations_.find(change.stable_id);
    if (found == tree_locations_.end()) return;
    const std::string key = path_generic_utf8((*found).second);
    if (change.expanded) {
        tree_expanded_paths_.insert(key);
        request_tree_expansion((*found).second);
    } else {
        tree_expanded_paths_.erase(key);
    }
}

void Application::on_property_list_value_committed(const gui_forms::PropertyValueChange& change) {
    if (change.row_id == "fm.property.name" &&
        change.current_value != change.previous_value) {
        commit_property_name(change.current_value);
    }
}

void Application::on_correspondence_selection_changed(const gui_forms::CorrespondenceSelectionChange& change) {
    (*objects_).set_selected_id(change.current_id);
}

void Application::on_correspondence_item_activated(const std::string& stable_id) {
    (*correspondence_).set_selected_id(stable_id);
    (*objects_).set_selected_id(stable_id);
    static_cast<void>((*command_open_).execute(
        "fm.correspondence.activation"));
}

void Application::on_correspondence_context_requested(const gui_forms::ObjectContextRequest& request) {
    if (request.stable_id.empty()) {
        (*correspondence_).set_selected_id({});
        (*objects_).clear_selection();
        update_selection({});
        (*background_menu_).show(correspondence_, request.screen_position);
    } else {
        (*correspondence_).set_selected_id(request.stable_id);
        (*objects_).set_selected_id(request.stable_id);
        (*object_menu_).show(correspondence_, request.screen_position);
    }
}

void Application::on_workspace_split_splitter_changed(const gui_forms::SplitChangeEvent&) { update_command_state(); }

void Application::on_selection_split_splitter_changed(const gui_forms::SplitChangeEvent&) { update_command_state(); }

void Application::install_handlers() {
    (*form_.file_manager_app_shell).set_track_focus_fallback(2, *menu_strip_);
    subscriptions_.push_back((*form_.file_manager_app_shell).arranged_bounds_changed().subscribe(
        std::bind_front(&Application::update_adaptive_layout, this)));
    bind_button_action(form_.file_manager_app_shell_location_navigation_back,
          CommandExecution{command_back_, "fm.button.go.back"});
    bind_button_action(form_.file_manager_app_shell_location_navigation_forward,
          CommandExecution{command_forward_, "fm.button.go.forward"});
    bind_button_action(form_.file_manager_app_shell_location_navigation_up,
          CommandExecution{command_up_, "fm.button.go.up"});
    bind_button_action(form_.file_manager_app_shell_commands_home,
          std::bind_front(&Application::navigate_home, this));
    subscriptions_.push_back((*shelf_sort_button_).drop_down_requested().subscribe(
        std::bind_front(&Application::on_shelf_sort_button_drop_down_requested, this)));
    subscriptions_.push_back((*shelf_sort_button_).drop_down_close_requested().subscribe(
        std::bind_front(&Application::on_shelf_sort_button_drop_down_close_requested, this)));
    subscriptions_.push_back((*shelf_view_button_).drop_down_requested().subscribe(
        std::bind_front(&Application::on_shelf_view_button_drop_down_requested, this)));
    subscriptions_.push_back((*shelf_view_button_).drop_down_close_requested().subscribe(
        std::bind_front(&Application::on_shelf_view_button_drop_down_close_requested, this)));
    bind_button_action(form_.file_manager_app_shell_commands_new_folder,
          std::bind_front(&Application::request_create_folder, this));
    bind_button_action(form_.file_manager_app_shell_commands_rename,
          std::bind_front(&Application::begin_rename, this));
    subscriptions_.push_back(
        (*shelf_move_copy_button_).drop_down_requested().subscribe(
            std::bind_front(&Application::on_shelf_move_copy_button_drop_down_requested, this)));
    subscriptions_.push_back(
        (*shelf_move_copy_button_).drop_down_close_requested().subscribe(
            std::bind_front(&Application::on_shelf_move_copy_button_drop_down_close_requested, this)));
    subscriptions_.push_back(
        (*shelf_overflow_button_).drop_down_requested().subscribe(
            std::bind_front(&Application::on_shelf_overflow_button_drop_down_requested, this)));
    subscriptions_.push_back(
        (*shelf_overflow_button_).drop_down_close_requested().subscribe(
            std::bind_front(&Application::on_shelf_overflow_button_drop_down_close_requested, this)));
    bind_button_action(form_.file_manager_app_shell_commands_move,
          std::bind_front(&Application::capture_transfer, this, true));
    bind_button_action(form_.file_manager_app_shell_commands_paste,
          std::bind_front(&Application::paste_transfer, this));
    bind_button_action(form_.file_manager_app_shell_status_cancel_copy,
          CommandExecution{command_cancel_copy_, "fm.button.transfer.cancel"});
    bind_button_action(form_.file_manager_app_shell_commands_selection_group_actions_delete,
          CommandExecution{command_delete_, "fm.button.selection.delete"});
    bind_button_action(form_.file_manager_app_shell_commands_arrange_group_actions_settings,
          CommandExecution{command_properties_, "fm.button.view.properties"});
    bind_button_action(form_.file_manager_app_shell_workspace_selection_content_heading_more_results,
          std::bind_front(&Application::request_more_results, this));
    bind_button_action(criteria_add_button_, std::bind_front(&Application::add_criteria_module, this));
    bind_button_action(form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_toggle,
        std::bind_front(&Application::toggle_preview, this));
    bind_button_action(form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_open,
          CommandExecution{command_open_, "fm.button.file.open"});
    bind_button_action(form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum,
          CommandExecution{command_checksum_, "fm.button.commands.sha256"});
    bind_button_action(form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_terminal,
          CommandExecution{command_terminal_, "fm.button.commands.terminal"});
    bind_button_action(form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_copy_path,
          CommandExecution{command_copy_path_, "fm.button.commands.copy-path"});
    bind_button_action(form_.file_manager_app_shell_settings_actions_back,
          std::bind_front(&Application::hide_settings, this));
    bind_button_action(form_.file_manager_app_shell_settings_actions_cancel,
          std::bind_front(&Application::cancel_settings_edits, this));
    bind_button_action(form_.file_manager_app_shell_settings_actions_reset,
          std::bind_front(&Application::reset_settings_page, this));
    bind_button_action(form_.file_manager_app_shell_settings_actions_apply,
          std::bind_front(&Application::apply_settings_changes, this));

    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_general,
        "general", "General");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_appearance,
        "appearance_access", "Appearance & Access");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_navigation,
        "navigation_views", "Navigation & Views");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_search,
        "search_indexing", "Search & Indexing");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_services,
        "services", "Services");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_handlers,
        "handlers_commands", "Handlers & Commands");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_previews,
        "previews_extensions", "Previews & Extensions");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_applications,
        "applications", "Applications");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_privacy,
        "privacy_data", "Privacy & Data");
    bind_settings_tab(form_.file_manager_app_shell_settings_body_tabs_advanced,
        "advanced", "Advanced");
    bind_button_action(form_.file_manager_app_shell_workspace_sidebar_root,
          std::bind_front(&Application::navigate_home, this));
    bind_button_action(form_.file_manager_app_shell_workspace_sidebar_parent,
          std::bind_front(&Application::navigate_up, this));
    subscriptions_.push_back(
        (*tree_root_mode_button_).drop_down_requested().subscribe(
            std::bind_front(&Application::on_tree_root_mode_button_drop_down_requested, this)));
    subscriptions_.push_back(
        (*tree_root_mode_button_).drop_down_close_requested().subscribe(
            std::bind_front(&Application::on_tree_root_mode_button_drop_down_close_requested, this)));

    subscriptions_.push_back((*breadcrumb_).segment_activated().subscribe(
        std::bind_front(&Application::on_breadcrumb_segment_activated, this)));
    subscriptions_.push_back((*breadcrumb_).overflow_activated().subscribe(
        std::bind_front(&Application::show_breadcrumb_overflow, this)));
    subscriptions_.push_back((*breadcrumb_).edit_committed().subscribe(
        std::bind_front(&Application::on_breadcrumb_edit_committed, this)));
    subscriptions_.push_back((*breadcrumb_).edit_started().subscribe(
        std::bind_front(&Application::on_breadcrumb_edit_started, this)));
    subscriptions_.push_back((*breadcrumb_).edit_cancelled().subscribe(
        std::bind_front(&Application::on_breadcrumb_edit_cancelled, this)));
    subscriptions_.push_back((*breadcrumb_).edit_completion_requested().subscribe(
        std::bind_front(&Application::accept_active_path_suggestion, this)));
    subscriptions_.push_back((*path_box_).text_changed().subscribe(
        std::bind_front(&Application::on_path_box_text_changed, this)));
    subscriptions_.push_back((*path_box_).focus_observed().subscribe(
        std::bind_front(&Application::on_path_box_focus_observed, this)));
    subscriptions_.push_back((*form_.root_control()).pointer_preview_observed().subscribe(
        std::bind_front(&Application::on_root_pointer_preview, this)));
    subscriptions_.push_back((*search_box_).committed().subscribe(
        std::bind_front(&Application::on_search_box_committed, this)));
    subscriptions_.push_back((*search_box_).cancelled().subscribe(std::bind_front(&Application::on_search_box_cancelled, this)));
    subscriptions_.push_back((*criteria_rack_).field_committed().subscribe(
        std::bind_front(&Application::on_criteria_rack_field_committed, this)));
    subscriptions_.push_back((*criteria_rack_).module_toggled().subscribe(
        std::bind_front(&Application::on_criteria_rack_module_toggled, this)));
    subscriptions_.push_back((*criteria_rack_).remove_requested().subscribe(
        std::bind_front(&Application::on_criteria_rack_remove_requested, this)));
    subscriptions_.push_back((*rename_box_).committed().subscribe(
        std::bind_front(&Application::on_rename_box_committed, this)));
    subscriptions_.push_back((*rename_box_).cancelled().subscribe(
        std::bind_front(&Application::cancel_rename, this)));
    subscriptions_.push_back((*objects_).selection_changed().subscribe(
        std::bind_front(&Application::on_objects_selection_changed, this)));
    subscriptions_.push_back((*objects_).sort_requested().subscribe(
        std::bind_front(&Application::on_objects_sort_requested, this)));
    subscriptions_.push_back((*objects_).arranged_bounds_changed().subscribe(
        std::bind_front(&Application::fit_details_columns, this)));
    subscriptions_.push_back((*objects_).item_activated().subscribe(
        std::bind_front(&Application::on_objects_item_activated, this)));
    subscriptions_.push_back((*objects_).context_requested().subscribe(
        std::bind_front(&Application::on_objects_context_requested, this)));
    subscriptions_.push_back((*objects_).pointer_observed().subscribe(
        std::bind_front(&Application::on_objects_pointer_observed, this)));
    subscriptions_.push_back((*tree_).item_activated().subscribe(
        std::bind_front(&Application::on_tree_item_activated, this)));
    subscriptions_.push_back((*tree_).selection_changed().subscribe(
        std::bind_front(&Application::on_tree_selection_changed, this)));
    subscriptions_.push_back((*tree_).expansion_changed().subscribe(
        std::bind_front(&Application::on_tree_expansion_changed, this)));
    subscriptions_.push_back((*property_list_).value_committed().subscribe(
        std::bind_front(&Application::on_property_list_value_committed, this)));
    subscriptions_.push_back((*correspondence_).selection_changed().subscribe(
        std::bind_front(&Application::on_correspondence_selection_changed, this)));
    subscriptions_.push_back((*correspondence_).item_activated().subscribe(
        std::bind_front(&Application::on_correspondence_item_activated, this)));
    subscriptions_.push_back((*correspondence_).context_requested().subscribe(
        std::bind_front(&Application::on_correspondence_context_requested, this)));
    subscriptions_.push_back((*workspace_split_).splitter_changed().subscribe(
        std::bind_front(&Application::on_workspace_split_splitter_changed, this)));
    subscriptions_.push_back((*selection_split_).splitter_changed().subscribe(
        std::bind_front(&Application::on_selection_split_splitter_changed, this)));
}

void Application::bind_host(std::function<void()> wake,
                            std::function<void()> request_close) {
    {
        std::lock_guard<std::mutex> lock(ui_mutex_);
        wake_ = std::move(wake);
        request_close_ = std::move(request_close);
    }
    update_command_state();
    request_bootstrap();
    request_settings();
    request_navigation(protected_root_, true);
}

void Application::post_worker(std::function<void()> work) {
    {
        std::lock_guard<std::mutex> lock(worker_mutex_);
        if (stopping_.load()) return;
        worker_queue_.push(std::move(work));
    }
    worker_cv_.notify_one();
}

void Application::post_ui(std::function<void()> work) {
    std::function<void()> wake{};
    {
        std::lock_guard<std::mutex> lock(ui_mutex_);
        if (stopping_.load()) return;
        ui_queue_.push(std::move(work));
        wake = wake_;
    }
    if (wake) wake();
}

void Application::worker_loop() {
    for (;;) {
        std::function<void()> work{};
        {
            std::unique_lock<std::mutex> lock(worker_mutex_);
            worker_cv_.wait(lock, std::bind_front(&Application::worker_ready, this));
            if (stopping_.load() && worker_queue_.empty()) return;
            work = std::move(worker_queue_.front());
            worker_queue_.pop();
        }
        try {
            work();
        } catch (const std::exception& error) {
            const std::string message = error.what();
            post_ui(std::bind_front(&Application::on_worker_failed, this, message));
        }
    }
}

void Application::drain_ui() {
    std::queue<std::function<void()>> pending{};
    {
        std::lock_guard<std::mutex> lock(ui_mutex_);
        pending.swap(ui_queue_);
    }
    while (!pending.empty()) {
        // stop() can be called reentrantly by the preceding completion.
        if (stopping_.load()) break;
        pending.front()();
        pending.pop();
    }
}

// Called on the UI/owning thread. Existing worker jobs drain during join so
// admitted mutations retain their completion semantics; UI completions are revoked.
void Application::stop() {
    if (stopping_.exchange(true)) return;
    {
        std::scoped_lock<std::mutex, std::mutex> lock(worker_mutex_, ui_mutex_);
        wake_ = {};
        request_close_ = {};
        while (!ui_queue_.empty()) ui_queue_.pop();
    }
    worker_cv_.notify_all();
    if (worker_.joinable()) worker_.join();
}

void Application::navigate_breadcrumb(const std::filesystem::path& target, const gui_forms::CommandInvocation&) {
    request_navigation(target, true);
}

void Application::on_path_suggestion_activated(const std::size_t index) {
    accept_path_suggestion(index);
}

void Application::on_path_suggestion_dismissed(const gui_forms::PopupDismissReason reason) {
    (void)reason;
    (*path_box_).set_text(path_utf8(location_));
    set_path_editing(false);
}

void Application::on_setting_committed(const gui_forms::PropertyValueChange& change) {
    if (!settings_schema_ || !settings_property_list_) return;
    const std::vector<fileman::orchestrator::SettingSchemaFieldInfo>::const_iterator field = std::find_if(
        (*settings_schema_).fields.begin(),
        (*settings_schema_).fields.end(),
        SettingId{change.row_id});
    if (field == (*settings_schema_).fields.end() ||
        (*field).availability != "available") {
        return;
    }
    (*settings_property_list_).set_validation(change.row_id, {});
    if ((*field).value_type == "boolean") {
        edit_setting(change.row_id,
                     change.current_value == "True" ||
                         change.current_value == "1");
        return;
    }
    if ((*field).value_type == "unsigned_integer") {
        try {
            std::size_t parsed = 0;
            const unsigned long long value = std::stoull(change.current_value,
                                          &parsed, 10);
            if (parsed != change.current_value.size() ||
                ((*field).minimum && value < *(*field).minimum) ||
                ((*field).maximum && value > *(*field).maximum)) {
                throw std::out_of_range("setting range");
            }
            edit_setting(change.row_id,
                static_cast<std::uint64_t>(value));
        } catch (const std::exception&) {
            (*settings_property_list_).set_value(
                change.row_id, change.previous_value);
            (*settings_property_list_).set_validation(
                change.row_id,
                "Enter an integer within the declared range");
        }
        return;
    }
    edit_setting(change.row_id, change.current_value);
}

void Application::on_services_refresh(gui_forms::ButtonBase&) {
    services_snapshot_.reset();
    service_notice_.clear();
    request_services();
}

void Application::on_service_command(const std::string& service_id, const std::string& command_id, const std::optional<std::string>& root, gui_forms::ButtonBase&) {
    run_service_command(service_id, command_id, root);
}

bool Application::worker_ready() {
    const bool ready = stopping_.load() || !worker_queue_.empty();
    return ready;
}

void Application::on_worker_failed(const std::string& message) {
    set_status("Background operation failed", message);
}

void Application::request_bootstrap() {
    (*form_.file_manager_app_shell_title_service).set_text("Orchestrator · connecting");
    post_worker(BootstrapWork{shared_from_this()});
}

void Application::request_settings() {
    if (settings_loading_ || settings_apply_in_flight_) return;
    settings_loading_ = true;
    (*form_.file_manager_app_shell_settings_heading_revision).set_text("connecting…");
    post_worker(SettingsWork{shared_from_this()});
}

void Application::request_services() {
    if (services_loading_ || service_command_in_flight_) return;
    services_loading_ = true;
    if (settings_open_ && settings_tab_ == "services") rebuild_settings_page();
    post_worker(ServicesWork{shared_from_this()});
}

void Application::show_settings() {
    settings_open_ = true;
    (*form_.file_manager_app_shell_commands).set_visible(false);
    (*form_.file_manager_app_shell_location).set_visible(false);
    (*form_.file_manager_app_shell_workspace).set_visible(false);
    (*form_.file_manager_app_shell_settings).set_visible(true);
    if (!settings_schema_ || !settings_snapshot_) request_settings();
    if (settings_tab_ == "services" && !services_snapshot_) request_services();
    rebuild_settings_page();
    update_command_state();
    set_status("Settings", "ORC-SET-001 · typed optimistic transactions");
}

void Application::hide_settings() {
    settings_open_ = false;
    (*form_.file_manager_app_shell_settings).set_visible(false);
    (*form_.file_manager_app_shell_commands).set_visible(true);
    (*form_.file_manager_app_shell_location).set_visible(true);
    (*form_.file_manager_app_shell_workspace).set_visible(true);
    update_command_state();
    focus_active_object_surface();
    set_status("Files", "direct filesystem · " +
        navigation_label(navigation_root_) +
        (mutation_scope_active() ? " · protected operations admitted"
                                 : " · read-only observation"));
}

void Application::select_settings_tab(std::string tab, std::string title) {
    settings_tab_ = std::move(tab);
    update_settings_tab_state();
    (*form_.file_manager_app_shell_settings_body_page_title).set_text(
        std::move(title));
    (*form_.file_manager_app_shell_settings_body_page_description).set_text(
        settings_tab_description(settings_tab_));
    if (settings_tab_ == "services") {
        request_services();
    } else {
        rebuild_settings_page();
    }
}

void Application::update_settings_tab_state() {
    const SettingsTab tabs[]{
        {form_.file_manager_app_shell_settings_body_tabs_general, "general"},
        {form_.file_manager_app_shell_settings_body_tabs_appearance,
         "appearance_access"},
        {form_.file_manager_app_shell_settings_body_tabs_navigation,
         "navigation_views"},
        {form_.file_manager_app_shell_settings_body_tabs_search,
         "search_indexing"},
        {form_.file_manager_app_shell_settings_body_tabs_services, "services"},
        {form_.file_manager_app_shell_settings_body_tabs_handlers,
         "handlers_commands"},
        {form_.file_manager_app_shell_settings_body_tabs_previews,
         "previews_extensions"},
        {form_.file_manager_app_shell_settings_body_tabs_applications,
         "applications"},
        {form_.file_manager_app_shell_settings_body_tabs_privacy,
         "privacy_data"},
        {form_.file_manager_app_shell_settings_body_tabs_advanced, "advanced"},
    };
    for (const SettingsTab& tab : tabs) {
        const std::shared_ptr<gui_forms::Button>& button = tab.button;
        const std::string_view id = tab.id;
        const bool active = settings_tab_ == id;
        (*button).set_selected(active);
        (*button).set_accessible_description(active
            ? "Current settings category"
            : "Open this settings category");
        if (active && settings_tab_selected_recipes_) {
            (*button).set_visual_recipes(*settings_tab_selected_recipes_);
        } else if (!active && settings_tab_normal_recipes_) {
            (*button).set_visual_recipes(*settings_tab_normal_recipes_);
        }
    }
}

void Application::apply_settings_state(
    fileman::orchestrator::SettingsSchemaInfo schema,
    fileman::orchestrator::SettingsSnapshotInfo snapshot,
    std::string notice) {
    if (schema.schema_revision != snapshot.schema_revision) {
        (*form_.file_manager_app_shell_settings_actions_status).set_text(
            "Settings schema/value revision mismatch");
        settings_schema_.reset();
        settings_snapshot_.reset();
        update_settings_actions();
        return;
    }
    settings_schema_ = std::move(schema);
    settings_snapshot_ = std::move(snapshot);
    (*form_.file_manager_app_shell_settings_heading_revision).set_text(
        "revision " + std::to_string((*settings_snapshot_).revision) + " · " +
        (*settings_snapshot_).recovery_provenance);
    apply_runtime_settings();
    rebuild_settings_page();
    if (!notice.empty()) {
        (*form_.file_manager_app_shell_settings_actions_status).set_text(
            std::move(notice));
    }
    update_settings_actions();
}

void Application::rebuild_settings_page() {
    settings_subscriptions_.clear();
    const bool services_page = settings_tab_ == "services";
    (*form_.file_manager_app_shell_settings_actions).set_visible(!services_page);
    (*form_.file_manager_app_shell_settings_body_page_title).set_visible(
        !services_page);
    (*form_.file_manager_app_shell_settings_body_page_description).set_visible(
        !services_page);
    const std::shared_ptr<gui_forms::FlowLayoutPanel> host = form_.file_manager_app_shell_settings_body_page_host;
    (*host).clear_children();
    settings_property_list_.reset();
    if (settings_tab_ == "services") {
        rebuild_services_page();
        update_settings_actions();
        return;
    }
    if (!settings_schema_ || !settings_snapshot_) {
        std::shared_ptr<gui_forms::Label> message = std::make_shared<gui_forms::Label>(
            gui_forms::StableId("fm.settings.loading"),
            settings_loading_ ? "Reading the typed settings schema…"
                              : "Settings service is unavailable");
        (*message).set_requested_bounds({0, 0, 620, 34});
        (*message).set_accessible_name("Settings state");
        (*host).add_child(message);
        return;
    }

    std::vector<gui_forms::PropertyRowSpec> rows{};
    for (const fileman::orchestrator::SettingSchemaFieldInfo& field : (*settings_schema_).fields) {
        if (field.presentation_tab != settings_tab_) continue;
        const SettingsValueMap::const_iterator pending = pending_settings_.find(field.id);
        const fileman::orchestrator::SettingValue* const committed = (*settings_snapshot_).find(field.id);
        const fileman::orchestrator::SettingValue value = pending != pending_settings_.end()
            ? (*pending).second
            : (committed ? *committed : field.default_value);
        const bool available = field.availability == "available";
        gui_forms::PropertyEditorKind editor = gui_forms::PropertyEditorKind::text;
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
        const std::string display_value = available
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
    (*settings_property_list_).set_accessible_name("Settings on this page");
    (*settings_property_list_).set_label_width(210.0);
    (*settings_property_list_).set_requested_bounds({0, 0, 720, 250});
    (*settings_property_list_).set_minimum_size({520, 160});
    (*settings_property_list_).set_maximum_size({4096, 250});
    (*settings_property_list_).set_groups({
        {"fm.settings.group." + settings_tab_, "SETTINGS", std::move(rows)},
    });
    (*host).add_child(settings_property_list_);
    (*host).set_flex_grow(*settings_property_list_, 1.0);
    settings_subscriptions_.push_back(
        (*settings_property_list_).value_committed().subscribe(
            std::bind_front(&Application::on_setting_committed, this)));
    update_settings_actions();
}

void Application::rebuild_services_page() {
    const std::shared_ptr<gui_forms::FlowLayoutPanel> host = form_.file_manager_app_shell_settings_body_page_host;
    std::vector<gui_forms::PropertyGroupSpec> groups{};
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
        const std::vector<fileman::orchestrator::SettingSchemaFieldInfo>::const_iterator field = std::find_if(
            (*settings_schema_).fields.begin(), (*settings_schema_).fields.end(),
            SettingId{"services.diagnostics_level"});
        if (field != (*settings_schema_).fields.end()) {
            groups.front().rows.push_back({
                (*field).id, "Diagnostics",
                (*field).availability +
                    ((*field).availability_reason.empty()
                         ? std::string{}
                         : " — " + (*field).availability_reason),
                "Availability: " + (*field).availability + " · " +
                    (*field).availability_reason,
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
        for (const fileman::orchestrator::ServiceInfo& service : (*services_snapshot_).services) {
            std::vector<gui_forms::PropertyRowSpec> rows{};
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
            for (const fileman::orchestrator::ServiceCommandInfo& command : service.commands) {
                const bool root_command = service.id == "engine" &&
                    (command.id == "reconcile" || command.id == "rebuild");
                const bool root_admitted = !root_command ||
                    (!engine_root_id_.empty() &&
                    std::find(service.roots.begin(), service.roots.end(),
                              engine_root_id_) != service.roots.end());
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
    (*settings_property_list_).set_accessible_name(
        "Installed service facts and admitted commands");
    (*settings_property_list_).set_label_width(210.0);
    (*settings_property_list_).set_requested_bounds({0, 0, 720, 250});
    (*settings_property_list_).set_minimum_size({520, 160});
    (*settings_property_list_).set_maximum_size({4096, 250});
    (*settings_property_list_).set_groups(std::move(groups));
    (*host).add_child(settings_property_list_);
    (*host).set_flex_grow(*settings_property_list_, 1.0);

    std::shared_ptr<gui_forms::Button> refresh = std::make_shared<gui_forms::Button>(
        gui_forms::StableId("fm.services.refresh.button"),
        services_loading_ ? "Reading services…" : "Refresh service facts");
    (*refresh).set_requested_bounds({0, 0, 190, 28});
    (*settings_property_list_).replace_editor("fm.services.refresh", refresh);
    settings_subscriptions_.push_back((*refresh).clicked().subscribe(
        std::bind_front(&Application::on_services_refresh, this)));

    if (services_snapshot_) {
        for (const fileman::orchestrator::ServiceInfo& service : (*services_snapshot_).services) {
            for (const fileman::orchestrator::ServiceCommandInfo& command : service.commands) {
                const bool root_command = service.id == "engine" &&
                    (command.id == "reconcile" || command.id == "rebuild");
                const bool root_admitted = !root_command ||
                    (!engine_root_id_.empty() &&
                    std::find(service.roots.begin(), service.roots.end(),
                              engine_root_id_) != service.roots.end());
                const std::string row_id = "fm.services." + service.id +
                    ".command." + command.id;
                std::shared_ptr<gui_forms::Button> button = std::make_shared<gui_forms::Button>(
                    gui_forms::StableId(row_id + ".button"), command.title);
                (*button).set_requested_bounds({0, 0, 180, 28});
                (*button).set_enabled(command.available && root_admitted &&
                                    !service_command_in_flight_ &&
                                    !services_loading_);
                (*button).set_accessible_description(command.effect);
                (*settings_property_list_).replace_editor(row_id, button);
                const std::optional<std::string> root = root_command
                    ? std::optional<std::string>(engine_root_id_)
                    : std::nullopt;
                settings_subscriptions_.push_back((*button).clicked().subscribe(
                    std::bind_front(&Application::on_service_command, this, service.id, command.id, root)));
            }
        }
    }
    (*form_.file_manager_app_shell_settings_actions_status).set_text(
        service_notice_);
}

bool Application::confirm_service_command(const std::string_view service_id,
                                          const std::string_view command_id) {
    if (command_id != "rebuild" && command_id != "restart" &&
        command_id != "shutdown") {
        return true;
    }
    if (!window_ || !(*window_).host_services()) return false;
    gui_forms::HostMessageDialogRequest message{};
    message.title = "Confirm service command";
    message.message = std::string("Run ") + std::string(command_id) +
        " for " + std::string(service_id) +
        "? The command is identity-checked immediately before execution.";
    message.buttons = gui_forms::HostMessageButtons::yes_no;
    message.icon = gui_forms::HostMessageIcon::warning;
    message.default_choice = gui_forms::HostDialogChoice::no;
    gui_forms::HostDialogRequest request{};
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.services";
    request.payload = std::move(message);
    const gui_forms::HostDialogResult result = (*(*window_).host_services()).show_dialog(request);
    const gui_forms::HostMessageDialogResult* const choice = std::get_if<gui_forms::HostMessageDialogResult>(
        &result.payload);
    const bool confirmed = choice && (*choice).choice == gui_forms::HostDialogChoice::yes;
    return confirmed;
}

void Application::run_service_command(
    std::string service_id, std::string command_id,
    std::optional<std::string> root_id) {
    if (!services_snapshot_ || service_command_in_flight_ ||
        !confirm_service_command(service_id, command_id)) {
        return;
    }
    const fileman::orchestrator::ServiceInfo* const service = (*services_snapshot_).find(service_id);
    if (!service) return;
    const std::optional<std::string> instance = (*service).instance_id;
    const std::optional<std::uint64_t> generation = service_id == "orchestrator"
        ? (*service).generation
        : std::optional<std::uint64_t>{};
    service_command_in_flight_ = true;
    service_notice_ = "Running " + service_id + " " + command_id + "…";
    rebuild_settings_page();
    post_worker(ServiceCommandWork{shared_from_this(), std::move(service_id), std::move(command_id), std::move(root_id), instance, generation});
}

void Application::edit_setting(
    std::string id, fileman::orchestrator::SettingValue value) {
    if (!settings_snapshot_ || settings_apply_in_flight_) return;
    const fileman::orchestrator::SettingValue* const committed = (*settings_snapshot_).find(id);
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
    (*form_.file_manager_app_shell_settings_actions_status).set_text(
        "Pending changes cancelled");
}

void Application::reset_settings_page() {
    if (!settings_schema_ || !settings_snapshot_ || settings_apply_in_flight_) {
        return;
    }
    bool changed = false;
    for (const fileman::orchestrator::SettingSchemaFieldInfo& field : (*settings_schema_).fields) {
        if (field.presentation_tab == settings_tab_ &&
            field.availability == "available") {
            const SettingsValueMap::const_iterator pending = pending_settings_.find(field.id);
            const fileman::orchestrator::SettingValue* const committed = (*settings_snapshot_).find(field.id);
            const fileman::orchestrator::SettingValue& effective = pending != pending_settings_.end()
                ? (*pending).second
                : committed ? *committed : field.default_value;
            if (effective == field.default_value) continue;
            changed = true;
            if (committed && *committed == field.default_value) {
                pending_settings_.erase(field.id);
            } else {
                pending_settings_[field.id] = field.default_value;
            }
        }
    }
    if (!changed) {
        update_settings_actions();
        (*form_.file_manager_app_shell_settings_actions_status).set_text(
            "Page values already match declared defaults");
        return;
    }
    rebuild_settings_page();
    (*form_.file_manager_app_shell_settings_actions_status).set_text(
        "Page defaults staged · Apply commits them");
}

void Application::apply_settings_changes() {
    if (!settings_snapshot_ || pending_settings_.empty() ||
        settings_apply_in_flight_) {
        return;
    }
    std::vector<fileman::orchestrator::SettingChange> changes{};
    changes.reserve(pending_settings_.size());
    for (const SettingsValueMap::value_type& setting : pending_settings_) {
        changes.push_back({setting.first, setting.second});
    }
    const std::uint64_t revision = (*settings_snapshot_).revision;
    settings_apply_in_flight_ = true;
    update_settings_actions();
    rebuild_settings_page();
    (*form_.file_manager_app_shell_settings_actions_status).set_text(
        "Applying revision " + std::to_string(revision) + "…");
    post_worker(SettingsCommitWork{shared_from_this(), revision, std::move(changes)});
}

void Application::apply_settings_commit(
    fileman::orchestrator::SettingsCommitInfo commit) {
    settings_apply_in_flight_ = false;
    pending_settings_.clear();
    const std::string audit = commit.audit_id;
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
    (*form_.file_manager_app_shell_settings_actions_apply).set_enabled(
        ready && !pending_settings_.empty());
    (*form_.file_manager_app_shell_settings_actions_cancel).set_enabled(
        ready && !pending_settings_.empty());
    bool page_has_available = false;
    bool page_differs_from_default = false;
    if (ready) {
        for (const fileman::orchestrator::SettingSchemaFieldInfo& field : (*settings_schema_).fields) {
            if (field.presentation_tab != settings_tab_ ||
                field.availability != "available") {
                continue;
            }
            page_has_available = true;
            const SettingsValueMap::const_iterator pending = pending_settings_.find(field.id);
            const fileman::orchestrator::SettingValue* const committed = (*settings_snapshot_).find(field.id);
            const fileman::orchestrator::SettingValue& effective = pending != pending_settings_.end()
                ? (*pending).second
                : committed ? *committed : field.default_value;
            if (effective != field.default_value) {
                page_differs_from_default = true;
                break;
            }
        }
    }
    const bool reset_ready = ready && page_differs_from_default;
    (*form_.file_manager_app_shell_settings_actions_reset).set_enabled(reset_ready);
    (*form_.file_manager_app_shell_settings_actions_reset).set_accessible_description(reset_ready
            ? "Stage declared defaults for the current settings page"
            : !ready
                ? "Settings values are not ready"
                : !page_has_available
                    ? "This page has no available settings to reset"
                    : "Page values already match declared defaults");
    if (ready && !pending_settings_.empty()) {
        (*form_.file_manager_app_shell_settings_actions_status).set_text(
            std::to_string(pending_settings_.size()) +
            (pending_settings_.size() == 1 ? " pending change"
                                           : " pending changes"));
    } else if (ready &&
               (*form_.file_manager_app_shell_settings_actions_status).text().empty()) {
        (*form_.file_manager_app_shell_settings_actions_status).set_text(
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
    const std::string density = setting_as<std::string>(
        *settings_snapshot_, "appearance.density").value_or("comfortable");
    const bool compact = density == "compact";
    (*tree_).set_item_height(compact ? 26.0 : 28.0);
    (*objects_).set_icon_cell_size(compact ? gui_forms::Size{104.0, 86.0}
                                        : gui_forms::Size{112.0, 92.0});
    (*objects_).set_details_row_height(compact ? 28.0 : 30.0);
    (*correspondence_).set_compact_height(compact ? 41.0 : 45.0);
    (*correspondence_).set_expanded_height(compact ? 103.0 : 118.0);
    if (window_) {
        gui_forms::PresentationSettings presentation = (*window_).presentation_settings();
        presentation.text_scale = static_cast<double>(setting_as<std::uint64_t>(
            *settings_snapshot_, "appearance.text_scale_percent").value_or(100)) /
            100.0;
        presentation.reduced_motion = setting_as<bool>(
            *settings_snapshot_, "appearance.reduce_motion").value_or(false);
        (*window_).set_presentation_settings(presentation);
    }
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_visible(checksum_visible_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_expected_host).set_visible(checksum_visible_);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_terminal).set_visible(terminal_visible_);
    const std::string default_view = setting_as<std::string>(
        *settings_snapshot_, "navigation.default_view").value_or("icons");
    set_view_mode(default_view == "details"
                      ? gui_forms::ObjectViewMode::details
                      : gui_forms::ObjectViewMode::icons);
    update_command_state();
    if (old_hidden != show_hidden_ && !history_.empty()) {
        request_navigation(location_, false);
    } else if (const std::optional<DirectoryEntry> entry = selected_entry()) {
        request_preview(*entry);
    }
}

void Application::request_navigation(std::filesystem::path path,
                                     const bool add_history) {
    const std::optional<NavigationTarget> target = resolve_navigation_target(
        navigation_roots_, location_, home_root_, path);
    if (!target) {
        (*path_box_).set_text(path_utf8(location_));
        set_path_editing(false);
        set_status("Location not admitted",
                   "read-only roots are Home, Volumes, and explicit launch roots");
        return;
    }
    if (created_folder_rename_ && pending_selection_identity_ &&
        *pending_selection_identity_ == (*created_folder_rename_).identity) {
        pending_selection_identity_.reset();
    }
    created_folder_rename_.reset();
    search_generation_.fetch_add(1);
    search_loading_ = false;
    criteria_loading_ = false;
    search_cursor_.reset();
    criteria_cursor_.reset();
    search_order_.clear();
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
    if (search_showing_) {
        search_showing_ = false;
        filter_.clear();
        (*search_box_).set_text({});
    }
    if (criteria_showing_) criteria_showing_ = false;
    set_path_editing(false);
    const std::uint64_t generation = requested_generation_.fetch_add(1) + 1;
    const std::filesystem::path root = (*target).root;
    path = (*target).path;
    set_status("Reading " + path_utf8(path),
               "direct filesystem · " + navigation_label(root) +
                   " · read-only observation");
    post_worker(NavigationWork{shared_from_this(), root, std::move(path), generation, add_history, show_hidden_});
}

void Application::request_tree_expansion(std::filesystem::path path) {
    const std::optional<NavigationTarget> target = resolve_navigation_target(
        navigation_roots_, location_, home_root_, path);
    if (!target) return;
    const std::uint64_t generation = tree_generation_.fetch_add(1) + 1U;
    const std::filesystem::path root = (*target).root;
    path = (*target).path;
    post_worker(TreeExpansionWork{shared_from_this(), root, std::move(path), generation, show_hidden_});
}

void Application::apply_directory(DirectorySnapshot snapshot,
                                  const bool add_history) {
    if (snapshot.generation != requested_generation_.load() ||
        snapshot.generation < applied_generation_) {
        return;
    }
    if (created_folder_rename_ &&
        (*created_folder_rename_).navigation_generation == snapshot.generation &&
        (*created_folder_rename_).search_generation != search_generation_.load()) {
        created_folder_rename_.reset();
        pending_selection_identity_.reset();
        return;
    }
    applied_generation_ = snapshot.generation;
    if (!snapshot.available()) {
        created_folder_rename_.reset();
        (*path_box_).set_text(path_utf8(location_));
        set_status("Location unavailable", snapshot.error);
        return;
    }
    const bool returning_from_virtual_surface =
        (*correspondence_).effectively_visible() || (*criteria_console_).visible();
    const bool preserve_location_edit = created_folder_rename_ && (*breadcrumb_).editing() &&
        window_ && (*window_).focused_control() == path_box_;
    // The snapshot is also the authoritative cache input for rebuild_tree()
    // below. Preserve its paths until that retained model has consumed them.
    location_ = snapshot.location;
    navigation_root_ = snapshot.root;
    if (!preserve_location_edit) {
        (*path_box_).set_text(path_utf8(location_));
        rebuild_breadcrumb();
    }
    (*correspondence_).set_items({});
    (*correspondence_).set_visible(false);
    (*criteria_console_).set_visible(false);
    (*objects_).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_visible(false);
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_minimum_size({0, 0});
    (*form_.file_manager_app_shell_workspace_selection_content_heading_copy_title).set_text(
        leaf_name(location_));
    (*form_.file_manager_app_shell_workspace_selection_content_heading_count).set_text(
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
        (*objects_).selected_ids().begin(), (*objects_).selected_ids().end());
    entries_.clear();
    std::vector<gui_forms::ObjectViewItem> items{};
    items.reserve(snapshot.entries.size());
    std::string pending_selection_id{};
    for (const DirectoryEntry& entry : snapshot.entries) {
        if (pending_selection_identity_ &&
            entry.identity == *pending_selection_identity_) {
            pending_selection_id = entry.stable_id;
        }
        entries_.emplace(entry.stable_id, entry);
        std::string display_name = entry.name;
        if (!show_extensions_ && !entry.directory &&
            entry.kind != EntryKind::symlink) {
            const std::string stem = path_utf8(entry.path.stem());
            if (!stem.empty()) display_name = stem;
        }
        items.push_back({entry.stable_id, std::move(display_name), entry.secondary_text,
                         kind_text(entry), object_glyph(entry.kind), true,
                         std::string(house_art::object_key(entry.kind))});
    }
    publish_object_items(std::move(items));
    std::vector<std::string> retained_selection{};
    for (const std::string& stable_id : previous_selection) {
        if (entries_.contains(stable_id)) retained_selection.push_back(stable_id);
    }
    if (!pending_selection_id.empty()) {
        (*objects_).set_selected_id(pending_selection_id);
        pending_selection_identity_.reset();
    } else if (!retained_selection.empty()) {
        (*objects_).set_selected_ids(std::move(retained_selection));
    } else {
        (*objects_).clear_selection();
        update_selection({});
    }
    rebuild_tree(snapshot);

    (*form_.file_manager_app_shell_location_navigation_back).set_enabled(
        !history_.empty() && history_index_ > 0);
    (*form_.file_manager_app_shell_location_navigation_forward).set_enabled(
        !history_.empty() && history_index_ + 1 < history_.size());
    (*form_.file_manager_app_shell_location_navigation_up).set_enabled(
        location_ != navigation_root_);
    (*form_.file_manager_app_shell_workspace_sidebar_parent).set_enabled(
        location_ != navigation_root_);
    update_mutation_controls();
    update_command_state();
    if (returning_from_virtual_surface) focus_active_object_surface();
    update_browsing_status();
    if (created_folder_rename_) {
        const CreatedFolderRename pending = *created_folder_rename_;
        created_folder_rename_.reset();
        const std::optional<DirectoryEntry> selected = selected_entry();
        if (pending.navigation_generation == snapshot.generation &&
            pending.search_generation == search_generation_.load() &&
            !settings_open_ && !(*rename_box_).visible() && !transfer_in_flight_ &&
            !property_rename_in_flight_ && focused_is_not_text_editor() && selected &&
            (*selected).identity == pending.identity) {
            begin_rename();
        }
    }
}

void Application::show_tree_root_menu() {
    show_menu(tree_root_menu_, tree_root_mode_button_);
}

void Application::select_tree_root_mode(const std::filesystem::path& root) {
    const std::vector<std::filesystem::path>::const_iterator admitted = std::find(navigation_roots_.begin(),
                                    navigation_roots_.end(), root);
    if (admitted == navigation_roots_.end()) return;
    tree_root_mode_ = *admitted;
    tree_expanded_paths_.clear();
    tree_expanded_paths_.insert(path_generic_utf8(tree_root_mode_));
    const TreeEntriesMap::const_iterator current = tree_directory_entries_.find(path_generic_utf8(location_));
    DirectorySnapshot snapshot{};
    snapshot.root = navigation_root_;
    snapshot.location = location_;
    if (current != tree_directory_entries_.end()) snapshot.entries = (*current).second;
    rebuild_tree(snapshot);
    request_tree_expansion(tree_root_mode_);
}

void Application::rebuild_tree(const DirectorySnapshot& snapshot) {
    std::vector<gui_forms::TreeViewItem> items{};
    tree_directory_entries_[path_generic_utf8(snapshot.location)] = snapshot.entries;
    tree_locations_.clear();
    const std::string mode_label = navigation_label(tree_root_mode_);
    (*tree_root_mode_button_).set_text(mode_label);
    (*tree_root_mode_button_).set_accessible_description(
        "Folder tree root: " + path_utf8(tree_root_mode_) +
        ". Choose another available root");
    (*command_tree_home_).set_checked(tree_root_mode_ == home_root_);
    if (command_tree_volumes_) {
        (*command_tree_volumes_).set_checked(
            volumes_root_ && tree_root_mode_ == *volumes_root_);
    }
    for (std::size_t index = 0;
         index < command_tree_admitted_roots_.size(); ++index) {
        (*command_tree_admitted_roots_[index]).set_checked(
            tree_admitted_root_paths_[index] == tree_root_mode_);
    }
    std::string selected_id{};
    std::size_t root_index = static_cast<std::size_t>(
        std::distance(navigation_roots_.begin(),
                      std::find(navigation_roots_.begin(),
                                navigation_roots_.end(), tree_root_mode_)));
    for (const std::filesystem::path& root : {tree_root_mode_}) {
        std::string root_id{};
        if (root == home_root_) {
            root_id = "fm.location.home";
        } else if (volumes_root_ && root == *volumes_root_) {
            root_id = "fm.location.volumes";
        } else {
            root_id = "fm.location.admitted." + std::to_string(root_index);
        }
        const bool expanded = path_is_within(root, location_) ||
            tree_expanded_paths_.contains(path_generic_utf8(root));
        const std::string_view root_icon = root == home_root_
            ? house_art::key(house_art::Icon::home)
            : volumes_root_ && root == *volumes_root_
                ? house_art::key(house_art::Icon::drive)
                : house_art::key(house_art::Icon::folder);
        items.push_back({root_id, navigation_label(root), 0, true,
                         expanded, true, std::string(root_icon)});
        tree_locations_.emplace(root_id, root);
        if (location_ == root) selected_id = root_id;
        if (expanded) {
            append_tree_children(items, root, 1U, selected_id);
        }
    }
    tree_model_syncing_ = true;
    try {
        (*tree_).set_items(std::move(items));
        (*tree_).set_selected_id(selected_id);
    } catch (...) {
        tree_model_syncing_ = false;
        throw;
    }
    tree_model_syncing_ = false;
}

void Application::append_tree_children(
    std::vector<gui_forms::TreeViewItem>& items,
    const std::filesystem::path& parent,
    const std::size_t depth,
    std::string& selected_id) {
    const TreeEntriesMap::const_iterator cached = tree_directory_entries_.find(path_generic_utf8(parent));
    if (cached == tree_directory_entries_.end()) return;
    for (const DirectoryEntry& entry : (*cached).second) {
        if (!entry.directory) continue;
        const std::string tree_stable_id = "fm.tree." + entry.stable_id;
        const bool on_current_ancestry =
            entry.path == location_ || path_is_within(entry.path, location_);
        const bool expanded = on_current_ancestry ||
            tree_expanded_paths_.contains(path_generic_utf8(entry.path));
        items.push_back({tree_stable_id, entry.name, depth, true,
                         expanded, true,
                         std::string(house_art::key(house_art::Icon::folder))});
        tree_locations_[tree_stable_id] = entry.path;
        if (entry.path == location_) selected_id = tree_stable_id;
        if (expanded) {
            append_tree_children(items, entry.path, depth + 1U, selected_id);
        }
    }
}

std::string Application::navigation_label(
    const std::filesystem::path& root) const {
    if (root == home_root_) return "Home";
    if (volumes_root_ && root == *volumes_root_) return "Volumes";
    const std::string label = leaf_name(root);
    return label;
}

bool Application::mutation_scope_active() const {
    const bool active = operations_ && path_is_within(protected_root_, location_);
    return active;
}

bool Application::engine_search_available() const {
    const bool available = !engine_root_id_.empty() &&
        path_is_within(protected_root_, location_);
    return available;
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
    if (location_ == navigation_root_) return;
    const std::filesystem::path parent = location_.parent_path();
    request_navigation(parent, true);
}

void Application::toggle_view_mode() {
    set_view_mode(details_mode_ ? gui_forms::ObjectViewMode::icons
                                : gui_forms::ObjectViewMode::details);
}

void Application::apply_filter() {
    filter_ = std::string((*search_box_).text());
    if (filter_.empty()) {
        request_navigation(location_, false);
    } else {
        criteria_showing_ = false;
        criteria_loading_ = false;
        criteria_cursor_.reset();
        (*criteria_console_).set_visible(false);
        request_engine_search();
    }
}

void Application::toggle_criteria_mode() {
    if (criteria_showing_) {
        request_navigation(location_, false);
        return;
    }
    show_criteria();
}

void Application::show_criteria() {
    if (!engine_search_available()) {
        set_status("Criteria unavailable for this location",
                   "no committed Engine root is admitted here · direct navigation remains available");
        return;
    }
    prepare_criteria_surface();
    request_engine_criteria();
}

void Application::prepare_criteria_surface() {
    filter_.clear();
    (*search_box_).set_text({});
    search_showing_ = false;
    search_loading_ = false;
    search_cursor_.reset();
    criteria_cursor_.reset();
    search_order_.clear();
    entries_.clear();
    (*correspondence_).set_items({});
    (*correspondence_).set_visible(false);
    (*objects_).set_items({});
    (*objects_).clear_selection();
    update_selection({});
    (*objects_).set_visible(true);
    criteria_showing_ = true;
    (*criteria_console_).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_minimum_size({0, 27});
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_content_heading_copy_title).set_text(
        "Criteria · " + leaf_name(location_));
    (*form_.file_manager_app_shell_workspace_selection_content_heading_count).set_text(
        "waiting for committed catalogue");
    update_criteria_action_state();
    update_mutation_controls();
    update_command_state();
}

void Application::add_criteria_module() {
    std::vector<gui_forms::InstrumentModuleSpec> modules = (*criteria_rack_).modules();
    if (!contains_module(modules, "fm.criteria.kind")) {
        modules.push_back(kind_criteria_module());
    } else if (!contains_module(modules, "fm.criteria.modified")) {
        modules.push_back(modified_criteria_module());
    } else if (!contains_module(modules, "fm.criteria.size")) {
        modules.push_back(size_criteria_module());
    } else {
        return;
    }
    (*criteria_rack_).set_modules(std::move(modules));
    update_criteria_action_state();
    if (criteria_showing_) request_engine_criteria();
}

void Application::remove_criteria_module(const std::string_view module_id) {
    std::vector<gui_forms::InstrumentModuleSpec> modules = (*criteria_rack_).modules();
    const std::size_t removed = std::erase_if(modules, ModuleId{module_id});
    if (removed == 0U) return;
    (*criteria_rack_).set_modules(std::move(modules));
    update_criteria_action_state();
    if (criteria_showing_) request_engine_criteria();
}

void Application::update_criteria_action_state() {
    const std::vector<gui_forms::InstrumentModuleSpec>& modules = (*criteria_rack_).modules();
    const std::size_t enabled = static_cast<std::size_t>(std::count_if(
        modules.begin(), modules.end(),
        module_enabled));
    (*criteria_action_state_).set_text(
        std::to_string(enabled) +
        (enabled == 1U ? " exact filter" : " exact filters") +
        " · catalogue only");
    (*criteria_action_state_).set_accessible_description(
        "No content, plugin, fuzzy, or live-filesystem predicate is active");
    const bool all_present = modules.size() >= 3U;
    (*criteria_add_button_).set_enabled(!all_present);
    (*criteria_add_button_).set_accessible_description(all_present
        ? "All three admitted intrinsic metadata modules are already present"
        : "Add the next missing Kind, Modified, or Size module");
}

std::optional<std::vector<fileman::orchestrator::SearchExactFilter>>
Application::criteria_filters() {
    std::vector<fileman::orchestrator::SearchExactFilter> filters{};
    bool valid = true;
    for (const gui_forms::InstrumentModuleSpec& module : (*criteria_rack_).modules()) {
        if (!module.enabled) {
            (*criteria_rack_).set_module_state(
                module.stable_id, gui_forms::InstrumentModuleState::live,
                "disabled · no filter emitted");
            continue;
        }
        std::string validation{};
        std::string field{};
        std::string emitted{};
        if (module.stable_id == "fm.criteria.kind") {
            field = "kind";
            const std::string_view selected = criteria_field_value(module, "value");
            if (selected == "Files") emitted = "file";
            else if (selected == "Folders") emitted = "directory";
            else if (selected == "Symbolic links") emitted = "symlink";
            else if (selected == "Other") emitted = "other";
            else validation = "Choose Files, Folders, Symbolic links, or Other";
        } else if (module.stable_id == "fm.criteria.modified") {
            const std::optional<std::int64_t> date = parse_criteria_date(criteria_field_value(module, "value"));
            if (!date) {
                validation = "Use a real date from 1970 through 2261 as YYYY-MM-DD";
            } else {
                const std::string_view operation = criteria_field_value(module, "operator");
                field = operation == "after" ? "modified_after" :
                    operation == "before" ? "modified_before" : std::string{};
                if (field.empty()) validation = "Choose after or before";
                else emitted = std::to_string(*date);
            }
        } else if (module.stable_id == "fm.criteria.size") {
            const std::optional<std::int64_t> size = parse_criteria_size(criteria_field_value(module, "value"));
            if (!size) {
                validation = "Enter a nonnegative whole-byte count";
            } else {
                const std::string_view operation = criteria_field_value(module, "operator");
                field = operation == "at least" ? "size_min" :
                    operation == "at most" ? "size_max" : std::string{};
                if (field.empty()) validation = "Choose at least or at most";
                else emitted = std::to_string(*size);
            }
        } else {
            validation = "This criterion module is not admitted";
        }
        (*criteria_rack_).set_field_validation(
            module.stable_id, "value", validation);
        (*criteria_rack_).set_module_state(
            module.stable_id,
            validation.empty() ? gui_forms::InstrumentModuleState::live
                               : gui_forms::InstrumentModuleState::invalid,
            validation.empty() ? "committed exact filter" : validation);
        if (!validation.empty()) {
            valid = false;
        } else {
            filters.push_back({std::move(field), std::move(emitted)});
        }
    }
    if (!valid || filters.empty()) return std::nullopt;
    return filters;
}

void Application::request_engine_criteria(const bool next_page) {
    if (next_page && criteria_loading_) return;
    std::uint64_t generation{};
    std::optional<fileman::orchestrator::SearchCursorInfo> cursor{};
    if (next_page) {
        if (!criteria_showing_ || !criteria_cursor_) return;
        generation = search_generation_.load();
        cursor = criteria_cursor_;
    } else {
        generation = search_generation_.fetch_add(1U) + 1U;
        criteria_cursor_.reset();
    }
    std::optional<std::vector<fileman::orchestrator::SearchExactFilter>> filters = criteria_filters();
    update_criteria_action_state();
    if (!filters) {
        criteria_loading_ = false;
        criteria_cursor_.reset();
        entries_.clear();
        search_order_.clear();
        (*objects_).set_items({});
        (*objects_).clear_selection();
        update_selection({});
        (*form_.file_manager_app_shell_workspace_selection_content_heading_count).set_text(
            "invalid or disabled criteria");
        (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
        update_command_state();
        set_status("Criteria not applied",
                   "enable at least one locally valid exact metadata filter");
        return;
    }
    if (!engine_search_available()) {
        set_status("Criteria unavailable for this location",
                   "no committed Engine root is admitted here");
        return;
    }
    criteria_loading_ = true;
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
    update_command_state();
    const std::string engine_root_id = engine_root_id_;
    std::string relative = path_generic_utf8(std::filesystem::relative(location_, protected_root_));
    std::optional<std::string> relative_path{};
    if (!relative.empty() && relative != ".") relative_path = std::move(relative);
    std::uint32_t maximum_results = 100;
    if (settings_snapshot_) {
        const std::uint64_t configured = setting_as<std::uint64_t>(
            *settings_snapshot_, "search.result_limit").value_or(100);
        maximum_results = static_cast<std::uint32_t>(
            std::clamp<std::uint64_t>(configured, 25, 500));
    }
    set_status(next_page ? "Loading more criteria results…"
                         : "Applying committed exact criteria…",
               "Orchestrator → Engine catalogue · current subtree · no live fallback");
    post_worker(CriteriaWork{shared_from_this(), engine_root_id, relative_path, std::move(*filters), maximum_results, generation, cursor, next_page});
}

void Application::request_engine_search(const bool next_page) {
    const std::string query = filter_;
    if (query.empty() || (next_page && search_loading_)) return;
    if (!engine_search_available()) {
        set_status("Search unavailable for this location",
                   "no Engine root is admitted here · direct navigation remains available");
        return;
    }
    std::optional<fileman::orchestrator::SearchCursorInfo> cursor{};
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
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
    update_command_state();
    const std::string engine_root_id = engine_root_id_;
    std::string relative = path_generic_utf8(std::filesystem::relative(location_, protected_root_));
    std::optional<std::string> relative_path{};
    if (!relative.empty() && relative != ".") relative_path = std::move(relative);
    std::uint32_t maximum_results = 100;
    if (settings_snapshot_) {
        const std::uint64_t configured = setting_as<std::uint64_t>(
            *settings_snapshot_, "search.result_limit").value_or(100);
        maximum_results = static_cast<std::uint32_t>(
            std::clamp<std::uint64_t>(configured, 25, 500));
    }
    set_status(next_page ? "Loading more matches…" :
                           "Searching “" + query + "”…",
               "Orchestrator → installed Engine · current subtree · paged");
    post_worker(SearchWork{shared_from_this(), engine_root_id, relative_path, query, maximum_results, generation, cursor, next_page});
}

void Application::apply_engine_criteria(
    PreparedSearchPage prepared,
    std::vector<fileman::orchestrator::SearchExactFilter>,
    const std::uint64_t generation,
    const bool append) {
    if (prepared.cancelled || stopping_.load() || generation != search_generation_.load() || !criteria_showing_) return;
    fileman::orchestrator::SearchPageInfo& page = prepared.page;
    criteria_loading_ = false;
    if (page.source != "catalogue") {
        criteria_cursor_.reset();
        (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(false);
        update_command_state();
        set_status("Criteria source refused",
                   "filtered virtual folders require a committed catalogue; live fallback was not accepted");
        return;
    }
    if (!append) {
        entries_.clear();
        search_order_.clear();
    }
    const std::vector<std::string> previous_selection(
        (*objects_).selected_ids().begin(), (*objects_).selected_ids().end());
    const std::size_t rejected = prepared.rejected;
    search_coverage_.observe(page, rejected, append);
    const std::string coverage_notice = search_coverage_.describe(page);
    std::size_t duplicate{};
    for (DirectoryEntry& entry : prepared.entries) {
        const std::string stable_id = entry.stable_id;
        if (entries_.contains(stable_id)) {
            ++duplicate;
            continue;
        }
        entries_.emplace(stable_id, std::move(entry));
        search_order_.push_back(stable_id);
    }
    std::vector<gui_forms::ObjectViewItem> items{};
    items.reserve(search_order_.size());
    for (const std::string& stable_id : search_order_) {
        const EntryMap::const_iterator found = entries_.find(stable_id);
        if (found == entries_.end()) continue;
        const DirectoryEntry& entry = (*found).second;
        std::string display_name = entry.name;
        if (!show_extensions_ && !entry.directory &&
            entry.kind != EntryKind::symlink) {
            const std::string stem = path_utf8(entry.path.stem());
            if (!stem.empty()) display_name = stem;
        }
        items.push_back({stable_id, std::move(display_name),
                         entry.secondary_text, kind_text(entry),
                         object_glyph(entry.kind), true,
                         std::string(house_art::object_key(entry.kind))});
    }
    publish_object_items(std::move(items));
    (*correspondence_).set_items({});
    (*correspondence_).set_visible(false);
    (*objects_).set_visible(true);
    (*criteria_console_).set_visible(true);
    std::vector<std::string> retained_selection{};
    for (const std::string& stable_id : previous_selection) {
        if (entries_.contains(stable_id)) retained_selection.push_back(stable_id);
    }
    if (!retained_selection.empty()) {
        (*objects_).set_selected_ids(std::move(retained_selection));
    } else {
        (*objects_).clear_selection();
        update_selection({});
    }
    criteria_cursor_ = std::move(page.cursor);
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(!page.complete && criteria_cursor_.has_value());
    (*form_.file_manager_app_shell_workspace_selection_content_heading_copy_title).set_text(
        "Criteria · " + leaf_name(location_));
    (*form_.file_manager_app_shell_workspace_selection_content_heading_count).set_text(
        std::to_string(entries_.size()) +
        (entries_.size() == 1U ? " object" : " objects"));
    (*criteria_title_).set_text(
        "CURRENT SUBTREE   ·   EXACT VIRTUAL FOLDER   ·   CATALOGUE GENERATION " +
        (page.generation ? std::to_string(*page.generation) : "UNREPORTED"));
    update_mutation_controls();
    update_command_state();
    std::string provenance = "installed Engine catalogue";
    if (page.generation) {
        provenance += " · generation " + std::to_string(*page.generation);
    } else {
        provenance += " · generation unreported";
    }
    provenance += " · exact intrinsic metadata filters";
    provenance += coverage_notice;
    if (append) provenance += " · appended page";
    if (rejected != 0U) {
        provenance += " · " + std::to_string(rejected) +
            (rejected == 1U ? " stale/out-of-root result refused"
                             : " stale/out-of-root results refused");
    }
    if (duplicate != 0U) {
        provenance += " · " + std::to_string(duplicate) +
            (duplicate == 1U ? " duplicate skipped" : " duplicates skipped");
    }
    const std::string result_status = search_coverage_.result_status(entries_.size(), true, page);
    set_status(result_status, std::move(provenance));
}

void Application::apply_engine_search(
    PreparedSearchPage prepared,
    std::string query,
    const std::uint64_t generation,
    const bool append) {
    if (prepared.cancelled || stopping_.load() || generation != search_generation_.load() ||
        std::string((*search_box_).text()) != query) return;
    fileman::orchestrator::SearchPageInfo& page = prepared.page;
    search_loading_ = false;
    search_showing_ = true;
    if (!append) {
        entries_.clear();
        search_order_.clear();
    }
    const std::vector<std::string> previous_selection(
        (*objects_).selected_ids().begin(), (*objects_).selected_ids().end());
    const std::size_t rejected = prepared.rejected;
    search_coverage_.observe(page, rejected, append);
    const std::string coverage_notice = search_coverage_.describe(page);
    std::size_t duplicate{};
    for (DirectoryEntry& entry : prepared.entries) {
        const std::string stable_id = entry.stable_id;
        if (entries_.contains(stable_id)) {
            ++duplicate;
            continue;
        }
        entries_.emplace(stable_id, std::move(entry));
        search_order_.push_back(stable_id);
    }
    std::vector<gui_forms::ObjectViewItem> items{};
    std::vector<gui_forms::CorrespondenceItem> correspondence_items{};
    items.reserve(search_order_.size());
    correspondence_items.reserve(search_order_.size());
    const std::string evidence_source = page.source == "catalogue"
        ? "Local index"
        : "Live filesystem";
    const std::string generation_detail = page.generation
        ? "Engine generation " + std::to_string(*page.generation)
        : "Live filesystem observation; no catalogue generation claimed";
    for (const std::string& stable_id : search_order_) {
        const EntryMap::const_iterator found = entries_.find(stable_id);
        if (found == entries_.end()) continue;
        const DirectoryEntry& entry = (*found).second;
        std::string display_name = entry.name;
        if (!show_extensions_ && !entry.directory &&
            entry.kind != EntryKind::symlink) {
            const std::string stem = path_utf8(entry.path.stem());
            if (!stem.empty()) display_name = stem;
        }
        items.push_back({stable_id, std::move(display_name), entry.secondary_text,
                         kind_text(entry), object_glyph(entry.kind), true,
                         std::string(house_art::object_key(entry.kind))});
        std::string relative_path = path_generic_utf8(entry.path.lexically_relative(protected_root_));
        if (relative_path.empty()) relative_path = path_generic_utf8(entry.path);
        gui_forms::CorrespondenceItem correspondence{
            stable_id,
            entry.name,
            relative_path,
            kind_text(entry) + " · " + entry.secondary_text,
            "No content claim. Provider matched local name or path data.",
            page.source == "catalogue" ? "CATALOGUE" : "LIVE",
            "LOCAL EVIDENCE",
            {evidence_source, "Filesystem observation"},
            generation_detail,
            object_glyph(entry.kind),
            true,
            false,
            false,
            {query},
        };
        correspondence_items.push_back(std::move(correspondence));
    }
    publish_object_items(std::move(items));
    (*correspondence_).set_items(std::move(correspondence_items));
    (*objects_).set_visible(false);
    (*correspondence_).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_minimum_size({0, 27});
    (*form_.file_manager_app_shell_workspace_selection_content_heading).set_visible(true);
    std::vector<std::string> retained_selection{};
    for (const std::string& stable_id : previous_selection) {
        if (entries_.contains(stable_id)) retained_selection.push_back(stable_id);
    }
    if (!retained_selection.empty()) {
        (*objects_).set_selected_ids(std::move(retained_selection));
        (*correspondence_).set_selected_id((*objects_).selected_id());
    } else {
        (*objects_).clear_selection();
        (*correspondence_).set_selected_id({});
        update_selection({});
    }
    search_cursor_ = std::move(page.cursor);
    (*form_.file_manager_app_shell_workspace_selection_content_heading_more_results).set_enabled(
        !page.complete && search_cursor_.has_value());
    (*form_.file_manager_app_shell_workspace_selection_content_heading_copy_title).set_text(
        "Search · " + query);
    (*form_.file_manager_app_shell_workspace_selection_content_heading_count).set_text(
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
    provenance += coverage_notice;
    if (append) provenance += " · appended page";
    if (rejected != 0) {
        provenance += " · " + std::to_string(rejected) +
            " stale/out-of-root result" + (rejected == 1 ? " refused" : "s refused");
    }
    if (duplicate != 0) {
        provenance += " · " + std::to_string(duplicate) +
            " duplicate" + (duplicate == 1 ? " skipped" : "s skipped");
    }
    const std::string result_status = search_coverage_.result_status(entries_.size(), false, page);
    set_status(result_status, std::move(provenance));
}

std::optional<DirectoryEntry> Application::selected_entry() const {
    if ((*objects_).selected_ids().size() != 1U) return std::nullopt;
    const EntryMap::const_iterator found = entries_.find(
        std::string((*objects_).selected_ids().front()));
    if (found == entries_.end()) return std::nullopt;
    return (*found).second;
}

void Application::reset_preview() {
    if (preview_image_id_.value != 0U && window_) {
        (void)(*window_).remove_image(preview_image_id_);
        preview_image_id_ = {};
    }
    (*preview_picture_).clear_image();
    (*preview_picture_).set_visible(false);
    (*preview_text_).set_text({});
    (*preview_text_).set_visible(false);
    (*preview_house_icon_).set_visible(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_visible(true);
}

void Application::request_preview(const DirectoryEntry& entry) {
    const std::uint64_t generation = preview_generation_.fetch_add(1) + 1U;
    reset_preview();
    (*preview_house_icon_).set_image_key(
        std::string(house_art::object_key(entry.kind)));
    (*preview_house_icon_).set_accessible_name(
        kind_text(entry) + " material icon");
    (*preview_house_icon_).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_visible(false);
    if (!builtin_previews_enabled_) {
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
            kind_text(entry) + " · built-in preview disabled");
        return;
    }
    if (entry.directory || entry.kind == EntryKind::symlink) return;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
        kind_text(entry) + " · loading preview…");
    const std::filesystem::path root = navigation_root_;
    post_worker(PreviewWork{shared_from_this(), root, entry, generation});
}

void Application::apply_preview(PreviewResult result, std::string stable_id,
                                const std::uint64_t generation) {
    if (generation != preview_generation_.load()) return;
    const std::optional<DirectoryEntry> selected = selected_entry();
    if (!selected || (*selected).stable_id != stable_id ||
        (!(*selected).identity.same_revision(result.identity) &&
         (result.kind == PreviewKind::text ||
          result.kind == PreviewKind::png))) {
        return;
    }
    reset_preview();
    if (result.kind == PreviewKind::text) {
        const bool empty_text = result.text_utf8.empty();
        std::string coverage{"Text excerpt · UTF-8"};
        if (empty_text) {
            result.text_utf8 = "Empty text file";
            coverage = "Empty UTF-8 text file";
        } else if (result.text_truncated) {
            coverage = "Text excerpt · 64 KiB limit";
        }
        (*preview_text_).set_text(std::move(result.text_utf8));
        (*preview_text_).set_visible(true);
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_visible(false);
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
            std::move(coverage));
        return;
    }
    if (result.kind == PreviewKind::png && window_) {
        const gui_forms::ImageLoadResult loaded = (*window_).load_png(result.png_bytes);
        if (loaded) {
            preview_image_id_ = loaded.image;
            (*preview_picture_).set_image(loaded.image);
            (*preview_picture_).set_visible(true);
            (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_visible(false);
            (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
                "Image · bounded PNG preview");
            return;
        }
        result.code = "png-resource-rejected";
        const std::string_view reason = gui_forms::image_resource_error_name(loaded.error);
        result.message = "This PNG could not be displayed. Reason: ";
        result.message.append(reason);
    }
    (*preview_text_).set_text(result.message);
    (*preview_text_).set_visible(true);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_visible(false);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
        kind_text(*selected) + " · preview unavailable");
}

void Application::update_selection(const std::string_view stable_id) {
    update_adaptive_preview();
    (*property_list_).set_visible(!(*objects_).selected_ids().empty());
    update_browsing_status();
    pending_delete_id_.reset();
    (*property_list_).set_value("fm.property.expected-sha256", {});
    if ((*rename_box_).visible()) cancel_rename();
    if (checksum_in_flight_) {
        checksum_generation_.fetch_add(1);
        checksum_in_flight_ = false;
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_text("SHA-256");
    }
    update_mutation_controls();
    if ((*objects_).selected_ids().size() > 1U) {
        preview_generation_.fetch_add(1);
        reset_preview();
        const std::size_t count = (*objects_).selected_ids().size();
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_text("MULTI");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_name).set_text(
            std::to_string(count) + " objects selected");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
            "Properties with multiple values are not synthesized");
        (*property_list_).set_value("fm.property.name", "—");
        if (const gui_forms::Control::Ptr editor = (*property_list_).editor("fm.property.name")) {
            (*editor).set_enabled(false);
        }
        (*property_list_).set_value("fm.property.kind", "Multiple kinds");
        (*property_list_).set_value("fm.property.location", path_utf8(location_));
        (*property_list_).set_value("fm.property.size", "Multiple values");
        (*property_list_).set_value("fm.property.modified", "Multiple values");
        update_command_state();
        return;
    }
    const EntryMap::const_iterator found = entries_.find(std::string(stable_id));
    if (found == entries_.end()) {
        preview_generation_.fetch_add(1);
        reset_preview();
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_surface_glyph).set_text("—");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_name).set_text(
            "Nothing selected");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
            "Choose an item to inspect it");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_path).set_text("Path · —");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_size).set_text("Size · —");
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_modified).set_text(
            "Modified · —");
        (*property_list_).set_value("fm.property.name", "—");
        if (const gui_forms::Control::Ptr editor = (*property_list_).editor("fm.property.name")) {
            (*editor).set_enabled(false);
        }
        (*property_list_).set_value("fm.property.kind", "—");
        (*property_list_).set_value("fm.property.location", "—");
        (*property_list_).set_value("fm.property.size", "—");
        (*property_list_).set_value("fm.property.modified", "—");
        update_command_state();
        return;
    }
    const DirectoryEntry& entry = (*found).second;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_name).set_text(entry.name);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_preview_kind).set_text(
        kind_text(entry));
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_path).set_text(
        "Path · " + path_utf8(entry.path));
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_size).set_text(
        "Size · " + entry.secondary_text);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_modified).set_text(
        "Modified · " + entry.modified_text);
    (*property_list_).set_value("fm.property.name", entry.name);
    (*property_list_).set_value("fm.property.kind", kind_text(entry));
    (*property_list_).set_value("fm.property.location", path_utf8(entry.path));
    (*property_list_).set_value("fm.property.size", entry.secondary_text);
    (*property_list_).set_value("fm.property.modified", entry.modified_text);
    update_command_state();
    request_preview(entry);
}

void Application::update_mutation_controls() {
    const bool one_selected = (*objects_).selected_ids().size() == 1;
    const std::optional<DirectoryEntry> entry = selected_entry();
    const bool available = mutation_scope_active() && !(*rename_box_).visible() &&
        !transfer_in_flight_ && !property_rename_in_flight_;
    if (const gui_forms::Control::Ptr editor = (*property_list_).editor("fm.property.name")) {
        (*editor).set_enabled(available && one_selected);
    }
    (*form_.file_manager_app_shell_commands_rename).set_enabled(
        available && one_selected);
    (*form_.file_manager_app_shell_commands_selection_group_actions_copy).set_enabled(
        available && one_selected);
    (*form_.file_manager_app_shell_commands_move).set_enabled(
        available && one_selected);
    (*form_.file_manager_app_shell_commands_paste).set_enabled(
        available && pending_transfer_.has_value() &&
        (*pending_transfer_).entry.path.parent_path() != location_);
    (*form_.file_manager_app_shell_commands_selection_group_actions_delete).set_enabled(
        available && one_selected);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_open).set_enabled(
        entry.has_value() && (*entry).kind != EntryKind::symlink);
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_enabled(
        checksum_visible_ && (checksum_in_flight_ ||
        (entry.has_value() && !(*entry).directory &&
         (*entry).kind != EntryKind::symlink)));
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_terminal).set_enabled(
#if defined(__linux__)
        false);
#else
        terminal_visible_);
#endif
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_copy_path).set_enabled(
        true);
    update_command_state();
}

void Application::request_checksum() {
    if (checksum_in_flight_) {
        checksum_generation_.fetch_add(1);
        checksum_in_flight_ = false;
        (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_text("SHA-256");
        update_mutation_controls();
        set_status("Checksum cancellation requested",
                   "no partial digest will be published");
        return;
    }
    const std::optional<DirectoryEntry> entry = selected_entry();
    if (!entry || (*entry).directory || (*entry).kind == EntryKind::symlink) return;
    const std::string expected = std::string((*expected_checksum_box_).text());
    const std::uint64_t generation = checksum_generation_.fetch_add(1) + 1U;
    checksum_in_flight_ = true;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_text("Cancel hash");
    update_mutation_controls();
    set_status("Computing SHA-256 · " + (*entry).name,
               "bounded 256 KiB stream · stable revision required");
    const std::filesystem::path root = navigation_root_;
    post_worker(ChecksumWork{shared_from_this(), root, *entry, expected, generation});
}

void Application::apply_checksum(ChecksumResult result, std::string expected,
                                 const std::uint64_t generation) {
    if (generation != checksum_generation_.load()) return;
    checksum_in_flight_ = false;
    (*form_.file_manager_app_shell_workspace_selection_inspector_facts_commands_checksum).set_text("SHA-256");
    update_mutation_controls();
    if (!result.succeeded()) {
        set_status(result.terminal == ChecksumTerminal::cancelled
                       ? "Checksum cancelled"
                       : "Checksum unavailable · " + result.code,
                   result.message);
        if (result.terminal != ChecksumTerminal::cancelled && window_ &&
            (*window_).host_services()) {
            gui_forms::HostMessageDialogRequest message{};
            message.title = "SHA-256 unavailable";
            message.message = result.message + "\n\nCode: " + result.code +
                "\nObject: " + path_utf8(result.path);
            message.buttons = gui_forms::HostMessageButtons::ok;
            message.icon = gui_forms::HostMessageIcon::warning;
            gui_forms::HostDialogRequest request{};
            request.request_id = next_host_request_id_++;
            request.owner_id = "file-manager.checksum";
            request.payload = std::move(message);
            (void)(*(*window_).host_services()).show_dialog(request);
        }
        return;
    }

    expected.erase(std::remove_if(expected.begin(), expected.end(),
        whitespace_byte),
        expected.end());
    std::transform(expected.begin(), expected.end(), expected.begin(),
                   lowercase_byte);
    const bool expected_valid = expected.empty() ||
        (expected.size() == 64U &&
         std::all_of(expected.begin(), expected.end(), hexadecimal_byte));
    std::string comparison{};
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
    if (!window_ || !(*window_).host_services()) return;
    gui_forms::HostMessageDialogRequest message{};
    message.title = "SHA-256 · " + path_utf8(result.path.filename());
    message.message = "Object: " + path_utf8(result.path) +
        "\nBytes read: " + std::to_string(result.bytes_read) +
        "\nAlgorithm: SHA-256\n\n" + result.digest_hex + comparison +
        "\n\nCopy this digest to the clipboard?";
    message.buttons = gui_forms::HostMessageButtons::yes_no;
    message.icon = expected_valid && !expected.empty() &&
            expected != result.digest_hex
        ? gui_forms::HostMessageIcon::warning
        : gui_forms::HostMessageIcon::information;
    message.default_choice = gui_forms::HostDialogChoice::yes;
    gui_forms::HostDialogRequest request{};
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.checksum";
    request.payload = std::move(message);
    const gui_forms::HostDialogResult dialog = (*(*window_).host_services()).show_dialog(request);
    const gui_forms::HostMessageDialogResult* const answer = std::get_if<gui_forms::HostMessageDialogResult>(
        &dialog.payload);
    if (answer && (*answer).outcome == gui_forms::HostDialogOutcome::accepted &&
        (*answer).choice == gui_forms::HostDialogChoice::yes) {
        const gui_forms::HostServiceStatus copied = (*(*window_).host_services()).write_clipboard_text(
            result.digest_hex);
        set_status(copied.accepted() ? "SHA-256 copied" :
                                      "Clipboard unavailable",
                   copied.accepted() ? "64 lowercase hexadecimal characters"
                                     : "digest remains visible in the dialog");
    }
}

void Application::request_open() {
    const std::optional<DirectoryEntry> entry = selected_entry();
    if (!entry || (*entry).kind == EntryKind::symlink) return;
    if ((*entry).directory) {
        request_navigation((*entry).path, true);
        return;
    }
    run_platform_command(PlatformCommandKind::open_default, *entry);
}

DirectoryEntry Application::terminal_target() const {
    if (std::optional<DirectoryEntry> entry = selected_entry(); entry && (*entry).directory &&
            (*entry).kind != EntryKind::symlink) {
        return *entry;
    }
    const DirectoryEntry location{"fm.location.current", location_,
        leaf_name(location_), "Folder", "Current location",
        observe_identity(location_), EntryKind::folder, true};
    return location;
}

void Application::request_terminal() {
    run_platform_command(PlatformCommandKind::open_terminal_here,
                         terminal_target());
}

void Application::copy_current_path() {
    const std::optional<DirectoryEntry> entry = selected_entry();
    const std::filesystem::path path = entry ? (*entry).path : location_;
    if (!window_ || !(*window_).host_services()) {
        set_status("Clipboard unavailable", path_utf8(path));
        return;
    }
    const gui_forms::HostServiceStatus copied = (*(*window_).host_services()).write_clipboard_text(
        path_utf8(path));
    set_status(copied.accepted() ? "Path copied" : "Clipboard unavailable",
               path_utf8(path));
}

void Application::run_platform_command(const PlatformCommandKind kind,
                                       const DirectoryEntry& entry) {
    PlatformCommandPlan plan;
    PlatformCommandResult planned = make_platform_command_plan(
        kind, navigation_root_, entry.path, entry.identity, plan);
    if (planned.code != "ok") {
        apply_platform_command(kind, std::move(planned), entry.path);
        return;
    }
    set_status(kind == PlatformCommandKind::open_terminal_here
                   ? "Opening Terminal…" : "Opening " + entry.name + "…",
               "native platform launcher · identity revalidation");
    post_worker(PlatformCommandWork{shared_from_this(), kind, std::move(plan), entry.path});
}

void Application::apply_platform_command(const PlatformCommandKind kind,
                                         PlatformCommandResult result,
                                         std::filesystem::path path) {
    set_status(result.launched ? result.message :
                   "Native launch refused · " + result.code,
               result.launched ? "no shell · no elevation · fixed argv"
                               : result.message);
    if (result.launched || !window_ || !(*window_).host_services()) return;
    gui_forms::HostMessageDialogRequest message{};
    message.title = kind == PlatformCommandKind::open_terminal_here
        ? "Terminal unavailable" : "Open unavailable";
    message.message = result.message + "\n\nCode: " + result.code +
        "\nPath: " + path_utf8(path);
    message.buttons = gui_forms::HostMessageButtons::ok;
    message.icon = gui_forms::HostMessageIcon::warning;
    gui_forms::HostDialogRequest request{};
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.platform-command";
    request.payload = std::move(message);
    (void)(*(*window_).host_services()).show_dialog(request);
}

void Application::request_create_folder() {
    if (!mutation_scope_active()) return;
    const std::filesystem::path parent = location_;
    ++create_folder_request_generation_;
    const CreateFolderContext context{requested_generation_.load(),
        search_generation_.load(), create_folder_request_generation_};
    created_folder_rename_.reset();
    set_status("Creating a folder", "protected operation · collision-safe");
    post_worker(CreateFolderWork{shared_from_this(), parent, context});
}

void Application::apply_created_folder(OperationResult result, const CreateFolderContext& context) {
    if (!result.succeeded()) {
        apply_operation(std::move(result));
        return;
    }
    const std::filesystem::path parent = result.resulting_path.parent_path();
    const bool context_current = context.navigation_generation == requested_generation_.load() &&
        context.search_generation == search_generation_.load() &&
        context.request_generation == create_folder_request_generation_ &&
        parent == location_ && !settings_open_ && !(*rename_box_).visible() &&
        !transfer_in_flight_ && !property_rename_in_flight_ && focused_is_not_text_editor();
    if (!context_current) {
        // Creation has committed, but it cannot revoke a newer navigation or edit.
        undo_available_ = result.undo_available;
        update_mutation_controls();
        set_status("Folder created", path_utf8(result.resulting_path));
        return;
    }
    const ObjectIdentity identity = result.identity;
    apply_operation(std::move(result));
    if (identity.available()) {
        created_folder_rename_ = CreatedFolderRename{identity,
            requested_generation_.load(), search_generation_.load()};
    }
}

void Application::begin_rename() {
    if (!mutation_scope_active() || (*objects_).selected_ids().size() != 1) return;
    const std::string stable_id = std::string((*objects_).selected_ids().front());
    const EntryMap::const_iterator found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    rename_target_id_ = stable_id;
    pending_delete_id_.reset();
    (*rename_box_).set_text((*found).second.name);
    (*breadcrumb_).set_visible(false);
    (*rename_box_).set_visible(true);
    compact_search_active_ = false;
    update_adaptive_layout((*form_.file_manager_app_shell).committed_arranged_bounds());
    const std::size_t basename_extent = rename_basename_extent((*found).second);
    (*rename_box_).select(gui_forms::Utf8Offset{0U}, gui_forms::Utf8Offset{basename_extent});
    update_mutation_controls();
    if (window_) (*window_).request_focus(rename_box_);
    set_status("Rename " + (*found).second.name,
               "Enter commits · Escape cancels · collision-safe");
}

void Application::commit_rename(std::string basename) {
    if (!mutation_scope_active() || !rename_target_id_) return;
    const EntryMap::const_iterator found = entries_.find(*rename_target_id_);
    if (found == entries_.end()) {
        cancel_rename();
        return;
    }
    const DirectoryEntry entry = (*found).second;
    rename_target_id_.reset();
    (*rename_box_).set_visible(false);
    set_path_editing(false);
    update_mutation_controls();
    focus_active_object_surface();
    if (basename == entry.name) {
        set_status("Name unchanged", entry.name);
        return;
    }
    set_status("Renaming " + entry.name,
               "revalidating no-follow filesystem identity");
    post_worker(RenameWork{shared_from_this(), entry, std::move(basename)});
}

void Application::commit_property_name(std::string basename) {
    const std::optional<DirectoryEntry> entry = selected_entry();
    if (!entry) {
        (*property_list_).set_value("fm.property.name", "—");
        return;
    }
    if (!mutation_scope_active() || property_rename_in_flight_) {
        (*property_list_).set_value("fm.property.name", (*entry).name);
        return;
    }
    if (basename == (*entry).name) return;

    property_rename_in_flight_ = true;
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status("Renaming " + (*entry).name,
               "property edit · revalidating no-follow filesystem identity");
    post_worker(PropertyRenameWork{shared_from_this(), *entry, std::move(basename)});
}

void Application::cancel_rename() {
    rename_target_id_.reset();
    (*rename_box_).set_visible(false);
    set_path_editing(false);
    update_mutation_controls();
    focus_active_object_surface();
}

void Application::capture_transfer(const bool move) {
    if (!mutation_scope_active() || transfer_in_flight_ ||
        (*objects_).selected_ids().size() != 1) {
        return;
    }
    const std::string stable_id = std::string((*objects_).selected_ids().front());
    const EntryMap::const_iterator found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    transfer_generation_.fetch_add(1);
    pending_transfer_ = PendingTransfer{(*found).second, move};
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status((move ? "Move" : "Copy") + std::string(" captured · ") +
                   (*found).second.name,
               "navigate to a different folder and press Paste");
}

void Application::paste_transfer() {
    if (!mutation_scope_active() || transfer_in_flight_ || !pending_transfer_ ||
        (*pending_transfer_).entry.path.parent_path() == location_) {
        return;
    }
    const PendingTransfer transfer = *pending_transfer_;
    const std::filesystem::path destination = location_;
    const std::uint64_t generation = transfer_generation_.fetch_add(1) + 1;
    transfer_in_flight_ = true;
    transfer_cancellable_ = !transfer.move;
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status(std::string(transfer.move ? "Moving " : "Copying ") +
                   transfer.entry.name,
               transfer.move ? "same-volume identity-preserving publication"
                             : "staged no-follow copy · no overwrite");
    post_worker(TransferWork{shared_from_this(), transfer, destination, generation});
}

void Application::cancel_transfer() {
    if (!transfer_in_flight_ || !transfer_cancellable_) return;
    const std::uint64_t generation = transfer_generation_.load();
    if (cancelled_transfer_generation_.load() == generation) return;
    // Keep this job's identity and busy state until its authoritative result.
    // A cancellation request that arrives after publication cannot undo a copy.
    cancelled_transfer_generation_.store(generation);
    update_mutation_controls();
    set_status("Copy cancellation requested", "Waiting for current filesystem work");
}

void Application::observe_object_pointer(const gui_forms::PointerEvent& event) {
    if (!mutation_scope_active() || transfer_in_flight_) return;
    const std::string_view item_id = (*objects_).item_id_at(event.position);
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
        const std::string_view target_id = item_id;
        if (target_id == pointer_drag_hover_id_) return;
        pointer_drag_hover_id_ = std::string(target_id);
        const EntryMap::const_iterator source = entries_.find(std::string(pointer_drag_.source_id()));
        const EntryMap::const_iterator target = entries_.find(pointer_drag_hover_id_);
        if (source != entries_.end() && target != entries_.end() &&
            (*target).second.directory && (*target).first != (*source).first) {
            set_status("Drop " + (*source).second.name + " on " +
                           (*target).second.name,
                       "ordinary drag moves · hold Option while dropping to copy");
        } else if (source != entries_.end()) {
            set_status("Dragging " + (*source).second.name,
                       "drop on a visible folder · Option copies");
        }
        return;
    }
    if (event.action != gui_forms::PointerAction::up ||
        event.button != gui_forms::PointerButton::primary) {
        return;
    }

    const std::optional<InternalDropIntent> intent = pointer_drag_.observe({
        DragPointerPhase::up, event.position.x, event.position.y, item_id,
        gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::alt)});
    pointer_drag_hover_id_.clear();
    if (!intent) return;
    const EntryMap::const_iterator source = entries_.find((*intent).source_id);
    const EntryMap::const_iterator target = entries_.find((*intent).destination_id);
    if (source == entries_.end() || target == entries_.end() ||
        !(*target).second.directory || (*target).first == (*source).first) {
        set_status("Drag cancelled", "no destination folder accepted the object");
        return;
    }
    request_internal_drop(
        (*source).second, (*target).second,
        (*intent).copy);
}

void Application::request_internal_drop(const DirectoryEntry& source,
                                        const DirectoryEntry& destination,
                                        const bool copy) {
    if (!mutation_scope_active() || transfer_in_flight_ ||
        !destination.directory ||
        !path_is_within(protected_root_, source.path) ||
        !path_is_within(protected_root_, destination.path)) {
        return;
    }
    pending_transfer_.reset();
    const std::uint64_t generation = transfer_generation_.fetch_add(1) + 1;
    transfer_in_flight_ = true;
    transfer_cancellable_ = copy;
    pending_delete_id_.reset();
    update_mutation_controls();
    set_status(std::string(copy ? "Copying " : "Moving ") + source.name +
                   " to " + destination.name,
               copy ? "drag copy · staged no-overwrite publication"
                    : "drag move · same-volume identity publication");
    post_worker(InternalDropWork{shared_from_this(), source, destination, copy, generation});
}

void Application::request_quarantine() {
    if (!mutation_scope_active() || (*objects_).selected_ids().size() != 1) return;
    const std::string stable_id = std::string((*objects_).selected_ids().front());
    const EntryMap::const_iterator found = entries_.find(stable_id);
    if (found == entries_.end()) return;
    if (!pending_delete_id_ || *pending_delete_id_ != stable_id) {
        pending_delete_id_ = stable_id;
        set_status("Delete armed · press Delete again for " + (*found).second.name,
                   "recoverable quarantine · no permanent removal");
        return;
    }
    pending_delete_id_.reset();
    const DirectoryEntry entry = (*found).second;
    set_status("Moving " + entry.name + " to quarantine",
               "revalidating no-follow filesystem identity");
    post_worker(QuarantineWork{shared_from_this(), entry});
}

void Application::request_undo() {
    if (!operations_) return;
    set_status("Undoing the last operation",
               "identity and destination revalidation");
    post_worker(UndoWork{shared_from_this()});
}

void Application::apply_operation(OperationResult result) {
    // Result is the worker-owned service snapshot, published through the UI queue.
    undo_available_ = result.undo_available;
    update_mutation_controls();
    if (result.terminal == OperationTerminal::cancelled) {
        if (result.recoverable_object_retained) {
            set_status("Copy cancelled · cleanup incomplete", path_utf8(result.resulting_path));
            show_operation_failure(result);
        } else {
            set_status("Copy cancelled", result.message);
        }
        return;
    }
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
    transfer_cancellable_ = false;
    if (generation == transfer_generation_.load() && result.succeeded()) {
        pending_transfer_.reset();
    }
    update_mutation_controls();
    apply_operation(std::move(result));
}

void Application::show_operation_failure(const OperationResult& result) {
    if ((result.terminal == OperationTerminal::cancelled &&
         !result.recoverable_object_retained) || !window_ ||
        !(*window_).host_services()) {
        return;
    }
    gui_forms::HostMessageDialogRequest message{};
    message.title = result.terminal == OperationTerminal::cancelled
        ? "Copy cleanup incomplete" : "File operation refused";
    message.message = result.message + "\n\nCode: " + result.code;
    if (!result.original_path.empty()) {
        message.message += "\nObject: " + path_utf8(result.original_path);
    }
    if (!result.resulting_path.empty()) {
        message.message += "\nDestination: " + path_utf8(result.resulting_path);
    }
    message.buttons = gui_forms::HostMessageButtons::ok;
    message.icon = result.terminal == OperationTerminal::conflict
        ? gui_forms::HostMessageIcon::warning
        : gui_forms::HostMessageIcon::error;
    message.default_choice = gui_forms::HostDialogChoice::ok;

    gui_forms::HostDialogRequest request{};
    request.request_id = next_host_request_id_++;
    request.owner_id = "file-manager.operations";
    request.payload = std::move(message);
    (void)(*(*window_).host_services()).show_dialog(request);
}

void Application::activate(const std::string_view stable_id) {
    const PathMap::const_iterator tree_location = tree_locations_.find(std::string(stable_id));
    if (tree_location != tree_locations_.end()) {
        request_navigation((*tree_location).second, true);
        return;
    }
    const EntryMap::const_iterator found = entries_.find(std::string(stable_id));
    if (found == entries_.end()) return;
    if ((*found).second.kind == EntryKind::symlink) {
        set_status("Open unavailable",
                   "symbolic-link activation is not admitted");
        return;
    }
    if ((*found).second.directory) {
        request_navigation((*found).second.path, true);
    } else {
        run_platform_command(PlatformCommandKind::open_default, (*found).second);
    }
}

void Application::update_browsing_status() {
    if (settings_open_ || search_showing_ || criteria_showing_) return;
    const std::span<const std::string> selected = (*objects_).selected_ids();
    std::string text{};
    if (selected.empty()) {
        text = entries_.empty() ? "This folder is empty" :
            std::to_string(entries_.size()) + (entries_.size() == 1U ? " object" : " objects");
    } else {
        std::uint64_t bytes{};
        std::size_t folders{};
        std::size_t files{};
        bool unavailable{};
        for (const std::string& id : selected) {
            const std::unordered_map<std::string, DirectoryEntry>::const_iterator found = entries_.find(id);
            if (found == entries_.end()) { unavailable = true; continue; }
            const DirectoryEntry& entry = (*found).second;
            if (entry.directory) { ++folders; continue; }
            if (entry.identity.type != std::filesystem::file_type::regular) continue;
            ++files;
            if (!entry.identity.available() || entry.identity.size > std::numeric_limits<std::uint64_t>::max() - bytes) {
                unavailable = true;
            } else {
                bytes += entry.identity.size;
            }
        }
        text = std::to_string(selected.size()) + " selected";
        if (files > 0) text += " · " + format_bytes(bytes) + " in files";
        if (folders > 0) text += " · " + std::to_string(folders) + (folders == 1U ? " folder" : " folders");
        if (unavailable) text += " · some sizes unavailable";
    }
    const std::string summary = path_utf8(location_) + " · direct filesystem" +
        (mutation_scope_active() ? " · protected operations admitted" : " · read-only observation");
    set_status(std::move(text), summary);
    (*form_.file_manager_app_shell_status_summary).set_accessible_description(
        "Current folder: " + path_utf8(location_));
}

void Application::set_status(std::string text, std::string summary) {
    (*form_.file_manager_app_shell_status_ready).set_text(std::move(text));
    (*form_.file_manager_app_shell_status_summary).set_text(std::move(summary));
}

} // namespace file_manager
