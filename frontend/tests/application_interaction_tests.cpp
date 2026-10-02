#include "file_manager/platform_paths.hpp"
#include "fixture_links.hpp"
#include "application.hpp"

#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <unistd.h>

namespace file_manager {

class ApplicationInteractionProbe final {
  public:
    static void post_ui(Application& application, std::function<void()> work) {
        application.post_ui(std::move(work));
    }
    static std::size_t pending_ui_count(Application& application) {
        const std::lock_guard<std::mutex> lock(application.ui_mutex_);
        const std::size_t count = application.ui_queue_.size();
        return count;
    }
    static std::uint64_t
    show_search_results(Application& application, std::string query,
                        std::vector<fileman::orchestrator::SearchResultInfo> results) {
        const std::uint64_t previous_generation = application.search_generation_.fetch_add(1U);
        const std::uint64_t generation = previous_generation + 1U;
        application.filter_ = query;
        (*application.search_box_).set_text(query);
        fileman::orchestrator::SearchPageInfo page{};
        page.terminal = "complete";
        page.source = "live";
        page.complete = true;
        page.results = std::move(results);
        application.apply_engine_search(std::move(page), std::move(query), generation, false);
        return generation;
    }

    static std::uint64_t search_generation(const Application& application) {
        const std::uint64_t generation = application.search_generation_.load();
        return generation;
    }
    static bool search_showing(const Application& application) {
        return application.search_showing_;
    }
    static bool search_loading(const Application& application) {
        return application.search_loading_;
    }
    static void enter_criteria(Application& application) {
        application.search_generation_.fetch_add(1U);
        application.prepare_criteria_surface();
    }
    static std::optional<std::vector<fileman::orchestrator::SearchExactFilter>>
    criteria_filters(Application& application) {
        std::optional<std::vector<fileman::orchestrator::SearchExactFilter>> filters =
            application.criteria_filters();
        return filters;
    }
    static void add_criteria_module(Application& application) {
        application.add_criteria_module();
    }
    static void remove_criteria_module(Application& application, std::string_view module_id) {
        application.remove_criteria_module(module_id);
    }
    static std::uint64_t
    show_criteria_results(Application& application, std::string source,
                          std::optional<std::uint64_t> catalogue_generation,
                          std::vector<fileman::orchestrator::SearchResultInfo> results) {
        const std::uint64_t previous_generation = application.search_generation_.fetch_add(1U);
        const std::uint64_t generation = previous_generation + 1U;
        application.criteria_showing_ = true;
        application.criteria_loading_ = true;
        (*application.criteria_console_).set_visible(true);
        fileman::orchestrator::SearchPageInfo page{};
        page.terminal = "complete";
        page.source = std::move(source);
        page.complete = true;
        page.generation = catalogue_generation;
        page.results = std::move(results);
        application.apply_engine_criteria(std::move(page), {}, generation, false);
        return generation;
    }
    static bool criteria_showing(const Application& application) {
        return application.criteria_showing_;
    }
    static bool criteria_loading(const Application& application) {
        return application.criteria_loading_;
    }
    static void set_criteria_logically_showing(Application& application, bool showing) {
        application.criteria_showing_ = showing;
    }
    static const std::string& filter(const Application& application) {
        return application.filter_;
    }
    static bool settings_open(const Application& application) {
        return application.settings_open_;
    }
    static void install_settings_state(Application& application,
                                       fileman::orchestrator::SettingsSchemaInfo schema,
                                       fileman::orchestrator::SettingsSnapshotInfo snapshot) {
        application.settings_loading_ = false;
        application.apply_settings_state(std::move(schema), std::move(snapshot));
    }
    static void show_settings(Application& application) {
        application.show_settings();
    }
    static void reset_settings_page(Application& application) {
        application.reset_settings_page();
    }
    static std::shared_ptr<gui_forms::Command> command(const Application& application,
                                                       const std::string_view stable_id) {
        for (const std::shared_ptr<gui_forms::Command>& candidate : application.commands_) {
            if (candidate && (*candidate).stable_id() == stable_id) {
                return candidate;
            }
        }
        return nullptr;
    }
    static DirectoryEntry terminal_target(const Application& application) {
        DirectoryEntry target = application.terminal_target();
        return target;
    }
    static const std::filesystem::path& location(const Application& application) {
        return application.location_;
    }
    static std::uint64_t applied_generation(const Application& application) {
        return application.applied_generation_;
    }
};

} // namespace file_manager

namespace {

class ApplicationStopGuard final {
  public:
    explicit ApplicationStopGuard(file_manager::Application& application)
        : application_(application) {}
    ~ApplicationStopGuard() {
        application_.stop();
    }
    ApplicationStopGuard(const ApplicationStopGuard&) = delete;
    ApplicationStopGuard& operator=(const ApplicationStopGuard&) = delete;

  private:
    file_manager::Application& application_;
};

using namespace std::chrono_literals;

void require(const bool condition, const std::string& message) {
    if (!condition)
        throw std::runtime_error(message);
}

class TemporaryTree final {
  public:
    TemporaryTree() {
        const std::filesystem::path temporary_parent =
            std::filesystem::canonical(std::filesystem::temp_directory_path());
        const std::string fixture_name = "file-manager-application-interaction-" +
                                         std::to_string(static_cast<long long>(::getpid()));
        root_ = temporary_parent / fixture_name;
        quarantine_ = temporary_parent / (fixture_name + "-trash");
        require(
            root_.is_absolute() && root_.parent_path() == temporary_parent &&
                quarantine_.is_absolute() && quarantine_.parent_path() == temporary_parent,
            "generated fixture cleanup paths must remain directly inside the temporary directory");
        try {
            std::filesystem::remove_all(root_);
            std::filesystem::remove_all(quarantine_);
            std::filesystem::create_directories(root_ / "Documents" / "Nested");
            std::filesystem::create_directories(root_ / "Pictures" / "Album");
            std::filesystem::create_directories(quarantine_);
            std::ofstream root_file(root_ / "root.txt");
            root_file << "root\n";
            std::ofstream inside_file(root_ / "Documents" / "inside.txt");
            inside_file << "inside\n";
            root_file.close();
            inside_file.close();
            require(root_file.good() && inside_file.good(),
                    "fixture files must be written completely");
        } catch (...) {
            cleanup();
            throw;
        }
    }
    ~TemporaryTree() {
        cleanup();
    }
    TemporaryTree(const TemporaryTree&) = delete;
    TemporaryTree& operator=(const TemporaryTree&) = delete;

    [[nodiscard]] const std::filesystem::path& root() const noexcept {
        return root_;
    }
    [[nodiscard]] const std::filesystem::path& quarantine() const noexcept {
        return quarantine_;
    }

  private:
    void cleanup() noexcept {
        std::error_code root_error{};
        std::error_code quarantine_error{};
        std::filesystem::remove_all(root_, root_error);
        std::filesystem::remove_all(quarantine_, quarantine_error);
    }
    std::filesystem::path root_{};
    std::filesystem::path quarantine_{};
};

template <typename Predicate>
void require_eventually(file_manager::Application& application, Predicate predicate,
                        const char* message) {
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + 3s;
    while (!predicate()) {
        application.drain_ui();
        if (std::chrono::steady_clock::now() >= deadline) {
            throw std::runtime_error(message);
        }
        std::this_thread::sleep_for(1ms);
    }
    application.drain_ui();
}

bool has_object_named(const gui_forms::ObjectView& objects, const std::string_view name) {
    for (const gui_forms::ObjectViewItem& item : objects.items()) {
        if (item.name == name)
            return true;
    }
    return false;
}

std::string object_id(const gui_forms::ObjectView& objects, const std::string_view name) {
    for (const gui_forms::ObjectViewItem& item : objects.items()) {
        if (item.name == name)
            return item.stable_id;
    }
    return {};
}

std::string tree_id(const gui_forms::TreeView& tree, const std::string_view text) {
    for (const gui_forms::TreeViewItem& item : tree.items()) {
        if (item.text == text)
            return item.stable_id;
    }
    return {};
}

std::optional<gui_forms::TreeViewItem> tree_item(const gui_forms::TreeView& tree,
                                                 const std::string_view text) {
    for (const gui_forms::TreeViewItem& item : tree.items()) {
        if (item.text == text)
            return item;
    }
    return std::nullopt;
}

std::string breadcrumb_id(const gui_forms::BreadcrumbTrail& trail, const std::string_view text) {
    for (const gui_forms::BreadcrumbSegment& segment : trail.segments()) {
        if (segment.text == text)
            return segment.stable_id;
    }
    return {};
}

const gui_forms::SemanticNode* find_semantic(const std::vector<gui_forms::SemanticNode>& nodes,
                                             const std::string_view stable_id) {
    for (const gui_forms::SemanticNode& node : nodes) {
        if (node.stable_id == stable_id)
            return &node;
        if (const gui_forms::SemanticNode* found = find_semantic(node.children, stable_id)) {
            return found;
        }
    }
    return nullptr;
}

// Category lookup is deliberately shallow, matching the original menu-row search.
const gui_forms::SemanticNode*
find_menu_category(const std::vector<gui_forms::SemanticNode>& categories,
                   const std::string_view stable_id) {
    for (const gui_forms::SemanticNode& category : categories) {
        if (category.stable_id == stable_id) {
            return &category;
        }
    }
    return nullptr;
}

bool has_action(const gui_forms::SemanticNode& node, const gui_forms::SemanticAction action) {
    const bool ready =
        std::find(node.actions.begin(), node.actions.end(), action) != node.actions.end();
    return ready;
}

void require_unique_semantic_ids(const std::vector<gui_forms::SemanticNode>& nodes,
                                 std::unordered_set<std::string>& identities) {
    for (const gui_forms::SemanticNode& node : nodes) {
        bool invalid_identity = node.stable_id.empty();
        if (!invalid_identity) {
            const std::pair<std::unordered_set<std::string>::iterator, bool> insertion =
                identities.insert(node.stable_id);
            invalid_identity = !insertion.second;
        }
        if (invalid_identity) {
            throw std::runtime_error(
                "semantic snapshot contains an empty or duplicate global stable ID: " +
                (node.stable_id.empty() ? std::string("<empty>") : node.stable_id));
        }
        require_unique_semantic_ids(node.children, identities);
    }
}

void require_unique_semantic_ids(gui_forms::Window& window) {
    std::unordered_set<std::string> identities{};
    const gui_forms::SemanticSnapshot snapshot = window.semantic_snapshot();
    require_unique_semantic_ids(snapshot.roots, identities);
}

void click(gui_forms::Window& window, const std::string_view stable_id) {
    const gui_forms::Control::Ptr control = window.find(stable_id);
    require(control != nullptr, "application click target must exist");
    const gui_forms::Rect bounds = (*control).absolute_bounds();
    const gui_forms::Point point{bounds.x + bounds.width * 0.5, bounds.y + bounds.height * 0.5};
    {
        const std::string diagnostic = "application click must route through retained hit testing";
        const bool pointer_handled = window.dispatch_pointer(
            {gui_forms::PointerAction::down, gui_forms::PointerButton::primary, point});
        require(pointer_handled, diagnostic);
        const bool pointer_handled_2 = window.dispatch_pointer(
            {gui_forms::PointerAction::up, gui_forms::PointerButton::primary, point});
        require(pointer_handled_2, diagnostic);
    }
}

class ImageRecordingPainter final : public gui_forms::Painter {
  public:
    void save() override {}
    void restore() override {}
    void translate(gui_forms::Point) override {}
    void clip_rect(gui_forms::Rect) override {}
    void fill_rect(gui_forms::Rect, gui_forms::Color) override {}
    void stroke_rect(gui_forms::Rect, gui_forms::Color, double) override {}
    void draw_line(gui_forms::Point, gui_forms::Point, gui_forms::Color, double) override {
        ++lines;
    }
    void draw_text_utf8(gui_forms::Point, std::string_view text, gui_forms::FontSpec,
                        gui_forms::Color color) override {
        if (!text.empty()) {
            ++text_runs;
            text_color = color;
            if (text.find("A&B && C&D") != std::string_view::npos) literal_ampersands = true;
        }
    }
    void draw_image(gui_forms::ImageId image, gui_forms::Rect, double) override {
        ++images;
        if (image == tracked_image) ++tracked_image_draws;
    }

    std::size_t images{};
    std::size_t lines{};
    std::size_t text_runs{};
    bool literal_ampersands{};
    gui_forms::ImageId tracked_image{};
    std::size_t tracked_image_draws{};
    gui_forms::Color text_color{};
};

void host_noop() {}

// The poll borrows controls owned by the window for this synchronous wait only.
struct TextPreviewReady final {
    const gui_forms::Label& text;
    bool operator()() const {
        const bool ready = text.visible() && text.text().starts_with("root");
        return ready;
    }
};

struct ImagePreviewReady final {
    const gui_forms::PictureBox& picture;
    bool operator()() const {
        const bool ready = picture.visible() && picture.has_valid_image();
        return ready;
    }
};

struct UnavailablePreviewReady final {
    const gui_forms::Label& text;
    bool operator()() const {
        const bool ready = text.visible() && text.text().starts_with("Preview is not available");
        return ready;
    }
};

// Application stores the bound observer. Stop clears host callbacks and joins the worker before
// state dies, including assertion unwinding. Wake is deliberately inert in this headless test.
class HostCloseObservation final {
  public:
    explicit HostCloseObservation(file_manager::Application& application)
        : application_(application) {}
    ~HostCloseObservation() {
        application_.stop();
    }
    HostCloseObservation(const HostCloseObservation&) = delete;
    HostCloseObservation& operator=(const HostCloseObservation&) = delete;
    void request_close() {
        ++close_requests;
    }
    std::size_t close_requests{};

  private:
    file_manager::Application& application_;
};

// Tokens are declared after this listener and disconnect before its storage dies.
// Emission is synchronous; recording neither emits another event nor disposes a control.
// Allocation failure propagates to the test boundary and unwinds those tokens.
struct CommandTraceListener final {
    std::vector<gui_forms::CommandInvocation> invocations{};
    void record(const gui_forms::CommandInvocation& invocation) {
        invocations.push_back(invocation);
    }
};

// Polling borrows fixture state only during the synchronous require_eventually call.
struct ObjectNamed final {
    const gui_forms::ObjectView& objects;
    std::string_view name{};
    bool operator()() const {
        const bool present = has_object_named(objects, name);
        return present;
    }
};

std::string rect_text(const gui_forms::Rect& value) {
    const std::string result = std::to_string(value.x) + "," + std::to_string(value.y) + "," +
                               std::to_string(value.width) + "," + std::to_string(value.height);
    return result;
}

std::string rect_text(const gui_forms::Control::Ptr& control) {
    if (!control) {
        return "absent";
    }
    const gui_forms::Rect bounds = (*control).committed_arranged_bounds();
    const std::string result = rect_text(bounds);
    return result;
}

bool is_folder_item(const gui_forms::ObjectViewItem& item) {
    const bool folder = item.description == "Folder";
    return folder;
}

const gui_forms::InstrumentModuleSpec*
find_modified_module(const std::vector<gui_forms::InstrumentModuleSpec>& modules) {
    for (const gui_forms::InstrumentModuleSpec& module : modules) {
        if (module.stable_id == "fm.criteria.modified") {
            return &module;
        }
    }
    return nullptr;
}

bool has_object_menu_rows(gui_forms::Window& window, const std::array<std::string_view, 12>& rows) {
    for (const std::string_view row : rows) {
        const std::string identity = "fm.context.object.popup.row." + std::string(row);
        const gui_forms::Control::Ptr control = window.find(identity);
        if (!control) {
            return false;
        }
    }
    return true;
}

// The context outlives the stop guard, so assertion unwinding revokes queued
// borrowed callbacks and joins the worker before destroying their target.
struct StopDuringDrainObservation final {
    file_manager::Application& application;
    bool first_completed{};
    bool second_ran{};
    bool late_ran{};

    void stop_from_first() {
        application.stop();
        first_completed = true;
    }
    void observe_second() {
        second_ran = true;
    }
    void observe_late() {
        late_ran = true;
    }
};

void test_stop_during_ui_drain_revokes_remaining_callbacks() {
    TemporaryTree fixture{};
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), std::nullopt, false,
                                                    std::string{});
    StopDuringDrainObservation observation{*application};
    ApplicationStopGuard stop_guard(*application);

    // No host binding: only these two callbacks enter the UI queue. Both must
    // move into drain_ui's local pending queue before the first callback stops.
    file_manager::ApplicationInteractionProbe::post_ui(
        *application, std::bind_front(&StopDuringDrainObservation::stop_from_first, &observation));
    file_manager::ApplicationInteractionProbe::post_ui(
        *application, std::bind_front(&StopDuringDrainObservation::observe_second, &observation));
    require(file_manager::ApplicationInteractionProbe::pending_ui_count(*application) == 2U,
            "stop-during-drain fixture must queue both named callbacks before dispatch");
    (*application).drain_ui();
    require(observation.first_completed,
            "first UI callback must finish stop without abandoning its own invocation");
    require(!observation.second_ran,
            "stop during UI drain must revoke the remaining locally pending callback");

    file_manager::ApplicationInteractionProbe::post_ui(
        *application, std::bind_front(&StopDuringDrainObservation::observe_late, &observation));
    require(file_manager::ApplicationInteractionProbe::pending_ui_count(*application) == 0U,
            "post_ui after stop must reject work instead of retaining a borrowed callback");
    (*application).drain_ui();
    require(!observation.second_ran && !observation.late_ran,
            "subsequent UI drain must execute neither revoked nor post-stop callbacks");
}

void test_application_controls_navigate_real_directories() {
    TemporaryTree tree_fixture{};
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), std::nullopt, false,
                                                    std::string{});
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);

    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    std::shared_ptr<gui_forms::TreeView> tree =
        std::dynamic_pointer_cast<gui_forms::TreeView>((*window).find("fm.navigation.tree"));
    std::shared_ptr<gui_forms::BreadcrumbTrail> breadcrumb =
        std::dynamic_pointer_cast<gui_forms::BreadcrumbTrail>((*window).find("fm.path.breadcrumb"));
    std::shared_ptr<gui_forms::TextBox> path_editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.path.editor"));
    require(objects && tree && breadcrumb && path_editor,
            "application must install its real navigation controls");
    struct LaunchRootEnumerated final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        bool operator()() const {
            const bool ready =
                has_object_named(*objects, "Documents") && has_object_named(*objects, "root.txt");
            return ready;
        }
    };
    require_eventually(*application, LaunchRootEnumerated{objects},
                       "application must enumerate the admitted launch root");
    require((*breadcrumb).children().size() == 1U && (*breadcrumb).editor() == path_editor &&
                (*breadcrumb).segments().size() == 1U &&
                (*breadcrumb).segments().front().text == tree_fixture.root().filename().string(),
            "application path host must retain one trail/editor identity rather than rebuilding "
            "buttons");

    const gui_forms::Control::Ptr empty_preview = (*window).find(
        "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface");
    const gui_forms::Control::Ptr empty_properties = (*window).find("fm.selection.properties");
    require(empty_preview && empty_properties && !(*empty_preview).visible() &&
                !(*empty_properties).visible(),
            "no selection must retain a compact prompt without an empty preview or editable facts");
    const std::shared_ptr<gui_forms::MenuStrip> readable_menu =
        std::dynamic_pointer_cast<gui_forms::MenuStrip>((*window).find("fm.application.menu"));
    const std::shared_ptr<gui_forms::PropertyList> readable_properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(empty_properties);
    require(readable_menu && (*readable_menu).font().size == 13.0 && readable_properties &&
                (*readable_properties).font().size == 13.0 &&
                (*readable_properties).row_height() >= 30.0 && (*breadcrumb).font().size == 13.0 &&
                (*breadcrumb).appearance() == gui_forms::BreadcrumbAppearance::raised &&
                (*tree).font().size == 13.0 && (*tree).item_height() >= 26.0 &&
                (*objects).font().size == 13.0 && (*objects).details_row_height() >= 28.0,
            "folder content must use readable fonts and row targets");

    const std::optional<gui_forms::TreeViewItem> admitted_root =
        tree_item(*tree, tree_fixture.root().filename().string());
    const std::optional<gui_forms::TreeViewItem> initial_documents = tree_item(*tree, "Documents");
    const std::optional<gui_forms::TreeViewItem> initial_pictures = tree_item(*tree, "Pictures");
    require(admitted_root.has_value(), "tree must retain the explicit admitted root");
    require((*admitted_root).depth == 0U, "admitted tree root must remain at depth zero");
    require((*admitted_root).expanded, "active admitted tree root must be expanded");
    require(initial_documents.has_value() && initial_pictures.has_value(),
            "expanded admitted tree root must retain first-level folders");
    require((*initial_documents).depth == 1U && (*initial_pictures).depth == 1U,
            "first-level tree folders must remain at depth one");
    require(!tree_item(*tree, "Home").has_value() && !tree_item(*tree, "Volumes").has_value(),
            "one root mode must not concatenate unrelated peer trees");
    const std::shared_ptr<gui_forms::Button> root_mode =
        std::dynamic_pointer_cast<gui_forms::Button>((*window).find("fm.navigation.root-mode"));
    (*window).perform_layout();
    require(root_mode && (*root_mode).text() == tree_fixture.root().filename().string(),
            "tree header must expose the active honest root mode");
    const gui_forms::Control::Ptr tree_header =
        (*window).find("file-manager-app.shell.workspace.sidebar.header");
    const gui_forms::Control::Ptr tree_root_mode_host =
        (*window).find("file-manager-app.shell.workspace.sidebar.header.root-mode-host");
    const gui_forms::Control::Ptr tree_caption =
        (*window).find("file-manager-app.shell.workspace.sidebar.header.label");
    const bool one_tree_header_band =
        tree_header && tree_root_mode_host && tree_caption &&
        (*root_mode).parent() == tree_root_mode_host &&
        (*tree_root_mode_host).parent() == tree_header && (*tree_caption).parent() == tree_header &&
        (*tree_header).committed_arranged_bounds().height == 27.0 &&
        (*root_mode).committed_arranged_bounds().width == 112.0 &&
        (*root_mode).committed_arranged_bounds().height == 27.0 &&
        (*root_mode).committed_arranged_bounds().y == (*tree_caption).committed_arranged_bounds().y;
    if (!one_tree_header_band) {
        throw std::runtime_error(
            "folder caption and root-mode disclosure must share one compact header band: header=" +
            rect_text(tree_header) + " caption=" + rect_text(tree_caption) +
            " mode=" + rect_text(root_mode));
    }
#if !defined(__APPLE__)
    const std::string volume_menu_id = "fm.navigation.root-mode-menu.popup.row.admitted.0";
    const std::string launch_menu_id = "fm.navigation.root-mode-menu.popup.row.admitted." +
                                       std::to_string(file_manager::local_volume_roots().size());
#else
    const std::string volume_menu_id = "fm.navigation.root-mode-menu.popup.row.volumes";
    const std::string launch_menu_id = "fm.navigation.root-mode-menu.popup.row.admitted.0";
#endif
    {
        const std::string diagnostic =
            "tree root mode menu must keep Home and actual filesystem roots immediately available";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.navigation.root-mode", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.navigation.root-mode-menu.popup.row.home") != nullptr &&
                    (*window).find(volume_menu_id) != nullptr,
                diagnostic);
    }
    {
        const std::string diagnostic =
            "tree root mode must switch through its retained menu command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.navigation.root-mode-menu.popup.row.home", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct HomeTreeReady final {
        const std::shared_ptr<gui_forms::TreeView>& tree;
        const std::shared_ptr<gui_forms::Button>& root_mode;
        bool operator()() const {

            const std::optional<gui_forms::TreeViewItem> home = tree_item(*tree, "Home");
            const bool ready = home && (*home).depth == 0U && (*root_mode).text() == "Home";
            return ready;
        }
    };
    require_eventually(*application, HomeTreeReady{tree, root_mode},
                       "Home mode switch must expose one honest Home-rooted tree");
    {
        const std::string diagnostic =
            "explicit protected root must remain an immediate honest mode";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.navigation.root-mode", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 =
            (*window).perform_semantic_action(launch_menu_id, gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
    }
    struct DocumentsTreeRestored final {
        const std::shared_ptr<gui_forms::TreeView>& tree;
        bool operator()() const {
            const bool ready = tree_item(*tree, "Documents").has_value();
            return ready;
        }
    };
    require_eventually(*application, DocumentsTreeRestored{tree},
                       "returning to the admitted root mode must restore its retained children");

    const std::optional<gui_forms::TreeViewItem> restored_pictures = tree_item(*tree, "Pictures");
    {
        const std::string diagnostic = "collapsed folder row must expose retained expansion";
        require(restored_pictures.has_value(), diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*restored_pictures).stable_id, gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    struct AlbumTreeExpanded final {
        const std::shared_ptr<gui_forms::TreeView>& tree;
        bool operator()() const {

            const std::optional<gui_forms::TreeViewItem> album = tree_item(*tree, "Album");
            const bool ready = album && (*album).depth == 2U;
            return ready;
        }
    };
    require_eventually(*application, AlbumTreeExpanded{tree},
                       "expanding a tree row must enumerate and retain its real children");

    const std::string documents = object_id(*objects, "Documents");
    {
        const std::string diagnostic = "folder selection must route into the factual inspector";
        require(!documents.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(documents, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
    }
    const std::shared_ptr<gui_forms::Button> preview_house_icon =
        std::dynamic_pointer_cast<gui_forms::Button>(
            (*window).find("fm.inspector.preview.house-icon"));
    const gui_forms::Control::Ptr preview_glyph = (*window).find(
        "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface.glyph");
    struct FolderPreviewReady final {
        const std::shared_ptr<gui_forms::Button>& preview_house_icon;
        const gui_forms::Control::Ptr& preview_glyph;
        bool operator()() const {

            const bool ready =
                preview_house_icon && (*preview_house_icon).visible() &&
                (*preview_house_icon).image_key() == "folder" &&
                (*preview_house_icon).image_list() &&
                (*(*preview_house_icon).image_list()).image_size() == gui_forms::Size{72.0, 72.0} &&
                preview_glyph && !(*preview_glyph).visible();
            return ready;
        }
    };
    require_eventually(*application, FolderPreviewReady{preview_house_icon, preview_glyph},
                       "directory inspection must use a large House material object instead of a "
                       "textual DIR card");
    (*window).perform_layout();
    require((*preview_house_icon).committed_arranged_bounds().width == 72.0 &&
                (*preview_house_icon).committed_arranged_bounds().height == 72.0,
            "visible directory material art must retain its exact inspector geometry");
    ImageRecordingPainter preview_painter{};
    (*preview_house_icon).on_paint(preview_painter, {0.0, 0.0, 72.0, 72.0});
    require(preview_painter.images == 1U,
            "directory inspector fallback must emit one native House image draw");
    {
        const std::string diagnostic = "object activation must route to directory navigation";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(documents, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct DocumentsOpened final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        bool operator()() const {
            const bool ready =
                has_object_named(*objects, "inside.txt") && !has_object_named(*objects, "root.txt");
            return ready;
        }
    };
    require_eventually(*application, DocumentsOpened{objects},
                       "object activation must display the real child directory");

    const std::string root_crumb =
        breadcrumb_id(*breadcrumb, tree_fixture.root().filename().string());
    {
        const std::string diagnostic =
            "root breadcrumb segment must activate through its stable virtual identity";
        require(!root_crumb.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_crumb, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(
        *application, ObjectNamed{*objects, "root.txt"},
        "breadcrumb root activation must navigate through ordinary history authority");

    {
        const std::string diagnostic =
            "breadcrumb terminal actuator must reveal its owned exact-path editor";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require((*breadcrumb).editing() && (*path_editor).visible(), diagnostic);
    }
    const std::chrono::steady_clock::time_point initial_preview_deadline =
        std::chrono::steady_clock::now() + 3s;
    for (;;) {
        (*application).drain_ui();
        const std::shared_ptr<gui_forms::Label> preview =
            std::dynamic_pointer_cast<gui_forms::Label>(
                (*window).find("fm.path.resolution-preview"));
        const gui_forms::Control::Ptr list = (*window).find("fm.path.suggestions.list");
        if (preview && (*preview).text().starts_with("Resolved: ") && list)
            break;
        if (std::chrono::steady_clock::now() >= initial_preview_deadline) {
            const gui_forms::Control::Ptr layer = (*window).find("fm.path.suggestions.layer");
            throw std::runtime_error("direct path mode must publish a factual canonical preview "
                                     "and bounded local suggestions: editing=" +
                                     std::to_string((*breadcrumb).editing()) +
                                     " editor=" + std::string((*path_editor).text()) + " preview=" +
                                     (preview ? (*preview).text() : std::string("<absent>")) +
                                     " list=" + std::to_string(static_cast<bool>(list)) +
                                     " layer=" + std::to_string(static_cast<bool>(layer)));
        }
        std::this_thread::sleep_for(1ms);
    }
    const gui_forms::Control::Ptr initial_popup_layer = (*window).find("fm.path.suggestions.layer");
    require(initial_popup_layer && (*window).focused_control() == path_editor,
            "suggestion popup must preserve inline editor focus");
    (*path_editor).select_all();
    {
        const std::string diagnostic =
            "typed prefix must update the inline editor before suggestion refresh";
        const bool text_handled = (*window).dispatch_text({"Doc"});
        require(text_handled, diagnostic);
    }
    struct DocumentsSuggestionReady final {
        const std::unique_ptr<gui_forms::Window>& window;
        bool operator()() const {

            const std::shared_ptr<gui_forms::ListBox> list =
                std::dynamic_pointer_cast<gui_forms::ListBox>(
                    (*window).find("fm.path.suggestions.list"));
            const bool ready =
                list && !(*list).items().empty() && (*list).items().front() == "Documents";
            return ready;
        }
    };
    require_eventually(*application, DocumentsSuggestionReady{window},
                       "generation-tagged suggestions must converge on the latest typed prefix");
    {
        const std::string diagnostic =
            "Tab must accept the active canonical directory suggestion without navigating";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::tab});
        require(key_handled, diagnostic);
        require((*path_editor).text() == (tree_fixture.root() / "Documents").string() &&
                    (*breadcrumb).editing(),
                diagnostic);
    }
    struct DocumentsResolutionReady final {
        const std::unique_ptr<gui_forms::Window>& window;
        const TemporaryTree& tree_fixture;
        bool operator()() const {

            const std::shared_ptr<gui_forms::Label> preview =
                std::dynamic_pointer_cast<gui_forms::Label>(
                    (*window).find("fm.path.resolution-preview"));
            const bool ready =
                preview &&
                (*preview).text() == "Resolved: " + (tree_fixture.root() / "Documents").string();
            return ready;
        }
    };
    require_eventually(*application, DocumentsResolutionReady{window, tree_fixture},
                       "Tab-filled value must refresh canonical resolution preview");
    {
        const std::string diagnostic =
            "Escape with suggestion popup must roll back the whole path edit and close the popup";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        require(!(*breadcrumb).editing() && (*path_editor).text() == tree_fixture.root().string() &&
                    (*window).find("fm.path.suggestions.layer") == nullptr,
                diagnostic);
    }

    {
        const std::string diagnostic = "path editor must reopen after popup Escape rollback";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    (*path_editor).select_all();
    {
        const std::string diagnostic = "pointer completion requires a fresh matching prefix";
        const bool text_handled = (*window).dispatch_text({"Doc"});
        require(text_handled, diagnostic);
    }
    struct PointerSuggestionReady final {
        const std::unique_ptr<gui_forms::Window>& window;
        bool operator()() const {

            const std::shared_ptr<gui_forms::ListBox> list =
                std::dynamic_pointer_cast<gui_forms::ListBox>(
                    (*window).find("fm.path.suggestions.list"));
            const bool ready =
                list && !(*list).items().empty() && (*list).items().front() == "Documents";
            return ready;
        }
    };
    require_eventually(*application, PointerSuggestionReady{window},
                       "pointer completion requires one visible Documents suggestion");
    const std::shared_ptr<gui_forms::ListBox> suggestion_list =
        std::dynamic_pointer_cast<gui_forms::ListBox>((*window).find("fm.path.suggestions.list"));
    const std::vector<gui_forms::SemanticNode> suggestion_nodes =
        (*suggestion_list).semantic_virtual_children();
    require(!suggestion_nodes.empty(),
            "pointer completion fixture must publish a concrete suggestion row");
    const gui_forms::Rect suggestion_bounds = suggestion_nodes.front().bounds;
    const gui_forms::Point suggestion_center{suggestion_bounds.x + suggestion_bounds.width * 0.5,
                                             suggestion_bounds.y + suggestion_bounds.height * 0.5};
    {
        const std::string diagnostic =
            "one ordinary suggestion click must fill the editor once and preserve inline focus";
        const bool pointer_handled = (*window).dispatch_pointer(
            {gui_forms::PointerAction::down, gui_forms::PointerButton::primary, suggestion_center});
        require(pointer_handled, diagnostic);
        const bool pointer_handled_2 = (*window).dispatch_pointer(
            {gui_forms::PointerAction::up, gui_forms::PointerButton::primary, suggestion_center});
        require(pointer_handled_2, diagnostic);
        require((*path_editor).text() == (tree_fixture.root() / "Documents").string() &&
                    (*breadcrumb).editing() && (*window).focused_control() == path_editor,
                diagnostic);
    }
    {
        const std::string diagnostic =
            "pointer-filled edit must remain cancellable as one path transaction";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        require(!(*breadcrumb).editing(), diagnostic);
    }

    {
        const std::string diagnostic = "path editor must reopen after pointer completion rollback";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
#if defined(_WIN32)
    const DWORD drive_mask = GetLogicalDrives();
    std::string unadmitted_path{};
    for (int drive = 25; drive >= 0; --drive) {
        if ((drive_mask & (1UL << drive)) == 0) {
            unadmitted_path =
                std::string(1, static_cast<char>('A' + drive)) + ":/unadmitted-file-manager-root";
            break;
        }
    }
    require(!unadmitted_path.empty(), "fixture needs one unused drive letter");
    (*path_editor).set_text(unadmitted_path);
#else
    (*path_editor).set_text("/definitely/not/an/admitted/file-manager/root");
#endif
    struct InvalidPathResolved final {
        const std::unique_ptr<gui_forms::Window>& window;
        bool operator()() const {

            const std::shared_ptr<gui_forms::Label> preview =
                std::dynamic_pointer_cast<gui_forms::Label>(
                    (*window).find("fm.path.resolution-preview"));
#if defined(__linux__)
            // The real '/' root is admitted on Linux. This spelling is inside
            // that scope but missing; it must not be mislabeled out-of-scope.
            const bool ready = preview && (*preview).text().starts_with("Unavailable: ");
            return ready;
#else
            const bool ready = preview && (*preview).text().starts_with("Not admitted: ");
            return ready;
#endif
        }
    };
    require_eventually(
        *application, InvalidPathResolved{window},
        "invalid spelling must complete resolution asynchronously and publish its actual failure");
#if defined(__linux__)
    {
        const std::string diagnostic = "missing Linux path must finish its edit transaction";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
        require(!(*breadcrumb).editing(), diagnostic);
    }
    struct MissingPathRolledBack final {
        const std::unique_ptr<gui_forms::Window>& window;
        const std::shared_ptr<gui_forms::TextBox>& path_editor;
        const TemporaryTree& tree_fixture;
        bool operator()() const {

            const std::shared_ptr<gui_forms::Label> status =
                std::dynamic_pointer_cast<gui_forms::Label>(
                    (*window).find("file-manager-app.shell.status.ready"));
            const bool ready = (*path_editor).text() == tree_fixture.root().string() && status &&
                               (*status).text() == "Location unavailable";
            return ready;
        }
    };
    require_eventually(
        *application, MissingPathRolledBack{window, path_editor, tree_fixture},
        "missing Linux path must retain the previous location after asynchronous failure");
#else
    {
        const std::string diagnostic =
            "inadmissible direct path must roll back without changing location";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
        require(!(*breadcrumb).editing() && (*path_editor).text() == tree_fixture.root().string(),
                diagnostic);
    }
    const std::shared_ptr<gui_forms::Label> status_ready =
        std::dynamic_pointer_cast<gui_forms::Label>(
            (*window).find("file-manager-app.shell.status.ready"));
    require(status_ready && (*status_ready).text() == "Location not admitted",
            "invalid path rollback must remain explicit in the status surface");
#endif

    {
        const std::string diagnostic = "path editor must be reusable after invalid input";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    (*path_editor).set_text("uncommitted-click-away");
    const gui_forms::Control::Ptr click_away_surface =
        (*window).find("file-manager-app.shell.title");
    require(click_away_surface != nullptr && !(*click_away_surface).focusable(),
            "click-away fixture must be a visible non-focusable retained surface");
    const gui_forms::Rect click_away_bounds = (*click_away_surface).absolute_bounds();
    const bool click_away_handled =
        (*window).dispatch_pointer({gui_forms::PointerAction::down,
                                    gui_forms::PointerButton::primary,
                                    {click_away_bounds.x + click_away_bounds.width * 0.5,
                                     click_away_bounds.y + click_away_bounds.height * 0.5}});
    static_cast<void>(click_away_handled);
    if ((*breadcrumb).editing() || (*path_editor).text() != tree_fixture.root().string()) {
        throw std::runtime_error("primary click on a non-focusable outside surface must roll back "
                                 "path editing: editing=" +
                                 std::to_string((*breadcrumb).editing()) +
                                 " text=" + std::string((*path_editor).text()) +
                                 " outside-bounds=" + std::to_string(click_away_bounds.x) + "," +
                                 std::to_string(click_away_bounds.y) + "," +
                                 std::to_string(click_away_bounds.width) + "," +
                                 std::to_string(click_away_bounds.height));
    }

    {
        const std::string diagnostic = "direct path mode must reopen after click-away rollback";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    (*path_editor).set_text("Documents");
    {
        const std::string diagnostic =
            "relative direct path commit must route through the retained editor";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct RelativeDocumentsOpened final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<gui_forms::BreadcrumbTrail>& breadcrumb;
        bool operator()() const {
            const bool ready =
                has_object_named(*objects, "inside.txt") && (*breadcrumb).segments().size() == 2U;
            return ready;
        }
    };
    require_eventually(*application, RelativeDocumentsOpened{objects, breadcrumb},
                       "relative direct path must resolve from the current admitted location");

    const std::string nested_object = object_id(*objects, "Nested");
    {
        const std::string diagnostic =
            "overflow fixture requires real navigation to a deeper folder";
        require(!nested_object.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(nested_object, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct NestedBreadcrumbReady final {
        const std::shared_ptr<gui_forms::BreadcrumbTrail>& breadcrumb;
        bool operator()() const {
            const bool ready = (*breadcrumb).segments().size() == 3U &&
                               (*breadcrumb).segments().back().text == "Nested";
            return ready;
        }
    };
    require_eventually(
        *application, NestedBreadcrumbReady{breadcrumb},
        "deep navigation must rebuild the breadcrumb model with stable path identities");
    const gui_forms::Rect ordinary_breadcrumb_bounds = (*breadcrumb).committed_arranged_bounds();
    (*breadcrumb)
        .arrange({ordinary_breadcrumb_bounds.x, ordinary_breadcrumb_bounds.y, 160.0,
                  ordinary_breadcrumb_bounds.height});
    require((*breadcrumb).committed_arranged_bounds().width == 160.0 &&
                !(*breadcrumb).hidden_segment_ids().empty(),
            "constrained application breadcrumb must expose exact hidden path identities");
    const std::string hidden_id((*breadcrumb).hidden_segment_ids().front());
    {
        const std::string diagnostic =
            "breadcrumb overflow must open a real retained menu for hidden exact paths";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).overflow_stable_id(), gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.path.overflow-menu.popup.row." + hidden_id) != nullptr,
                diagnostic);
    }
    {
        const std::string diagnostic =
            "hidden breadcrumb path must remain operational through its menu row";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.path.overflow-menu.popup.row." + hidden_id, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct OverflowDocumentsOpened final {
        const std::shared_ptr<gui_forms::BreadcrumbTrail>& breadcrumb;
        bool operator()() const {
            const bool ready = (*breadcrumb).segments().size() == 2U &&
                               (*breadcrumb).segments().back().text == "Documents";
            return ready;
        }
    };
    require_eventually(*application, OverflowDocumentsOpened{breadcrumb},
                       "overflow menu activation must navigate to the exact hidden path");
    (*window).perform_layout();

    const std::string final_root_crumb =
        breadcrumb_id(*breadcrumb, tree_fixture.root().filename().string());
    {
        const std::string diagnostic =
            "breadcrumb root must remain stable after overflow projection";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(final_root_crumb, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(
        *application, ObjectNamed{*objects, "root.txt"},
        "breadcrumb must restore the root before ordinary navigation regression checks");

    const std::string documents_after_path = object_id(*objects, "Documents");
    {
        const std::string diagnostic =
            "object navigation must remain available after breadcrumb interactions";
        require(!documents_after_path.empty(), diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            documents_after_path, gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(
        *application, ObjectNamed{*objects, "inside.txt"},
        "object navigation must still reach Documents after breadcrumb interactions");
    {
        const std::string diagnostic = "Up must be an invokable retained command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.location.navigation.up", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "root.txt"},
                       "Up must return to the active admitted root");

    {
        const std::string diagnostic = "Back must be an invokable retained command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.location.navigation.back", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "inside.txt"},
                       "Back must restore the previous real directory");
    {
        const std::string diagnostic = "Forward must be an invokable retained command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.location.navigation.forward", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "root.txt"},
                       "Forward must restore the next real directory");

    const std::string documents_tree = tree_id(*tree, "Documents");
    {
        const std::string diagnostic =
            "tree selection must route through the public retained action";
        require(!documents_tree.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(documents_tree, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "inside.txt"},
                       "one tree selection must navigate without requiring a double-click");
    const std::optional<gui_forms::TreeViewItem> nested = tree_item(*tree, "Nested");
    require(nested && (*nested).depth == 2U && (*tree).selected_id() == documents_tree,
            "current ancestry must remain hierarchical and select its folder row");

    (*application).stop();
}

void test_selected_file_previews_reach_visible_layout() {
    TemporaryTree fixture{};
    std::ofstream literal_text(fixture.root() / "root.txt", std::ios::binary | std::ios::trunc);
    literal_text << "root\nA&B && C&D\n";
    literal_text.close();
    require(literal_text.good(), "literal preview fixture must be written completely");
    const std::filesystem::path source = std::filesystem::path(__FILE__).parent_path().parent_path() /
        "ui/boards/file_manager/assets/house/view@2x.png";
    const bool copied = std::filesystem::copy_file(source, fixture.root() / "preview.png");
    require(copied, "preview fixture must contain a real decodable PNG");
    const bool unsupported_copied = std::filesystem::copy_file(source, fixture.root() / "unsupported.bin");
    require(unsupported_copied, "unsupported fixture must exist before enumeration");
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), std::nullopt, false, std::string{});
    ApplicationStopGuard stop_guard(*application);
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>((*window).find("fm.objects.current-folder"));
    const std::shared_ptr<gui_forms::Label> text =
        std::dynamic_pointer_cast<gui_forms::Label>((*window).find("fm.inspector.preview.text"));
    const std::shared_ptr<gui_forms::PictureBox> picture =
        std::dynamic_pointer_cast<gui_forms::PictureBox>((*window).find("fm.inspector.preview.image"));
    require(objects && text && picture, "assembled application must own preview controls");
    require_eventually(*application, ObjectNamed{*objects, "preview.png"}, "preview fixture must enumerate");
    const std::string text_id = object_id(*objects, "root.txt");
    const bool text_selected = (*window).perform_semantic_action(text_id, gui_forms::SemanticAction::select);
    require(text_selected, "text must be selected through the public user action");
    require_eventually(*application, TextPreviewReady{*text}, "selected text must reach the preview control");
    (*window).perform_layout();
    const gui_forms::Rect text_bounds = (*text).committed_arranged_bounds();
    require((*text).effectively_visible() && text_bounds.width >= 100.0 && text_bounds.height >= 60.0,
        "selected text must have a visible readable layout, not merely loaded bytes");
    ImageRecordingPainter text_painter{};
    (*text).on_paint(text_painter, {0.0, 0.0, text_bounds.width, text_bounds.height});
    require(text_painter.text_runs > 0U && text_painter.text_color.red >= 200U &&
        text_painter.text_color.green >= 200U && text_painter.text_color.blue >= 200U,
        "preview must paint readable light text against its authored dark surface");
    require(text_painter.literal_ampersands,
        "file text must retain literal ampersands rather than interpret keyboard mnemonics");
    const std::string image_id = object_id(*objects, "preview.png");
    const bool image_selected = (*window).perform_semantic_action(image_id, gui_forms::SemanticAction::select);
    require(image_selected, "image must be selected through the public user action");
    require_eventually(*application, ImagePreviewReady{*picture}, "selected PNG must enter the application image registry");
    (*window).perform_layout();
    const gui_forms::Rect image_bounds = (*picture).image_bounds();
    const gui_forms::Control::Ptr preview_surface = (*window).find(
        "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface");
    const gui_forms::Rect picture_absolute = (*picture).absolute_bounds();
    const gui_forms::Rect surface_absolute = (*preview_surface).absolute_bounds();
    require(surface_absolute.contains(picture_absolute),
        "selected image control must remain inside its preview clip");
    require((*picture).effectively_visible() && image_bounds.width > 0.0 && image_bounds.height > 0.0,
        "selected PNG must have visible image geometry");
    ImageRecordingPainter painter{};
    const gui_forms::Rect picture_bounds = (*picture).committed_arranged_bounds();
    (*picture).on_paint(painter, {0.0, 0.0, picture_bounds.width, picture_bounds.height});
    require(painter.images == 1U, "selected PNG must issue an image draw");
    ImageRecordingPainter whole_window_painter{};
    whole_window_painter.tracked_image = (*picture).image();
    static_cast<void>((*window).paint(whole_window_painter));
    require(whole_window_painter.tracked_image_draws > 0U,
        "whole-window retained replay must include the selected image, not only direct control painting");

    // The first Mac native probe was constrained to 1024x674 by its desktop.
    // That ordinary viewport has room for content and a usable inspector.
    (*window).resize({1024.0, 674.0});
    (*window).perform_layout();
    require((*picture).effectively_visible() && (*picture).image_bounds().width >= 100.0,
        "ordinary 1024-wide browsing must keep the selected preview available by default");
    (*window).resize({800.0, 674.0});
    (*window).perform_layout();
    require((*picture).effectively_visible(),
        "narrowing should retire the folder tree before the selected-file inspector");

    constexpr std::string_view toggle_id = "file-manager-app.shell.workspace.selection.inspector.facts.preview.toggle";
    const std::shared_ptr<gui_forms::Button> toggle = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find(toggle_id));
    const std::shared_ptr<gui_forms::PropertyList> properties = std::dynamic_pointer_cast<gui_forms::PropertyList>(
        (*window).find("fm.selection.properties"));
    require(toggle && properties, "preview must expose an ordinary expansion control in the single scroll owner");
    const double expanded_height = (*properties).header_height();
    (*window).resize({1340.0, 500.0});
    (*window).perform_layout();
    const double compact_height = (*properties).header_height();
    require(!(*picture).effectively_visible() && compact_height < expanded_height && (*toggle).enabled(),
        "short-window collapse must release header space and leave expansion available");
    require((*toggle).effectively_visible(), "short-window preview expansion control must remain visible");
    const gui_forms::Rect toggle_bounds = (*toggle).committed_arranged_bounds();
    require(toggle_bounds.width > 0.0 && toggle_bounds.height > 0.0,
        "preview expansion control must have a nonempty target");
    const bool expanded = (*window).perform_semantic_action(toggle_id, gui_forms::SemanticAction::press);
    require(expanded, "short-window preview expansion must accept a user action");
    (*window).perform_layout();
    require((*picture).effectively_visible() && (*properties).header_height() == expanded_height &&
        (*objects).selected_id() == image_id,
        "manual expansion must restore the existing preview without changing selection");
    const bool collapsed = (*window).perform_semantic_action(toggle_id, gui_forms::SemanticAction::press);
    require(collapsed, "preview collapse must accept a user action");
    (*window).resize({1340.0, 850.0});
    (*window).perform_layout();
    require(!(*picture).effectively_visible() && (*properties).header_height() == compact_height,
        "an explicit collapsed preference must survive later resize");
    const std::string unsupported_id = object_id(*objects, "unsupported.bin");
    const bool unsupported_selected = (*window).perform_semantic_action(unsupported_id, gui_forms::SemanticAction::select);
    require(unsupported_selected, "unsupported file must accept ordinary selection");
    require_eventually(*application, UnavailablePreviewReady{*text}, "unsupported format must explain its limitation in the preview body");
    const bool explanation_expanded = (*window).perform_semantic_action(toggle_id, gui_forms::SemanticAction::press);
    require(explanation_expanded, "unavailable preview explanation must be expandable");
    (*window).perform_layout();
    require((*text).effectively_visible() && !(*picture).visible(),
        "an unavailable preview must display its explanation without retaining the previous file image");
}

void test_application_command_surfaces_and_house_mark() {
    TemporaryTree tree_fixture{};
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), std::nullopt, false,
                                                    std::string{});
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*window).perform_layout();

    const std::shared_ptr<gui_forms::PropertyList> properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            (*window).find("fm.selection.properties"));
    const gui_forms::Control::Ptr preview =
        (*window).find("file-manager-app.shell.workspace.selection.inspector.facts.preview");
    require(properties && preview && (*properties).header_content() == preview &&
                (*preview).parent() == properties,
            "preview and properties must share the PropertyList scroll owner");
    const gui_forms::Control::Ptr tree_seam =
        (*window).find("file-manager-app.shell.workspace.splitter");
    const gui_forms::Control::Ptr selection_seam =
        (*window).find("file-manager-app.shell.workspace.selection.splitter");
    require(tree_seam && selection_seam && (*tree_seam).committed_arranged_bounds().width == 12.0 &&
                (*selection_seam).committed_arranged_bounds().width == 12.0 &&
                (*tree_seam).visual_outsets().left == 9.0 &&
                (*tree_seam).visual_outsets().right == 9.0 &&
                (*selection_seam).visual_outsets().left == 9.0 &&
                (*selection_seam).visual_outsets().right == 9.0,
            "real tree and selection seams must keep 3/12 geometry plus complete engaged shadow "
            "outsets");

    const gui_forms::Control::Ptr mark = (*window).find("file-manager-app.shell.title.mark");
    const gui_forms::Control::Ptr mark_front =
        (*window).find("file-manager-app.shell.title.mark.front");
    require(mark && (*mark).committed_arranged_bounds().width == 30.0 &&
                (*mark).committed_arranged_bounds().height == 30.0 && mark_front &&
                (*mark_front).committed_arranged_bounds().width == 27.0 &&
                (*mark_front).committed_arranged_bounds().height == 25.0 &&
                (*mark_front).authored_surface_material().has_value() &&
                (*(*mark_front).authored_surface_material()).fills.size() >= 2U,
            "generated Web.Forms application mark must retain its fixed title slot and layered "
            "face without post-generation replacement");

    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    require(objects != nullptr, "application object field must exist");
    const std::shared_ptr<gui_forms::TreeView> tree =
        std::dynamic_pointer_cast<gui_forms::TreeView>((*window).find("fm.navigation.tree"));
    require((*objects).icon_cell_size() == gui_forms::Size{104.0, 86.0} &&
                (*objects).image_list() &&
                (*(*objects).image_list()).image_size() == gui_forms::Size{42.0, 42.0} && tree &&
                (*tree).image_list() &&
                (*(*tree).image_list()).image_size() == gui_forms::Size{17.0, 17.0},
            "File Manager must supply the exact compact object and tree art geometry");
    const gui_forms::Control::Ptr selection_group =
        (*window).find("file-manager-app.shell.commands.selection-group");
    const gui_forms::Control::Ptr arrange_group =
        (*window).find("file-manager-app.shell.commands.arrange-group");
    const gui_forms::Control::Ptr command_shelf = (*window).find("file-manager-app.shell.commands");
    const gui_forms::Control::Ptr selection_actions =
        (*window).find("file-manager-app.shell.commands.selection-group.actions");
    const gui_forms::Control::Ptr arrange_actions =
        (*window).find("file-manager-app.shell.commands.arrange-group.actions");
    const std::shared_ptr<gui_forms::Label> selection_caption =
        std::dynamic_pointer_cast<gui_forms::Label>(
            (*window).find("file-manager-app.shell.commands.selection-group.label"));
    const std::shared_ptr<gui_forms::Label> arrange_caption =
        std::dynamic_pointer_cast<gui_forms::Label>(
            (*window).find("file-manager-app.shell.commands.arrange-group.label"));
    const std::shared_ptr<gui_forms::DropDownButton> more =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.overflow"));
    const std::shared_ptr<gui_forms::DropDownButton> view_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.arrange-group.actions.details"));
    const std::shared_ptr<gui_forms::DropDownButton> sort_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.arrange-group.actions.refresh"));
    const std::shared_ptr<gui_forms::DropDownButton> move_copy_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.selection-group.actions.copy"));
    const std::shared_ptr<gui_forms::Button> delete_button =
        std::dynamic_pointer_cast<gui_forms::Button>(
            (*window).find("file-manager-app.shell.commands.selection-group.actions.delete"));
    const std::shared_ptr<gui_forms::Button> properties_button =
        std::dynamic_pointer_cast<gui_forms::Button>(
            (*window).find("file-manager-app.shell.commands.arrange-group.actions.settings"));
    require(command_shelf && selection_actions && arrange_actions && selection_group &&
                arrange_group && (*selection_group).visible() && (*arrange_group).visible() &&
                selection_caption && (*selection_caption).text() == "SELECTION" &&
                arrange_caption && (*arrange_caption).text() == "ARRANGE & INSPECT" && more &&
                (*more).visible() && (*more).layout_collapsed() && !(*more).effectively_visible() &&
                view_button && sort_button && move_copy_button && delete_button &&
                properties_button,
            "ordinary shelf must retain two labelled groups and three real drop-down commands");
    for (const std::shared_ptr<gui_forms::Button>& button :
         {std::static_pointer_cast<gui_forms::Button>(move_copy_button), delete_button,
          std::static_pointer_cast<gui_forms::Button>(view_button),
          std::static_pointer_cast<gui_forms::Button>(sort_button), properties_button}) {
        const std::optional<gui_forms::ControlStateRecipes>& recipes =
            (*button).visual_recipes_override();
        const std::shared_ptr<gui_forms::ImageList> image_list = (*button).image_list();
        require(recipes.has_value(),
                "every permanent shelf member must retain authored visual recipes");
        const gui_forms::ControlVisualRecipe& normal =
            (*recipes).resolve(gui_forms::ControlSurfaceState::normal);
        require(image_list && (*image_list).image_size() == gui_forms::Size{28.0, 28.0} &&
                    normal.material.fills.size() == 1U &&
                    normal.material.fills.front().kind == gui_forms::MaterialFillKind::solid &&
                    normal.material.fills.front().color.alpha == 0U && normal.material.border &&
                    (*normal.material.border).color.alpha == 0U &&
                    (*recipes).resolve(gui_forms::ControlSurfaceState::normal) !=
                        (*recipes).resolve(gui_forms::ControlSurfaceState::hot) &&
                    (*recipes).resolve(gui_forms::ControlSurfaceState::hot) !=
                        (*recipes).resolve(gui_forms::ControlSurfaceState::pressed) &&
                    (*recipes).resolve(gui_forms::ControlSurfaceState::normal) !=
                        (*recipes).resolve(gui_forms::ControlSurfaceState::disabled),
                "ordinary shelf commands must use 28-unit art on a transparent rest plate with "
                "distinct active states");
    }
    const gui_forms::Rect shelf_bounds = (*command_shelf).committed_arranged_bounds();
    const gui_forms::Rect selection_bounds = (*selection_group).committed_arranged_bounds();
    const gui_forms::Rect arrange_bounds = (*arrange_group).committed_arranged_bounds();
    const gui_forms::Rect selection_actions_bounds =
        (*selection_actions).committed_arranged_bounds();
    const gui_forms::Rect arrange_actions_bounds = (*arrange_actions).committed_arranged_bounds();
    const gui_forms::Rect move_copy_bounds = (*move_copy_button).committed_arranged_bounds();
    const gui_forms::Rect delete_bounds = (*delete_button).committed_arranged_bounds();
    const gui_forms::Rect view_bounds = (*view_button).committed_arranged_bounds();
    const gui_forms::Rect sort_bounds = (*sort_button).committed_arranged_bounds();
    const gui_forms::Rect selection_caption_bounds =
        (*selection_caption).committed_arranged_bounds();
    const gui_forms::Rect arrange_caption_bounds = (*arrange_caption).committed_arranged_bounds();
    const bool office_pearl_geometry =
        shelf_bounds.height == 72.0 && selection_bounds.width >= 176.0 &&
        selection_bounds.height == 69.0 && arrange_bounds.width >= 306.0 &&
        arrange_bounds.height == 69.0 && selection_actions_bounds.height == 48.0 &&
        arrange_actions_bounds.height == 48.0 && move_copy_bounds.height == 48.0 &&
        delete_bounds.height == 48.0 && view_bounds.height == 48.0 && sort_bounds.height == 48.0 &&
        selection_caption_bounds.height == 16.0 && arrange_caption_bounds.height == 16.0 &&
        selection_caption_bounds.y >=
            selection_actions_bounds.y + selection_actions_bounds.height &&
        arrange_caption_bounds.y >= arrange_actions_bounds.y + arrange_actions_bounds.height &&
        arrange_bounds.x >= selection_bounds.x + selection_bounds.width + 4.0;
    if (!office_pearl_geometry) {
        throw std::runtime_error(
            "ordinary Office Pearl geometry mismatch: shelf=" + rect_text(shelf_bounds) +
            " selection=" + rect_text(selection_bounds) + " arrange=" + rect_text(arrange_bounds) +
            " selection-actions=" + rect_text(selection_actions_bounds) +
            " arrange-actions=" + rect_text(arrange_actions_bounds) +
            " move-copy=" + rect_text(move_copy_bounds) + " delete=" + rect_text(delete_bounds) +
            " view=" + rect_text(view_bounds) + " sort=" + rect_text(sort_bounds) +
            " selection-caption=" + rect_text(selection_caption_bounds) +
            " arrange-caption=" + rect_text(arrange_caption_bounds));
    }
    std::shared_ptr<gui_forms::MenuStrip> application_menu =
        std::dynamic_pointer_cast<gui_forms::MenuStrip>((*window).find("fm.application.menu"));
    require(application_menu && (*application_menu).selected_item_id() == "fm.menu.home",
            "Home command category must remain selected above the permanent shelf");
    std::vector<gui_forms::SemanticNode> menu_semantics =
        (*application_menu).semantic_virtual_children();
    const gui_forms::SemanticNode* home_category =
        find_menu_category(menu_semantics, "fm.menu.home");
    require(home_category != nullptr &&
                gui_forms::has_semantic_state((*home_category).states,
                                              gui_forms::SemanticState::selected) &&
                !gui_forms::has_semantic_state((*home_category).states,
                                               gui_forms::SemanticState::expanded),
            "closed application menu must publish Home as selected but not expanded");
    {
        const std::string diagnostic =
            "File popup must open without changing the selected Home category";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.file", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    menu_semantics = (*application_menu).semantic_virtual_children();
    const gui_forms::SemanticNode* open_file_category =
        find_menu_category(menu_semantics, "fm.menu.file");
    const gui_forms::SemanticNode* selected_home_category =
        find_menu_category(menu_semantics, "fm.menu.home");
    {
        const std::string diagnostic =
            "transient File expansion must remain distinct from persistent Home category selection";
        require(open_file_category != nullptr &&
                    gui_forms::has_semantic_state((*open_file_category).states,
                                                  gui_forms::SemanticState::expanded) &&
                    !gui_forms::has_semantic_state((*open_file_category).states,
                                                   gui_forms::SemanticState::selected) &&
                    selected_home_category != nullptr &&
                    gui_forms::has_semantic_state((*selected_home_category).states,
                                                  gui_forms::SemanticState::selected) &&
                    !gui_forms::has_semantic_state((*selected_home_category).states,
                                                   gui_forms::SemanticState::expanded),
                diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }
    const std::string expected_initial_view =
        (*objects).view_mode() == gui_forms::ObjectViewMode::details ? "Details" : "Small icons";
    const std::shared_ptr<gui_forms::Command> icons_command =
        file_manager::ApplicationInteractionProbe::command(*application, "view.small-icons");
    const std::shared_ptr<gui_forms::Command> details_command =
        file_manager::ApplicationInteractionProbe::command(*application, "view.details");
    if ((*view_button).text() != "View" ||
        (*view_button).accessible_description().find(expected_initial_view) == std::string::npos ||
        !icons_command || !details_command ||
        (*icons_command).state().checked !=
            ((*objects).view_mode() == gui_forms::ObjectViewMode::icons) ||
        (*details_command).state().checked !=
            ((*objects).view_mode() == gui_forms::ObjectViewMode::details)) {
        throw std::runtime_error("permanent View shelf truth mismatch: text=\"" +
                                 std::string((*view_button).text()) + "\" description=\"" +
                                 std::string((*view_button).accessible_description()) +
                                 "\" object-mode=" + expected_initial_view);
    }
    {
        const std::string diagnostic =
            "read-only mutation commands must remain visible but refuse invocation";
        require((*move_copy_button).effectively_visible() &&
                    (*delete_button).effectively_visible() &&
                    !(*move_copy_button).effectively_enabled() &&
                    !(*delete_button).effectively_enabled(),
                diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*move_copy_button).stable_id().value(), gui_forms::SemanticAction::show_menu);
        require(!semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            (*delete_button).stable_id().value(), gui_forms::SemanticAction::press);
        require(!semantic_action_handled_2, diagnostic);
    }
    ImageRecordingPainter closed_view_painter{};
    (*view_button).on_paint(closed_view_painter, (*view_button).committed_arranged_bounds());
    {
        const std::string diagnostic =
            "focused View drop-down must open through the composed Alt+Down route";
        const bool focus_requested = (*window).request_focus(view_button);
        require(focus_requested, diagnostic);
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::down, gui_forms::Modifier::alt});
        require(key_handled, diagnostic);
    }
    ImageRecordingPainter open_view_painter{};
    (*view_button).on_paint(open_view_painter, (*view_button).committed_arranged_bounds());
    require(gui_forms::has_semantic_state((*view_button).semantic_descriptor().states,
                                          gui_forms::SemanticState::expanded) &&
                open_view_painter.lines == closed_view_painter.lines + 1U,
            "open View must add expanded semantics and one bounded bottom-edge cue");
    {
        const std::string diagnostic = "View shelf popup must switch to small icons";
        require((*window).find("fm.shelf.view.popup.row.small-icons") != nullptr &&
                    (*view_button).drop_down_open(),
                diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.shelf.view.popup.row.small-icons", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require((*objects).view_mode() == gui_forms::ObjectViewMode::icons &&
                    !(*view_button).drop_down_open(),
                diagnostic);
    }
    ImageRecordingPainter reclosed_view_painter{};
    (*view_button).on_paint(reclosed_view_painter, (*view_button).committed_arranged_bounds());
    require(!gui_forms::has_semantic_state((*view_button).semantic_descriptor().states,
                                           gui_forms::SemanticState::expanded) &&
                reclosed_view_painter.lines == closed_view_painter.lines,
            "closed View must remove expanded semantics and its bottom-edge cue");
    {
        const std::string diagnostic =
            "View shelf command must keep its prototype label while checked mode and accessibility "
            "move to Details";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*view_button).stable_id().value(), gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.shelf.view.popup.row.details", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*objects).view_mode() == gui_forms::ObjectViewMode::details &&
                    (*view_button).text() == "View" &&
                    (*view_button).accessible_description().find("Details") != std::string::npos,
                diagnostic);
    }

    click(*window, (*sort_button).stable_id().value());
    {
        const std::string diagnostic = "Sort shelf popup must invoke a concrete sort command";
        require((*window).find("fm.shelf.sort.popup.row.kind") != nullptr, diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.shelf.sort.popup.row.kind", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    require((*sort_button).text() == "Sort: Kind",
            "Sort command must publish its current retained state");

    {
        const std::string diagnostic = "Properties shelf command must be invokable";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.commands.arrange-group.actions.settings",
            gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    const gui_forms::Control::Ptr focused = (*window).focused_control();
    require(focused && (*focused).stable_id().value() == "fm.selection.properties",
            "Properties must reveal and focus the factual inspector");

    (*window).resize({400.0, 850.0});
    (*window).perform_layout();
    require((*selection_group).visible() && !(*selection_group).layout_collapsed() &&
                (*arrange_group).visible() && (*arrange_group).layout_collapsed() &&
                (*more).visible() && !(*more).layout_collapsed() && (*more).text() == "More" &&
                (*selection_caption).effectively_visible() &&
                !(*arrange_caption).effectively_visible(),
            "medium shelf must retain Selection and collapse lower-priority Arrange into More");
    {
        const std::string diagnostic =
            "medium shelf overflow must expose every hidden Arrange command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.commands.overflow", gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.shelf.more-menu.popup.row.view") &&
                    (*window).find("fm.shelf.more-menu.popup.row.sort") &&
                    (*window).find("fm.shelf.more-menu.popup.row.properties"),
                diagnostic);
    }
    {
        const std::string diagnostic =
            "overflow Properties must share the permanent command authority";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.shelf.more-menu.popup.row.properties", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }

    (*window).resize({220.0, 850.0});
    (*window).perform_layout();
    require((*selection_group).visible() && (*selection_group).layout_collapsed() &&
                (*arrange_group).visible() && (*arrange_group).layout_collapsed() &&
                (*more).visible() && !(*more).layout_collapsed() && (*more).text() == "More",
            "narrow shelf must collapse both command groups into the authored More menu");
    {
        const std::string diagnostic =
            "narrow Commands overflow must retain exactly all five permanent shelf members";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.commands.overflow", gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.shelf.more-menu.popup.row.move-copy") &&
                    (*window).find("fm.shelf.more-menu.popup.row.delete") &&
                    (*window).find("fm.shelf.more-menu.popup.row.view") &&
                    (*window).find("fm.shelf.more-menu.popup.row.sort") &&
                    (*window).find("fm.shelf.more-menu.popup.row.properties"),
                diagnostic);
    }
    {
        const std::string diagnostic = "narrow shelf overflow must close through ordinary Escape";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }

    (*window).resize({150.0, 150.0});
    (*window).perform_layout();
    const std::shared_ptr<gui_forms::MenuStrip> minimum_menu =
        std::dynamic_pointer_cast<gui_forms::MenuStrip>((*window).find("fm.application.menu"));
    require(minimum_menu && (*minimum_menu).effectively_visible() &&
                (*minimum_menu).items().size() == 1U &&
                (*minimum_menu).items().front().stable_id == "fm.menu.compact" &&
                !(*more).effectively_visible(),
            "minimum window must preserve the complete compact menu while collapsing secondary "
            "shelf chrome");

    (*window).resize({1340.0, 850.0});
    (*window).perform_layout();
    require((*selection_group).visible() && !(*selection_group).layout_collapsed() &&
                (*arrange_group).visible() && !(*arrange_group).layout_collapsed() &&
                (*selection_caption).effectively_visible() &&
                (*arrange_caption).effectively_visible() && (*more).visible() &&
                (*more).layout_collapsed() && !(*more).effectively_visible() &&
                (*sort_button).text() == "Sort: Kind" &&
                (*selection_group).committed_arranged_bounds().width >= 176.0 &&
                (*arrange_group).committed_arranged_bounds().width >= 306.0 &&
                (*move_copy_button).committed_arranged_bounds().height == 48.0 &&
                (*sort_button).committed_arranged_bounds().height == 48.0 &&
                (*arrange_group).committed_arranged_bounds().x >=
                    (*selection_group).committed_arranged_bounds().x +
                        (*selection_group).committed_arranged_bounds().width + 4.0,
            "widening must restore both labelled groups without losing command state");

    {
        const std::string diagnostic =
            "File menu Settings command must open the real settings surface";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.file", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.application.menu.menu.popup.row.file.settings") != nullptr,
                diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.file.settings", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
    }
    const gui_forms::Control::Ptr settings = (*window).find("file-manager-app.shell.settings");
    require(settings && (*settings).visible(),
            "Settings command must replace the file surface visibly");
    {
        const std::string diagnostic = "Back to files must restore the file surface";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.settings.actions.back", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require(!(*settings).visible(), diagnostic);
    }

    (*application).stop();
}

bool terminal_command_truthful(const gui_forms::Command& command) {
#if defined(__linux__)
    const bool truthful = !command.state().enabled && !command.state().availability_reason.empty();
#else
    const bool truthful = command.state().enabled && command.state().availability_reason.empty();
#endif
    return truthful;
}

void test_application_command_truth_across_files_search_and_settings() {
    TemporaryTree tree_fixture{};
    std::filesystem::create_directories(tree_fixture.root() / "Empty");
    const bool symlink_available = create_fixture_link(tree_fixture.root() / "root.txt",
                                                       tree_fixture.root() / "root-link.txt");
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), tree_fixture.quarantine(),
                                                    true, "fixture-root");
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    HostCloseObservation host_observation{*application};
    (*application)
        .bind_host(host_noop,
                   std::bind_front(&HostCloseObservation::request_close, &host_observation));

    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    std::shared_ptr<gui_forms::TreeView> folder_tree =
        std::dynamic_pointer_cast<gui_forms::TreeView>((*window).find("fm.navigation.tree"));
    std::shared_ptr<gui_forms::CorrespondenceView> correspondence =
        std::dynamic_pointer_cast<gui_forms::CorrespondenceView>(
            (*window).find("fm.search.correspondence"));
    std::shared_ptr<gui_forms::TextBox> search =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.search.current-folder"));
    const std::shared_ptr<gui_forms::TextBox> path_editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.path.editor"));
    std::shared_ptr<gui_forms::TextBox> rename =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.operations.rename"));
    std::shared_ptr<gui_forms::Label> status_ready = std::dynamic_pointer_cast<gui_forms::Label>(
        (*window).find("file-manager-app.shell.status.ready"));
    std::shared_ptr<gui_forms::Label> status_summary = std::dynamic_pointer_cast<gui_forms::Label>(
        (*window).find("file-manager-app.shell.status.summary"));
    std::shared_ptr<gui_forms::DropDownButton> view_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.arrange-group.actions.details"));
    std::shared_ptr<gui_forms::DropDownButton> sort_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.arrange-group.actions.refresh"));
    std::shared_ptr<gui_forms::DropDownButton> move_copy_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.selection-group.actions.copy"));
    require(objects && folder_tree && correspondence && search && rename && status_ready &&
                status_summary && view_button && sort_button && move_copy_button,
            "command truth fixture must expose files, search, status, and shelf surfaces");
    std::shared_ptr<gui_forms::Command> open_command =
        file_manager::ApplicationInteractionProbe::command(*application, "file.open");
    std::shared_ptr<gui_forms::Command> back_command =
        file_manager::ApplicationInteractionProbe::command(*application, "go.back");
    std::shared_ptr<gui_forms::Command> icons_command =
        file_manager::ApplicationInteractionProbe::command(*application, "view.small-icons");
    std::shared_ptr<gui_forms::Command> sort_name_command =
        file_manager::ApplicationInteractionProbe::command(*application, "sort.name");
    std::shared_ptr<gui_forms::Command> checksum_command =
        file_manager::ApplicationInteractionProbe::command(*application, "commands.sha256");
    std::shared_ptr<gui_forms::Command> terminal_command =
        file_manager::ApplicationInteractionProbe::command(*application, "commands.terminal");
    std::shared_ptr<gui_forms::Command> copy_path_command =
        file_manager::ApplicationInteractionProbe::command(*application, "commands.copy-path");
    require(open_command && back_command && icons_command && sort_name_command &&
                checksum_command && terminal_command && copy_path_command,
            "command truth fixture must retain canonical Open, Back, View, Sort, SHA-256, "
            "Terminal, and Copy path commands");
    CommandTraceListener trace_listener{};
    const std::vector<gui_forms::CommandInvocation>& command_trace = trace_listener.invocations;
    gui_forms::SubscriptionToken open_trace =
        (*open_command)
            .invoked()
            .subscribe(gui_forms::Delegate<const gui_forms::CommandInvocation&>::bind<
                       CommandTraceListener, &CommandTraceListener::record>(trace_listener));
    gui_forms::SubscriptionToken back_trace =
        (*back_command)
            .invoked()
            .subscribe(gui_forms::Delegate<const gui_forms::CommandInvocation&>::bind<
                       CommandTraceListener, &CommandTraceListener::record>(trace_listener));
    gui_forms::SubscriptionToken copy_path_trace =
        (*copy_path_command)
            .invoked()
            .subscribe(gui_forms::Delegate<const gui_forms::CommandInvocation&>::bind<
                       CommandTraceListener, &CommandTraceListener::record>(trace_listener));
    gui_forms::SubscriptionToken checksum_trace =
        (*checksum_command)
            .invoked()
            .subscribe(gui_forms::Delegate<const gui_forms::CommandInvocation&>::bind<
                       CommandTraceListener, &CommandTraceListener::record>(trace_listener));
    struct CommandFixtureEnumerated final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const bool& symlink_available;
        bool operator()() const {
            const bool ready = has_object_named(*objects, "root.txt") &&
                               has_object_named(*objects, "Documents") &&
                               has_object_named(*objects, "Empty") &&
                               (!symlink_available || has_object_named(*objects, "root-link.txt"));
            return ready;
        }
    };
    require_eventually(*application, CommandFixtureEnumerated{objects, symlink_available},
                       "command truth fixture must enumerate the protected root");
    require_unique_semantic_ids(*window);
    require((*icons_command).state().enabled &&
                (*icons_command).state().availability_reason.empty() &&
                (*sort_name_command).state().enabled &&
                (*sort_name_command).state().availability_reason.empty(),
            "enabled ordinary folder View and Sort commands must not retain a disabled-search "
            "explanation");

#if defined(__APPLE__)
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::meta;
#else
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::control;
#endif

    {
        const std::string diagnostic =
            "the advertised primary-modifier+A gesture must execute Select All through the visible "
            "folder surface";
        const bool focus_requested = (*window).request_focus(objects);
        require(focus_requested, diagnostic);
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::a, primary});
        require(key_handled, diagnostic);
        require((*objects).selected_ids().size() == (*objects).items().size(), diagnostic);
    }

    require((*status_ready)
                    .text()
                    .starts_with(std::to_string((*objects).items().size()) + " selected") &&
                (*status_summary).text().starts_with(file_manager::path_utf8(tree_fixture.root())),
            "ordinary selection status must count objects and retain the exact current folder");
    {
        const std::string diagnostic =
            "primary-modifier+L must focus the exact editable folder path";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::l, primary});
        require(key_handled, diagnostic);
        require((*window).focused_control() == path_editor && (*path_editor).visible() &&
                    (*path_editor).text() == file_manager::path_utf8(tree_fixture.root()),
                diagnostic);
    }
    (*path_editor).set_text("unfinished folder draft");
    {
        const std::string diagnostic =
            "repeating primary-modifier+L must select the existing draft without replacing it";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::l, primary});
        require(key_handled, diagnostic);
        require((*path_editor).selected_text() == "unfinished folder draft", diagnostic);
    }
    const bool draft_refresh_handled =
        (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::f5});
    static_cast<void>(draft_refresh_handled);
    require((*path_editor).text() == "unfinished folder draft" && (*path_editor).visible(),
            "F5 must preserve an active path draft");
    const std::uint64_t before_refresh =
        file_manager::ApplicationInteractionProbe::applied_generation(*application);
    {
        const std::string diagnostic = "F5 must execute Refresh from the ordinary folder surface";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        const bool focus_requested = (*window).request_focus(objects);
        require(focus_requested, diagnostic);
        const bool key_handled_2 =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::f5});
        require(key_handled_2, diagnostic);
    }
    struct RefreshApplied final {
        file_manager::Application& application;
        std::uint64_t before{};
        bool operator()() const {
            const bool ready =
                file_manager::ApplicationInteractionProbe::applied_generation(application) > before;
            return ready;
        }
    };
    require_eventually(*application, RefreshApplied{*application, before_refresh},
                       "F5 must publish a fresh directory enumeration");

    const std::string root_file = object_id(*objects, "root.txt");
    std::shared_ptr<gui_forms::PropertyList> selection_properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            (*window).find("fm.selection.properties"));
    {
        const std::string diagnostic =
            "the advertised F2 gesture must begin the shared protected Rename command";
        require(!root_file.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
        require((*status_ready).text() == "1 selected · " +
                                              file_manager::format_bytes(std::filesystem::file_size(
                                                  tree_fixture.root() / "root.txt")) +
                                              " in files" &&
                    (*checksum_command).state().enabled &&
                    (*checksum_command).state().availability_reason.empty() &&
                    terminal_command_truthful(*terminal_command),
                diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::f2});
        require(key_handled, diagnostic);
        require((*rename).visible() && (*window).focused_control() == rename, diagnostic);
    }
    std::shared_ptr<gui_forms::TextBox> expected_sha =
        std::dynamic_pointer_cast<gui_forms::TextBox>(
            selection_properties ? (*selection_properties).editor("fm.property.expected-sha256")
                                 : nullptr);
    require(selection_properties && expected_sha &&
                (*expected_sha).parent() == selection_properties && (*expected_sha).visible() &&
                (*expected_sha).enabled() && (*expected_sha).maximum_length() == 64U,
            "one selected regular file must expose one bounded expected SHA-256 editor owned by "
            "the live PropertyList");
    (*expected_sha).set_text(std::string(64U, 'a'));
    require((*expected_sha).text() == std::string(64U, 'a'),
            "expected SHA-256 editor must retain an exact comparison value for the selected "
            "regular file");
    {
        const std::string diagnostic =
            "TextBox must retain first refusal for its own Select All editing chord";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::a, primary});
        require(key_handled, diagnostic);
        require((*rename).selected_text() == "root.txt" && (*objects).selected_ids().size() == 1U,
                diagnostic);
    }
    {
        const std::string diagnostic =
            "Escape must cancel the accelerator-opened Rename transaction";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        require(!(*rename).visible(), diagnostic);
    }

    {
        const std::string diagnostic =
            "selected regular file must open its retained object context menu";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
    }
    const gui_forms::Control::Ptr object_menu_panel =
        (*window).find("fm.context.object.popup.panel.0");
    constexpr std::array<std::string_view, 12> object_menu_rows{"open",
                                                                "separator.transfer",
                                                                "copy",
                                                                "move",
                                                                "rename",
                                                                "delete",
                                                                "separator.commands",
                                                                "sha256",
                                                                "terminal",
                                                                "copy-path",
                                                                "separator.properties",
                                                                "properties"};
    require(object_menu_panel &&
                (*object_menu_panel).children().size() == object_menu_rows.size() &&
                has_object_menu_rows(*window, object_menu_rows),
            "object context menu must expose exactly its admitted command and separator rows");
    {
        const std::string diagnostic =
            "object context Copy path row must invoke its shared command";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.context.object.popup.row.copy-path", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require(!command_trace.empty() && command_trace.back().command_id == "commands.copy-path" &&
                    command_trace.back().source_id == "fm.context.object.popup.row.copy-path" &&
                    (*status_ready).text() == "Clipboard unavailable",
                diagnostic);
    }
    {
        const std::string diagnostic =
            "object context Properties row must reveal and focus the factual inspector";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.properties", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*window).focused_control() == selection_properties &&
                    (*status_ready).text() == "Selection and Properties active",
                diagnostic);
    }
    {
        const std::string diagnostic =
            "object context Copy row must stage the selected exact object";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.copy", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*status_ready).text() == "Copy captured · root.txt", diagnostic);
    }
    {
        const std::string diagnostic =
            "object context Move row must stage the selected exact object";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.move", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*status_ready).text() == "Move captured · root.txt", diagnostic);
    }
    {
        const std::string diagnostic =
            "object context Rename row must open the bounded editor and cancel cleanly";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.rename", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*rename).visible() && (*window).focused_control() == rename &&
                    (*rename).text() == "root.txt",
                diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        require(!(*rename).visible(), diagnostic);
    }
    {
        const std::string diagnostic =
            "object context Delete row must enter the first recoverable confirmation state";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.delete", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*status_ready).text() == "Delete armed · press Delete again for root.txt",
                diagnostic);
    }
    {
        const std::string diagnostic =
            "transfer capture must clear the Delete arm without mutating the fixture";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.copy", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require((*status_ready).text() == "Copy captured · root.txt" &&
                    std::filesystem::exists(tree_fixture.root() / "root.txt"),
                diagnostic);
    }
    {
        const std::string diagnostic =
            "object context SHA-256 row must invoke its canonical read-only command";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.context.object.popup.row.sha256", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require(!command_trace.empty() && command_trace.back().command_id == "commands.sha256" &&
                    command_trace.back().source_id == "fm.context.object.popup.row.sha256",
                diagnostic);
    }
    struct ChecksumComparisonReady final {
        const std::shared_ptr<gui_forms::Label>& status_ready;
        const std::shared_ptr<gui_forms::Label>& status_summary;
        bool operator()() const {

            const bool ready = (*status_ready).text().starts_with("SHA-256 complete") &&
                               (*status_summary).text() == "expected digest did not match";
            return ready;
        }
    };
    require_eventually(*application, ChecksumComparisonReady{status_ready, status_summary},
                       "object context SHA-256 must complete against the selected stable file and "
                       "compare the expected digest");

    const file_manager::DirectoryEntry file_terminal_target =
        file_manager::ApplicationInteractionProbe::terminal_target(*application);
    file_manager::PlatformCommandPlan file_terminal_plan{};
    const file_manager::PlatformCommandResult file_terminal_result =
        file_manager::make_platform_command_plan(
            file_manager::PlatformCommandKind::open_terminal_here, tree_fixture.root(),
            file_terminal_target.path, file_terminal_target.identity, file_terminal_plan);
#if defined(__linux__)
    require(
        file_terminal_target.path == tree_fixture.root() && file_terminal_target.directory &&
            file_terminal_result.code == "terminal-unavailable" &&
            file_terminal_plan.executable.empty() && file_terminal_plan.arguments.empty(),
        "Linux must retain the selected directory but refuse an unconfigured terminal launcher");
#else
#if defined(_WIN32)
    const std::vector<std::string> expected_terminal_arguments{"/D"};
    const bool terminal_executable_matches = file_terminal_plan.executable.filename() == L"cmd.exe";
#else
    const std::vector<std::string> expected_terminal_arguments{
        "/usr/bin/open", "-a", "Terminal", file_terminal_plan.selected_path.string()};
    const bool terminal_executable_matches = file_terminal_plan.executable == "/usr/bin/open";
#endif
    std::error_code terminal_equivalence_error{};
    if (file_terminal_target.path != tree_fixture.root() || !file_terminal_target.directory ||
        file_terminal_result.code != "ok" || !terminal_executable_matches ||
        file_terminal_plan.arguments != expected_terminal_arguments ||
        !std::filesystem::equivalent(file_terminal_plan.selected_path, tree_fixture.root(),
                                     terminal_equivalence_error) ||
        terminal_equivalence_error) {
        throw std::runtime_error(
            "regular-file Terminal fallback mismatch: target=" +
            file_terminal_target.path.string() + " root=" + tree_fixture.root().string() +
            " directory=" + std::to_string(file_terminal_target.directory) + " result=" +
            file_terminal_result.code + " executable=" + file_terminal_plan.executable.string() +
            " argc=" + std::to_string(file_terminal_plan.arguments.size()) + " last=" +
            (file_terminal_plan.arguments.empty() ? std::string("<empty>")
                                                  : file_terminal_plan.arguments.back()));
    }
#endif

#if defined(__APPLE__)
    constexpr std::uint32_t delete_key = gui_forms::PhysicalKey::backspace;
#else
    constexpr std::uint32_t delete_key = gui_forms::PhysicalKey::delete_forward;
#endif
    {
        const std::string diagnostic = "the advertised Delete gesture must enter the same two-step "
                                       "recoverable quarantine flow";
        const bool focus_requested = (*window).request_focus(objects);
        require(focus_requested, diagnostic);
        const bool key_handled = (*window).dispatch_key({gui_forms::KeyAction::down, delete_key});
        require(key_handled, diagnostic);
        require((*status_ready).text().starts_with("Delete armed"), diagnostic);
    }

    {
        const std::string diagnostic =
            "Home Move / copy submenu must invoke the protected staged Copy command";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.home", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.home.move-copy", gui_forms::SemanticAction::expand);
        require(semantic_action_handled_2, diagnostic);
        const bool semantic_action_handled_3 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.home.copy", gui_forms::SemanticAction::press);
        require(semantic_action_handled_3, diagnostic);
    }
    (*objects).clear_selection();
    require((*objects).selected_ids().empty() && !(*move_copy_button).enabled(),
            "staged Paste must not enable the Paste-free Move/Copy dropdown when no object is "
            "selected");
    (*window).resize({220.0, 850.0});
    (*window).perform_layout();
    {
        const std::string diagnostic =
            "narrow staged-transfer fixture must open the permanent Commands projection";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.commands.overflow", gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
    }
    gui_forms::SemanticSnapshot narrow_semantics = (*window).semantic_snapshot();
    {
        std::unordered_set<std::string> identities{};
        require_unique_semantic_ids(narrow_semantics.roots, identities);
    }
    const gui_forms::SemanticNode* empty_move_copy =
        find_semantic(narrow_semantics.roots, "fm.shelf.more-menu.popup.row.move-copy");
    {
        const std::string diagnostic =
            "narrow Move/Copy submenu must be disabled when neither contained transfer command is "
            "actionable";
        require(empty_move_copy && (*empty_move_copy).actions.empty() &&
                    (*empty_move_copy).description.find("No commands are currently available") !=
                        std::string::npos,
                diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.shelf.more-menu.popup.row.move-copy", gui_forms::SemanticAction::expand);
        require(!semantic_action_handled, diagnostic);
    }
    {
        const std::string diagnostic =
            "narrow staged-transfer Commands projection must close through Escape";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }
    (*window).resize({1340.0, 850.0});
    (*window).perform_layout();
    const std::string documents = object_id(*objects, "Documents");
    const std::string documents_tree = tree_id(*folder_tree, "Documents");
    require(
        !documents_tree.empty() && documents_tree != documents,
        "tree and object presentations of one folder must own distinct global semantic identities");
    {
        const std::string diagnostic =
            "expected SHA-256 carryover fixture must restore one regular-file selection";
        require(!documents.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(root_file, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
        require((*expected_sha).enabled(), diagnostic);
    }
    (*expected_sha).set_text(std::string(64U, 'a'));
    {
        const std::string diagnostic =
            "expected SHA-256 input must not carry from one filesystem object to a selected folder";
        require((*expected_sha).text() == std::string(64U, 'a'), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(documents, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
        require((*expected_sha).text().empty() && !(*expected_sha).enabled(), diagnostic);
    }
    const file_manager::DirectoryEntry directory_terminal_target =
        file_manager::ApplicationInteractionProbe::terminal_target(*application);
    require(directory_terminal_target.path == tree_fixture.root() / "Documents" &&
                directory_terminal_target.directory &&
                directory_terminal_target.kind == file_manager::EntryKind::folder,
            "Terminal Here must prefer the exact selected nonsymlink directory");
    {
        const std::string diagnostic =
            "the advertised Enter gesture must execute Open on the focused folder object";
        require((*window).focused_control() == objects, diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "inside.txt"},
                       "Enter Open must navigate to the real Documents folder");
    if (command_trace.empty() || command_trace.back().command_id != "file.open" ||
        command_trace.back().source_id != "fm.accelerator.file.open") {
        throw std::runtime_error(
            "Enter canonical Open trace mismatch: count=" + std::to_string(command_trace.size()) +
            " last-command=" +
            (command_trace.empty() ? std::string("<none>") : command_trace.back().command_id) +
            " last-source=" +
            (command_trace.empty() ? std::string("<none>") : command_trace.back().source_id));
    }
    {
        const std::string diagnostic =
            "the advertised primary-modifier+V gesture must execute the enabled Paste command";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::v, primary});
        require(key_handled, diagnostic);
    }
    struct PastedFilePublished final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const TemporaryTree& tree_fixture;
        bool operator()() const {
            const bool ready =
                std::filesystem::exists(tree_fixture.root() / "Documents" / "root.txt") &&
                has_object_named(*objects, "root.txt");
            return ready;
        }
    };
    require_eventually(
        *application, PastedFilePublished{objects, tree_fixture},
        "shortcut Paste must publish the staged file and refresh the visible destination");

    {
        const std::string diagnostic =
            "Alt+Left must execute the shared Back command from the object surface";
        const bool focus_requested = (*window).request_focus(objects);
        require(focus_requested, diagnostic);
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::left, gui_forms::Modifier::alt});
        require(key_handled, diagnostic);
    }
    struct ShortcutBackReady final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        bool operator()() const {
            const bool ready = has_object_named(*objects, "Documents") &&
                               !has_object_named(*objects, "inside.txt");
            return ready;
        }
    };
    require_eventually(*application, ShortcutBackReady{objects},
                       "Back shortcut must restore the previous real folder");
    require(!command_trace.empty() && command_trace.back().command_id == "go.back" &&
                command_trace.back().source_id == "fm.accelerator.go.back",
            "Back shortcut must publish the canonical navigation command identity");
    {
        const std::string diagnostic = "Alt+Right must execute the shared Forward command";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::right, gui_forms::Modifier::alt});
        require(key_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "inside.txt"},
                       "Forward shortcut must restore the next real folder");
    {
        const std::string diagnostic = "Alt+Up must execute the shared bounded Up command";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::up, gui_forms::Modifier::alt});
        require(key_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "Documents"},
                       "Up shortcut must return to the admitted parent");

    const std::string empty = object_id(*objects, "Empty");
    {
        const std::string diagnostic =
            "empty-folder fixture must open through the same command accelerator";
        require(!empty.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(empty, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct EmptyFolderReady final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        bool operator()() const {
            const bool ready = (*objects).items().empty();
            return ready;
        }
    };
    require_eventually(*application, EmptyFolderReady{objects},
                       "empty folder must publish an actually empty visible ObjectView");
    {
        const std::string diagnostic = "Edit menu must remain an honest surface in an empty folder";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.edit", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    gui_forms::SemanticSnapshot semantics = (*window).semantic_snapshot();
    const gui_forms::SemanticNode* empty_select_all =
        find_semantic(semantics.roots, "fm.application.menu.menu.popup.row.edit.select-all");
    {
        const std::string diagnostic =
            "empty-folder Select All must explain and enforce its disabled state";
        require(empty_select_all &&
                    !gui_forms::has_semantic_state((*empty_select_all).states,
                                                   gui_forms::SemanticState::enabled) &&
                    (*empty_select_all).description.find("no visible objects") !=
                        std::string::npos &&
                    !has_action(*empty_select_all, gui_forms::SemanticAction::press),
                diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.edit.select-all", gui_forms::SemanticAction::press);
        require(!semantic_action_handled, diagnostic);
    }
    {
        const std::string diagnostic =
            "disabled empty-folder Select All shortcut must perform no work";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        const bool key_handled_2 = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::a, primary});
        require(!key_handled_2, diagnostic);
    }
    {
        const std::string diagnostic = "Alt+Up must leave the empty child folder";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::up, gui_forms::Modifier::alt});
        require(key_handled, diagnostic);
    }
    require_eventually(*application, ObjectNamed{*objects, "root.txt"},
                       "returning to the root must restore nonempty folder commands");

    if (symlink_available) {
        const std::string symlink = object_id(*objects, "root-link.txt");
        {
            const std::string diagnostic =
                "symlink refusal requires one exact selected symlink identity";
            require(!symlink.empty(), diagnostic);
            const bool semantic_action_handled =
                (*window).perform_semantic_action(symlink, gui_forms::SemanticAction::select);
            require(semantic_action_handled, diagnostic);
        }
        const std::size_t invocation_count = command_trace.size();
        {
            const std::string diagnostic =
                "selected symlink must open the same retained object context surface";
            const bool semantic_action_handled =
                (*window).perform_semantic_action(symlink, gui_forms::SemanticAction::show_menu);
            require(semantic_action_handled, diagnostic);
        }
        semantics = (*window).semantic_snapshot();
        const gui_forms::SemanticNode* symlink_context_open =
            find_semantic(semantics.roots, "fm.context.object.popup.row.open");
        {
            const std::string diagnostic =
                "semantic symlink activation must remain visible but cannot bypass the disabled "
                "canonical Open command";
            require(symlink_context_open &&
                        !gui_forms::has_semantic_state((*symlink_context_open).states,
                                                       gui_forms::SemanticState::enabled) &&
                        (*symlink_context_open).description.find("Symbolic-link") !=
                            std::string::npos &&
                        !has_action(*symlink_context_open, gui_forms::SemanticAction::press),
                    diagnostic);
            const bool semantic_action_handled = (*window).perform_semantic_action(
                "fm.context.object.popup.row.open", gui_forms::SemanticAction::press);
            require(!semantic_action_handled, diagnostic);
            const bool key_handled = (*window).dispatch_key(
                {gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
            require(key_handled, diagnostic);
            require(!(*open_command).state().enabled &&
                        (*open_command).state().availability_reason.find("Symbolic-link") !=
                            std::string::npos,
                    diagnostic);
            const bool semantic_action_handled_2 =
                (*window).perform_semantic_action(symlink, gui_forms::SemanticAction::press);
            require(semantic_action_handled_2, diagnostic);
            require(command_trace.size() == invocation_count &&
                        has_object_named(*objects, "root-link.txt"),
                    diagnostic);
        }
    }

    const std::uint64_t synthetic_generation =
        file_manager::ApplicationInteractionProbe::show_search_results(
            *application, "root",
            {{"root.txt", tree_fixture.root() / "root.txt", "file", 5U, false}});
    require((*correspondence).visible() && !(*objects).visible() && !(*view_button).enabled() &&
                !(*sort_button).enabled(),
            "correspondence search must disable permanent folder-only View and Sort controls");
    require_unique_semantic_ids(*window);
    (*window).perform_layout();
    const std::string correspondence_result = (*correspondence).items().front().stable_id;
    {
        const std::string diagnostic =
            "search background context fixture must start with one synchronized visible result "
            "selection";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            correspondence_result, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
        require((*correspondence).selected_id() == correspondence_result &&
                    (*objects).selected_id() == correspondence_result,
                diagnostic);
    }
    const gui_forms::Rect correspondence_bounds = (*correspondence).absolute_bounds();
    const std::optional<gui_forms::Rect> correspondence_row =
        (*correspondence).item_bounds(correspondence_result);
    require(correspondence_row.has_value(),
            "search background fixture requires one realized result row");
    const gui_forms::Point correspondence_background{
        correspondence_bounds.x + correspondence_bounds.width * .5,
        correspondence_bounds.y + (*correspondence_row).y + (*correspondence_row).height + 8.0};
    const bool background_down = (*window).dispatch_pointer({gui_forms::PointerAction::down,
                                                             gui_forms::PointerButton::secondary,
                                                             correspondence_background});
    const bool background_up = (*window).dispatch_pointer({gui_forms::PointerAction::up,
                                                           gui_forms::PointerButton::secondary,
                                                           correspondence_background});
    const bool has_new_folder =
        (*window).find("fm.context.background.popup.row.new-folder") != nullptr;
    const bool has_properties =
        (*window).find("fm.context.background.popup.row.properties") != nullptr;
    const bool has_open = (*window).find("fm.context.background.popup.row.open") != nullptr;
    if (!background_down || !background_up || !(*correspondence).selected_id().empty() ||
        !(*objects).selected_ids().empty() || !has_new_folder || !has_properties || has_open) {
        throw std::runtime_error(
            "empty search background context mismatch: down=" + std::to_string(background_down) +
            " up=" + std::to_string(background_up) +
            " correspondence-selection=" + std::string((*correspondence).selected_id()) +
            " object-selection=" + std::string((*objects).selected_id()) +
            " new-folder=" + std::to_string(has_new_folder) +
            " properties=" + std::to_string(has_properties) + " open=" + std::to_string(has_open) +
            " control=" + std::to_string(correspondence_bounds.x) + "," +
            std::to_string(correspondence_bounds.y) + "," +
            std::to_string(correspondence_bounds.width) + "," +
            std::to_string(correspondence_bounds.height) +
            " point=" + std::to_string(correspondence_background.x) + "," +
            std::to_string(correspondence_background.y) +
            " row=" + std::to_string((*correspondence_row).x) + "," +
            std::to_string((*correspondence_row).y) + "," +
            std::to_string((*correspondence_row).width) + "," +
            std::to_string((*correspondence_row).height));
    }
    {
        const std::string diagnostic =
            "search background menu must close through Escape and restore focus to the visible "
            "correspondence surface";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        require((*window).focused_control() == correspondence, diagnostic);
    }
    {
        const std::string diagnostic = "Edit menu must expose the search-context command state";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.edit", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    semantics = (*window).semantic_snapshot();
    const gui_forms::SemanticNode* search_select_all =
        find_semantic(semantics.roots, "fm.application.menu.menu.popup.row.edit.select-all");
    require(search_select_all &&
                (*search_select_all).description.find("one factual selection") !=
                    std::string::npos &&
                !has_action(*search_select_all, gui_forms::SemanticAction::press),
            "search Select All must be disabled with its single-selection authority reason");
    {
        const std::string diagnostic =
            "search correspondence must not pretend to execute Select All";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
        const bool focus_requested = (*window).request_focus(correspondence);
        require(focus_requested, diagnostic);
        const bool key_handled_2 = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::a, primary});
        require(!key_handled_2, diagnostic);
    }

    {
        const std::string diagnostic =
            "Refresh in search must repeat the Engine query without clearing or replacing the "
            "visible surface";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.view", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.view.refresh", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require(file_manager::ApplicationInteractionProbe::search_generation(*application) >
                        synthetic_generation &&
                    file_manager::ApplicationInteractionProbe::search_showing(*application) &&
                    file_manager::ApplicationInteractionProbe::search_loading(*application) &&
                    file_manager::ApplicationInteractionProbe::filter(*application) == "root" &&
                    (*search).text() == "root",
                diagnostic);
    }

    {
        const std::string diagnostic = "Settings menu row must open the owned settings surface";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.file", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.file.settings", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require(file_manager::ApplicationInteractionProbe::settings_open(*application), diagnostic);
    }
    {
        const std::string diagnostic =
            "persistent File menu must expose settings-context command truth";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.file", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    require_unique_semantic_ids(*window);
    semantics = (*window).semantic_snapshot();
    const gui_forms::SemanticNode* settings_back =
        find_semantic(semantics.roots, "fm.application.menu.menu.popup.row.file.settings");
    const gui_forms::SemanticNode* settings_open =
        find_semantic(semantics.roots, "fm.application.menu.menu.popup.row.file.choose-open");
    {
        const std::string diagnostic = "Settings must disable hidden-workspace Open and accurately "
                                       "name its one return command";
        require(settings_back && (*settings_back).name == "Back to files" && settings_open &&
                    !has_action(*settings_open, gui_forms::SemanticAction::press) &&
                    (*settings_open).description.find("Return to Files") != std::string::npos,
                diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.file.settings", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require(!file_manager::ApplicationInteractionProbe::settings_open(*application) &&
                    (*window).focused_control() == correspondence,
                diagnostic);
    }

    {
        const std::string diagnostic =
            "search Escape must clear the query through the retained TextBox contract";
        const bool focus_requested = (*window).request_focus(search);
        require(focus_requested, diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }
    struct FilesSurfaceRestored final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<gui_forms::CorrespondenceView>& correspondence;
        bool operator()() const {
            const bool ready = (*objects).visible() && !(*correspondence).visible() &&
                               has_object_named(*objects, "root.txt");
            return ready;
        }
    };
    require_eventually(*application, FilesSurfaceRestored{objects, correspondence},
                       "clearing search must restore the nonempty folder surface");
    require((*window).focused_control() == objects,
            "leaving correspondence search must focus the visible ObjectView");
    {
        const std::string diagnostic = "Edit menu must reopen after leaving search";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.edit", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    semantics = (*window).semantic_snapshot();
    const gui_forms::SemanticNode* restored_select_all =
        find_semantic(semantics.roots, "fm.application.menu.menu.popup.row.edit.select-all");
    require(restored_select_all &&
                gui_forms::has_semantic_state((*restored_select_all).states,
                                              gui_forms::SemanticState::enabled) &&
                has_action(*restored_select_all, gui_forms::SemanticAction::press) &&
                host_observation.close_requests == 0U,
            "nonempty Files must restore the same actionable Select All command without spuriously "
            "closing the window");

    {
        const std::string diagnostic =
            "Edit popup must dismiss before moving semantic focus to Help";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }
    const bool help_opened =
        (*window).perform_semantic_action("fm.menu.help", gui_forms::SemanticAction::expand);
    const bool about_invoked = (*window).perform_semantic_action(
        "fm.application.menu.menu.popup.row.help.about", gui_forms::SemanticAction::press);
    require(help_opened && about_invoked &&
                (*status_ready).text() == "File Manager · 0.001-alpha development build" &&
                (*status_summary).text() == "local filesystem authority · GUI.Forms + Web.Forms",
            "Help About row must publish the exact development version and local authority even "
            "without a dialog backend; ready=" +
                (*status_ready).text() + "; summary=" + (*status_summary).text() + "; help=" +
                std::to_string(help_opened) + "; about=" + std::to_string(about_invoked));
    {
        const std::string diagnostic =
            "File Close row must invoke the bound native-close request exactly once";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.file", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.application.menu.menu.popup.row.file.close", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
        require(host_observation.close_requests == 1U, diagnostic);
    }

    (*application).stop();
}

void test_criteria_virtual_folder_is_retained_exact_and_catalogue_only() {
    TemporaryTree tree_fixture{};
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), std::nullopt, false,
                                                    "fixture-root");
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);

    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    std::shared_ptr<gui_forms::InstrumentRack> rack =
        std::dynamic_pointer_cast<gui_forms::InstrumentRack>((*window).find("fm.criteria.rack"));
    gui_forms::Control::Ptr console = (*window).find("fm.criteria.console");
    std::shared_ptr<gui_forms::Label> title =
        std::dynamic_pointer_cast<gui_forms::Label>((*window).find("fm.criteria.title"));
    std::shared_ptr<gui_forms::Label> action_state =
        std::dynamic_pointer_cast<gui_forms::Label>((*window).find("fm.criteria.actions.state"));
    std::shared_ptr<gui_forms::Button> add =
        std::dynamic_pointer_cast<gui_forms::Button>((*window).find("fm.criteria.add"));
    std::shared_ptr<gui_forms::Command> criteria_command =
        file_manager::ApplicationInteractionProbe::command(*application, "view.criteria");
    require(objects && rack && console && title && action_state && add && criteria_command &&
                !(*console).visible(),
            "Criteria fixture must own one initially hidden retained console and command");
    require_eventually(*application, ObjectNamed{*objects, "root.txt"},
                       "Criteria fixture must begin from a real enumerated directory");

    file_manager::ApplicationInteractionProbe::enter_criteria(*application);
    (*window).perform_layout();
    require((*console).visible() && (*objects).visible() && (*objects).items().empty() &&
                !has_object_named(*objects, "root.txt") && (*objects).selected_ids().empty() &&
                !(*std::dynamic_pointer_cast<gui_forms::CorrespondenceView>(
                      (*window).find("fm.search.correspondence")))
                     .visible() &&
                (*criteria_command).state().checked && (*rack).modules().size() == 2U &&
                (*rack).modules()[0].stable_id == "fm.criteria.kind" &&
                (*rack).modules()[1].stable_id == "fm.criteria.modified" &&
                (*rack).field_editor("fm.criteria.kind", "field") &&
                (*rack).field_editor("fm.criteria.kind", "operator") &&
                (*rack).field_editor("fm.criteria.kind", "value") &&
                (*window).find("fm.criteria.kind.enable") &&
                (*window).find("fm.criteria.kind.remove") &&
                (*action_state).text() == "2 exact filters · catalogue only",
            "Criteria must clear the prior directory immediately and present a checked retained "
            "instrument rack above the ordinary ObjectView with field/operator/value/enable/remove "
            "controls");
    {
        const std::string diagnostic =
            "persistent View menu must expose Criteria as a checked content mode";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.menu.view", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.application.menu.menu.popup.row.view.criteria") != nullptr,
                diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }

    std::optional<std::vector<fileman::orchestrator::SearchExactFilter>> filters =
        file_manager::ApplicationInteractionProbe::criteria_filters(*application);
    require(filters && (*filters).size() == 2U && (*filters)[0].field == "kind" &&
                (*filters)[0].value == "file" && (*filters)[1].field == "modified_after" &&
                (*filters)[1].value == "1767225600000000000",
            "default Kind and Modified modules must compile to exact Engine filters and a UTC day "
            "boundary");

    std::shared_ptr<gui_forms::TextBox> modified_value =
        std::dynamic_pointer_cast<gui_forms::TextBox>(
            (*rack).field_editor("fm.criteria.modified", "value"));
    require(static_cast<bool>(modified_value),
            "Modified criterion must own a retained text editor");
    (*modified_value).set_text("2026-02-30");
    filters = file_manager::ApplicationInteractionProbe::criteria_filters(*application);
    require(!filters && (*rack).modules()[1].state == gui_forms::InstrumentModuleState::invalid &&
                !(*rack).modules()[1].fields[2].validation_message.empty(),
            "invalid calendar input must fail locally and mark the retained module invalid");
    (*modified_value).set_text("2026-01-01");
    {
        const std::string diagnostic = "Modified module must support retained enable state";
        const bool module_enabled = (*rack).set_module_enabled("fm.criteria.modified", false);
        require(module_enabled, diagnostic);
    }
    filters = file_manager::ApplicationInteractionProbe::criteria_filters(*application);
    require(filters && (*filters).size() == 1U && (*filters).front().field == "kind",
            "disabled criteria must emit no Engine predicate");

    file_manager::ApplicationInteractionProbe::add_criteria_module(*application);
    require(
        (*rack).modules().size() == 3U && (*rack).field_editor("fm.criteria.size", "field") &&
            (*rack).field_editor("fm.criteria.size", "operator") &&
            (*rack).field_editor("fm.criteria.size", "value") && !(*add).enabled(),
        "bounded add action must create the third admitted Size module and then disable itself");
    file_manager::ApplicationInteractionProbe::remove_criteria_module(*application,
                                                                      "fm.criteria.kind");
    require((*rack).modules().size() == 2U && !(*rack).field_editor("fm.criteria.kind", "value") &&
                (*add).enabled(),
            "remove must delete one retained module and reopen the bounded add action");
    file_manager::ApplicationInteractionProbe::add_criteria_module(*application);
    require(
        (*rack).modules().size() == 3U && (*rack).field_editor("fm.criteria.kind", "value"),
        "add must restore the next missing admitted module rather than duplicate an existing one");

    file_manager::ApplicationInteractionProbe::show_criteria_results(
        *application, "catalogue", 42U,
        {{"root.txt", tree_fixture.root() / "root.txt", "file", 5U, false},
         {"Documents", tree_fixture.root() / "Documents", "directory", 0U, false}});
    (*window).perform_layout();
    require(has_object_named(*objects, "root.txt") && has_object_named(*objects, "Documents") &&
                (*objects).visible() &&
                (*title).text().find("GENERATION 42") != std::string::npos &&
                (*console).committed_arranged_bounds().height > 90.0 &&
                (*objects).committed_arranged_bounds().y >=
                    (*console).committed_arranged_bounds().height,
            "catalogue Criteria results must remain ordinary selectable objects below a visible "
            "generation-labelled rack");

    file_manager::ApplicationInteractionProbe::show_criteria_results(
        *application, "live", std::nullopt,
        {{"root.txt", tree_fixture.root() / "root.txt", "file", 5U, false}});
    require(
        (*objects).items().size() == 2U &&
            !file_manager::ApplicationInteractionProbe::criteria_loading(*application),
        "a filtered live-source page must be refused without replacing the last catalogue objects");

    (*window).resize({560.0, 620.0});
    (*window).perform_layout();
    const std::optional<gui_forms::Rect> kind_bounds = (*rack).module_bounds("fm.criteria.kind");
    const std::optional<gui_forms::Rect> size_bounds = (*rack).module_bounds("fm.criteria.size");
    require(kind_bounds && size_bounds &&
                std::max((*kind_bounds).y, (*size_bounds).y) >
                    std::min((*kind_bounds).y, (*size_bounds).y) &&
                (*console).committed_arranged_bounds().height > 150.0,
            "narrow Criteria layout must wrap retained modules and grow the shallow console "
            "instead of clipping them");

    file_manager::ApplicationInteractionProbe::set_criteria_logically_showing(*application, true);
    {
        const std::string diagnostic =
            "checked Criteria command must exit through ordinary navigation";
        const bool command_executed = (*criteria_command).execute("fixture.criteria.exit");
        require(command_executed, diagnostic);
    }
    struct CriteriaExited final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const gui_forms::Control::Ptr& console;
        bool operator()() const {
            const bool ready = !(*console).visible() && has_object_named(*objects, "root.txt");
            return ready;
        }
    };
    require_eventually(*application, CriteriaExited{objects, console},
                       "exiting Criteria must restore a real directory ObjectView");
    require(!file_manager::ApplicationInteractionProbe::criteria_showing(*application),
            "ordinary navigation must clear Criteria logical state");
    require_unique_semantic_ids(*window);
    (*application).stop();
}

void test_settings_tabs_and_transaction_actions_are_truthful() {
    TemporaryTree tree_fixture{};
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), std::nullopt, false,
                                                    std::string{});
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();

    fileman::orchestrator::SettingsSchemaInfo schema{};
    schema.schema_revision = "fixture-schema";
    schema.fields = {
        {"general.restore_last_location",
         "file_manager",
         "general",
         "settings.general.restore_last_location",
         "boolean",
         true,
         std::nullopt,
         std::nullopt,
         {},
         "none",
         "deferred",
         "session persistence is not admitted"},
        {"appearance.density",
         "file_manager",
         "appearance_access",
         "settings.appearance.density",
         "string",
         std::string("comfortable"),
         std::nullopt,
         std::nullopt,
         {"comfortable", "compact"},
         "window_recompose",
         "available",
         "retained layout density is application-owned"},
    };
    fileman::orchestrator::SettingsSnapshotInfo snapshot{};
    snapshot.schema_revision = schema.schema_revision;
    snapshot.revision = 7U;
    snapshot.recovery_provenance = "fixture";
    snapshot.values = {
        {"general.restore_last_location", true},
        {"appearance.density", std::string("compact")},
    };
    file_manager::ApplicationInteractionProbe::install_settings_state(
        *application, std::move(schema), std::move(snapshot));
    file_manager::ApplicationInteractionProbe::show_settings(*application);
    (*window).perform_layout();

    std::shared_ptr<gui_forms::Button> general = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.body.tabs.general"));
    std::shared_ptr<gui_forms::Button> appearance = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.body.tabs.appearance"));
    std::shared_ptr<gui_forms::Button> applications = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.body.tabs.applications"));
    std::shared_ptr<gui_forms::Button> reset = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.actions.reset"));
    std::shared_ptr<gui_forms::Button> cancel = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.actions.cancel"));
    std::shared_ptr<gui_forms::Button> apply = std::dynamic_pointer_cast<gui_forms::Button>(
        (*window).find("file-manager-app.shell.settings.actions.apply"));
    std::shared_ptr<gui_forms::Label> status = std::dynamic_pointer_cast<gui_forms::Label>(
        (*window).find("file-manager-app.shell.settings.actions.status"));
    require(general && appearance && applications && reset && cancel && apply && status &&
                (*general).selected() && !(*appearance).selected() && !(*applications).selected() &&
                !(*reset).enabled() &&
                (*reset).accessible_description().find("no available") != std::string::npos,
            "General must begin as the sole selected settings category and cannot reset "
            "deferred-only values");
    const std::optional<gui_forms::ControlStateRecipes> selected_recipes =
        (*general).visual_recipes_override();
    const std::optional<gui_forms::ControlStateRecipes> normal_recipes =
        (*appearance).visual_recipes_override();
    require(selected_recipes && normal_recipes && *selected_recipes != *normal_recipes,
            "settings fixture requires distinct authored selected and ordinary tab recipes");

    {
        const std::string diagnostic =
            "Appearance selection must move retained semantic/visual tab state and enable a "
            "meaningful reset";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*appearance).stable_id().value(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require(!(*general).selected() && (*appearance).selected() &&
                    (*general).visual_recipes_override() == normal_recipes &&
                    (*appearance).visual_recipes_override() == selected_recipes &&
                    (*reset).enabled(),
                diagnostic);
    }
    std::shared_ptr<gui_forms::PropertyList> properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            (*window).find("fm.settings.properties"));
    {
        const std::string diagnostic =
            "Reset page must act on the visible nondefault typed Appearance value";
        require(properties && (*properties).value("appearance.density") == "compact", diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*reset).stable_id().value(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    properties = std::dynamic_pointer_cast<gui_forms::PropertyList>(
        (*window).find("fm.settings.properties"));
    require(properties && (*properties).value("appearance.density") == "comfortable" &&
                !(*reset).enabled() && (*cancel).enabled() && (*apply).enabled() &&
                (*status).text() == "Page defaults staged · Apply commits them",
            "staged defaults must update the visible value and exact Apply/Cancel/Reset states");
    {
        const std::string diagnostic = "Cancel must discard the staged settings transaction";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*cancel).stable_id().value(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    properties = std::dynamic_pointer_cast<gui_forms::PropertyList>(
        (*window).find("fm.settings.properties"));
    require(properties && (*properties).value("appearance.density") == "compact" &&
                (*reset).enabled() && !(*cancel).enabled() && !(*apply).enabled() &&
                (*status).text() == "Pending changes cancelled",
            "Cancel must restore committed values and recompute every settings action");

    {
        const std::string diagnostic = "empty Applications category must become the sole selected "
                                       "tab without advertising Reset";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*applications).stable_id().value(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require(!(*general).selected() && !(*appearance).selected() && (*applications).selected() &&
                    !(*reset).enabled() &&
                    (*reset).accessible_description().find("no available") != std::string::npos,
                diagnostic);
    }
    file_manager::ApplicationInteractionProbe::reset_settings_page(*application);
    require((*status).text() == "Page values already match declared defaults" &&
                !(*reset).enabled(),
            "defensive no-op Reset must report that no defaults were staged");

    (*application).stop();
}

void test_property_name_rename_is_protected_and_collision_safe() {
    TemporaryTree tree_fixture{};
    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(tree_fixture.root(), tree_fixture.quarantine(),
                                                    true, std::string{});
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);

    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    std::shared_ptr<gui_forms::PropertyList> properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            (*window).find("fm.selection.properties"));
    require(objects && properties, "protected rename test requires object and property surfaces");
    require_eventually(*application, ObjectNamed{*objects, "root.txt"},
                       "protected root must enumerate before property rename");

    const std::string original_id = object_id(*objects, "root.txt");
    {
        const std::string diagnostic = "property rename requires one selected filesystem object";
        require(!original_id.empty(), diagnostic);
        const bool semantic_action_handled =
            (*window).perform_semantic_action(original_id, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
    }
    std::shared_ptr<gui_forms::TextBox> name_editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*properties).editor("fm.property.name"));
    require(name_editor && (*name_editor).enabled() &&
                (*properties).value("fm.property.name") == "root.txt",
            "Name property must be inset-editable in the protected scope");

    const std::shared_ptr<gui_forms::DropDownButton> move_copy_button =
        std::dynamic_pointer_cast<gui_forms::DropDownButton>(
            (*window).find("file-manager-app.shell.commands.selection-group.actions.copy"));
    {
        const std::string diagnostic =
            "permanent Move/Copy popup must contain only the admitted pair";
        require(move_copy_button && (*move_copy_button).effectively_enabled(), diagnostic);
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*move_copy_button).stable_id().value(), gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
        require((*window).find("fm.shelf.move-copy.popup.row.copy") &&
                    (*window).find("fm.shelf.move-copy.popup.row.move") &&
                    !(*window).find("fm.shelf.move-copy.popup.row.paste"),
                diagnostic);
    }
    {
        const std::string diagnostic = "Move/Copy popup must close through Escape";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }

    (*window).resize({220.0, 850.0});
    (*window).perform_layout();
    {
        const std::string diagnostic = "narrow protected shelf must open its Commands menu";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.commands.overflow", gui_forms::SemanticAction::show_menu);
        require(semantic_action_handled, diagnostic);
    }
    require((*window).find("fm.shelf.more-menu.popup.row.move-copy") != nullptr,
            "narrow Commands menu must retain the Move/Copy member");
    {
        const std::string diagnostic = "narrow Commands Move/Copy member must expand its submenu";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.shelf.more-menu.popup.row.move-copy", gui_forms::SemanticAction::expand);
        require(semantic_action_handled, diagnostic);
    }
    require((*window).find("fm.shelf.more-menu.popup.row.copy") != nullptr,
            "narrow shelf Move/Copy submenu must retain Copy");
    require((*window).find("fm.shelf.more-menu.popup.row.move") != nullptr,
            "narrow shelf Move/Copy submenu must retain Move");
    require((*window).find("fm.shelf.more-menu.popup.row.paste") == nullptr,
            "narrow shelf Move/Copy submenu must not leak Paste into permanent chrome");
    {
        const std::string diagnostic = "narrow protected shelf popup must close through Escape";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::escape});
        require(key_handled, diagnostic);
    }
    (*window).resize({1340.0, 850.0});
    (*window).perform_layout();

    {
        const std::string diagnostic = "Name property editor must accept retained focus";
        const bool focus_requested = (*window).request_focus(name_editor);
        require(focus_requested, diagnostic);
    }
    (*name_editor).select_all();
    {
        const std::string diagnostic = "Name property must commit through normal text input";
        const bool text_handled = (*window).dispatch_text({"Documents"});
        require(text_handled, diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct RenameCollisionRefused final {
        const TemporaryTree& tree_fixture;
        const std::shared_ptr<gui_forms::PropertyList>& properties;
        bool operator()() const {

            const bool ready =
                (*properties).value("fm.property.name") == "root.txt" &&
                std::filesystem::exists(tree_fixture.root() / "root.txt") &&
                !std::filesystem::exists(tree_fixture.root() / "Documents" / "root.txt");
            return ready;
        }
    };
    require_eventually(*application, RenameCollisionRefused{tree_fixture, properties},
                       "colliding property rename must roll back without moving the object");

    {
        const std::string diagnostic = "Name property editor must recover after a refused rename";
        const bool focus_requested = (*window).request_focus(name_editor);
        require(focus_requested, diagnostic);
    }
    (*name_editor).select_all();
    {
        const std::string diagnostic = "Name property must accept a second collision-free commit";
        const bool text_handled = (*window).dispatch_text({"renamed.txt"});
        require(text_handled, diagnostic);
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct RenamedFilePublished final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const TemporaryTree& tree_fixture;
        bool operator()() const {

            const bool ready = std::filesystem::exists(tree_fixture.root() / "renamed.txt") &&
                               !std::filesystem::exists(tree_fixture.root() / "root.txt") &&
                               has_object_named(*objects, "renamed.txt");
            return ready;
        }
    };
    require_eventually(*application, RenamedFilePublished{objects, tree_fixture},
                       "successful property rename must refresh the real protected directory");

    (*application).stop();
}

void adaptive_noop() {}

void require_inside(const gui_forms::Control::Ptr& control, const gui_forms::Size viewport,
                    const double minimum_height, const std::string& label) {
    const gui_forms::Rect rectangle = (*control).absolute_bounds();
    require((*control).effectively_visible() && rectangle.x >= -0.01 && rectangle.y >= -0.01 &&
                rectangle.width >= 60.0 && rectangle.height >= minimum_height &&
                rectangle.x + rectangle.width <= viewport.width + 0.01 &&
                rectangle.y + rectangle.height <= viewport.height + 0.01,
            label + " must fit the viewport with usable dimensions: " +
                std::to_string(rectangle.x) + "," + std::to_string(rectangle.y) + "," +
                std::to_string(rectangle.width) + "," + std::to_string(rectangle.height));
}

void test_adaptive_layout_preserves_fields_commands_and_selection() {
    TemporaryTree fixture{};
    const std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(fixture.root(), std::nullopt, false,
                                                    "fixture-adaptive-root");
    ApplicationStopGuard stop_guard(*application);
    const std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(adaptive_noop, adaptive_noop);
    const std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    require_eventually(
        *application,
        std::bind_front(has_object_named, std::cref(*objects), std::string_view("root.txt")),
        "adaptive fixture must enumerate");
    const gui_forms::Control::Ptr path_host =
        (*window).find("file-manager-app.shell.location.path-host");
    const gui_forms::Control::Ptr search_host =
        (*window).find("file-manager-app.shell.location.search-host");
    const gui_forms::Control::Ptr preview = (*window).find(
        "file-manager-app.shell.workspace.selection.inspector.facts.preview.surface");
    const std::shared_ptr<gui_forms::TextBox> path =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.path.editor"));
    const std::shared_ptr<gui_forms::TextBox> search =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.search.current-folder"));
    const std::shared_ptr<gui_forms::MenuStrip> menu =
        std::dynamic_pointer_cast<gui_forms::MenuStrip>((*window).find("fm.application.menu"));
    const std::string selected = object_id(*objects, "root.txt");
    {
        const std::string diagnostic = "adaptive fixture selection";
        const bool semantic_action_handled =
            (*window).perform_semantic_action(selected, gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
    }
#if defined(__APPLE__)
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::meta;
#else
    constexpr gui_forms::Modifier primary = gui_forms::Modifier::control;
#endif
    for (const double scale : {1.0, 1.5, 2.0}) {
        (*window).set_scale(scale);
        for (const gui_forms::Size size : {gui_forms::Size{150, 150},
                                           {360, 260},
                                           {540, 620},
                                           {800, 320},
                                           {1340, 850},
                                           {1920, 1080}}) {
            (*window).resize(size);
            (*window).perform_layout();
            if ((*objects).absolute_bounds().height < 64.0) {
                const std::shared_ptr<gui_forms::ResponsiveTrackPanel> shell =
                    std::dynamic_pointer_cast<gui_forms::ResponsiveTrackPanel>(
                        (*window).find("file-manager-app.shell"));
                const gui_forms::ResponsiveLayoutSnapshot snapshot = (*shell).layout_snapshot();
                std::cerr << "adaptive scale=" << scale << " viewport=" << size.width << ','
                          << size.height << '\n';
                for (const gui_forms::ResponsiveTrackResult& track : snapshot.resolution.tracks) {
                    std::cerr << "track min=" << track.minimum << " allocated=" << track.allocated
                              << " collapsed=" << track.collapsed() << '\n';
                }
            }
            require_inside(objects, size, 64.0, "content");
            require_inside(path_host, size, 28.0, "location field");
            if (size.width >= 420 && size.height >= 360)
                require_inside(search_host, size, 28.0, "search field");
            if (size.width == 540)
                require((*search_host).absolute_bounds().y >=
                            (*path_host).absolute_bounds().y + 30.0,
                        "medium viewport must stack distinct path and query fields");
            require((*objects).selected_id() == selected,
                    "resize/DPI transitions must preserve exact selection");
            require((*objects).font().size == 13.0 && (*menu).font().size == 13.0,
                    "adaptation must not shrink readable fonts");
            if (size.height < 560)
                require(!(*preview).effectively_visible(), "short inspector must compact preview");
        }
    }
    (*window).resize({150, 150});
    (*window).perform_layout();
    require((*menu).items().size() == 1U && (*menu).items().front().items.size() == 7U,
            "compact menu must retain every original menu category");
    {
        const std::string diagnostic =
            "Ctrl/Command+F must reveal and focus search at minimum size";
        const bool focus_requested = (*window).request_focus(objects);
        require(focus_requested, diagnostic);
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::f, primary});
        require(key_handled, diagnostic);
    }
    (*window).perform_layout();
    require_inside(search_host, {150, 150}, 28.0, "minimum search field");
    require((*window).focused_control() == search && !(*path_host).effectively_visible(),
            "minimum search uses its own retained field");
    (*search).set_text("unsubmitted query draft");
    {
        const std::string diagnostic =
            "Ctrl/Command+L must return to exact location from minimum search";
        const bool key_handled = (*window).dispatch_key(
            {gui_forms::KeyAction::down, gui_forms::PhysicalKey::l, primary});
        require(key_handled, diagnostic);
    }
    (*window).perform_layout();
    require_inside(path_host, {150, 150}, 28.0, "minimum path editor");
    require((*window).focused_control() == path && !(*search_host).effectively_visible(),
            "path and search remain distinct controls");
    (*path).set_text("uncommitted location draft");
    (*window).resize({540, 620});
    (*window).perform_layout();
    require((*path).text() == "uncommitted location draft" && (*window).focused_control() == path,
            "reflow must preserve the path draft and focus");
    (*window).resize({1340, 850});
    (*window).perform_layout();
    require((*menu).items().size() == 7U && (*preview).effectively_visible() &&
                (*objects).selected_id() == selected &&
                (*search).text() == "unsubmitted query draft",
            "wide restoration must recover menu geography, preview and exact selection");
}

void test_installed_criteria_route_when_requested() {
    const char* root_value = std::getenv("FILE_MANAGER_INSTALLED_CRITERIA_ROOT");
    const char* root_id_value = std::getenv("FILE_MANAGER_INSTALLED_CRITERIA_ROOT_ID");
    if (root_value == nullptr && root_id_value == nullptr)
        return;
    require(root_value != nullptr && root_id_value != nullptr,
            "installed Criteria probe requires both root and root ID environment variables");

    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(root_value, std::nullopt, false, root_id_value);
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);
    std::shared_ptr<gui_forms::ObjectView> objects =
        std::dynamic_pointer_cast<gui_forms::ObjectView>(
            (*window).find("fm.objects.current-folder"));
    std::shared_ptr<gui_forms::InstrumentRack> rack =
        std::dynamic_pointer_cast<gui_forms::InstrumentRack>((*window).find("fm.criteria.rack"));
    std::shared_ptr<gui_forms::Label> title =
        std::dynamic_pointer_cast<gui_forms::Label>((*window).find("fm.criteria.title"));
    std::shared_ptr<gui_forms::CorrespondenceView> correspondence =
        std::dynamic_pointer_cast<gui_forms::CorrespondenceView>(
            (*window).find("fm.search.correspondence"));
    std::shared_ptr<gui_forms::PropertyList> properties =
        std::dynamic_pointer_cast<gui_forms::PropertyList>(
            (*window).find("fm.selection.properties"));
    std::shared_ptr<gui_forms::Command> command =
        file_manager::ApplicationInteractionProbe::command(*application, "view.criteria");
    require(objects && rack && title && correspondence && properties && command,
            "installed Criteria probe requires the production retained surface");
    struct InstalledRootReady final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        bool operator()() const {
            const bool ready = !(*objects).items().empty();
            return ready;
        }
    };
    require_eventually(*application, InstalledRootReady{objects},
                       "installed Criteria probe must first enumerate the real root");
    {
        const std::string diagnostic =
            "installed Criteria command must be available for the admitted root";
        const bool command_executed = (*command).execute("installed.criteria.probe");
        require(command_executed, diagnostic);
    }
    struct CatalogueResultsReady final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::criteria_showing(*application) &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty();
            return ready;
        }
    };
    require_eventually(*application, CatalogueResultsReady{objects, application},
                       "installed Criteria route must return real catalogue objects");
    require((*command).state().checked && (*rack).visible() && (*objects).visible() &&
                !(*correspondence).visible() &&
                (*title).text().find("CATALOGUE GENERATION ") != std::string::npos,
            "installed Criteria route must remain a generation-labelled ordinary ObjectView");
    const std::string first_file = (*objects).items().front().name;
    std::shared_ptr<gui_forms::ComboBox> kind_value =
        std::dynamic_pointer_cast<gui_forms::ComboBox>(
            (*rack).field_editor("fm.criteria.kind", "value"));
    require(kind_value && (*kind_value).selected_text() == "Files",
            "installed Criteria probe requires the production Kind editor in Files state");
    (*kind_value).set_selected_index(1U);
    struct CatalogueFoldersReady final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        bool operator()() const {

            const bool ready =
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty() &&
                std::all_of((*objects).items().begin(), (*objects).items().end(), is_folder_item);
            return ready;
        }
    };
    require_eventually(*application, CatalogueFoldersReady{objects, application},
                       "installed Kind editor must replace file results with catalogue folders");
    const std::string first_folder = (*objects).items().front().name;
    require(first_folder != first_file,
            "installed Kind edit must materially change the rendered result set");
    std::shared_ptr<gui_forms::CheckBox> modified_enable =
        std::dynamic_pointer_cast<gui_forms::CheckBox>(
            (*window).find("fm.criteria.modified.enable"));
    require(modified_enable && (*modified_enable).checked(),
            "installed Criteria probe requires the production Modified enable control");
    const std::uint64_t generation_before_disable =
        file_manager::ApplicationInteractionProbe::search_generation(*application);
    (*modified_enable).on_activate();
    struct ModifiedDisabled final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        const std::shared_ptr<gui_forms::InstrumentRack>& rack;
        const std::shared_ptr<gui_forms::CheckBox>& modified_enable;
        const std::uint64_t& generation_before_disable;
        bool operator()() const {

            const std::vector<gui_forms::InstrumentModuleSpec>& modules = (*rack).modules();
            const gui_forms::InstrumentModuleSpec* modified = find_modified_module(modules);
            const bool ready =
                !(*modified_enable).checked() && modified != nullptr && !(*modified).enabled &&
                (*modified).status_text == "disabled · no filter emitted" &&
                file_manager::ApplicationInteractionProbe::search_generation(*application) >
                    generation_before_disable &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty();
            return ready;
        }
    };
    require_eventually(
        *application,
        ModifiedDisabled{objects, application, rack, modified_enable, generation_before_disable},
        "installed Modified enable control must reissue a valid Kind-only catalogue request");
    std::shared_ptr<gui_forms::Button> modified_remove =
        std::dynamic_pointer_cast<gui_forms::Button>((*window).find("fm.criteria.modified.remove"));
    std::shared_ptr<gui_forms::Button> add_module =
        std::dynamic_pointer_cast<gui_forms::Button>((*window).find("fm.criteria.add"));
    require(modified_remove && add_module && (*add_module).enabled(),
            "installed Criteria probe requires production remove and add controls");
    const std::uint64_t generation_before_remove =
        file_manager::ApplicationInteractionProbe::search_generation(*application);
    {
        const std::string diagnostic =
            "installed Modified remove button must accept semantic press";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.criteria.modified.remove", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct ModifiedRemoved final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        const std::shared_ptr<gui_forms::InstrumentRack>& rack;
        const std::shared_ptr<gui_forms::Button>& add_module;
        const std::uint64_t& generation_before_remove;
        bool operator()() const {

            const bool ready =
                (*rack).modules().size() == 1U &&
                (*rack).modules().front().stable_id == "fm.criteria.kind" &&
                (*add_module).enabled() &&
                file_manager::ApplicationInteractionProbe::search_generation(*application) >
                    generation_before_remove &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty();
            return ready;
        }
    };
    require_eventually(
        *application,
        ModifiedRemoved{objects, application, rack, add_module, generation_before_remove},
        "installed Modified remove control must settle a real Kind-only catalogue request");
    const std::uint64_t generation_before_add =
        file_manager::ApplicationInteractionProbe::search_generation(*application);
    {
        const std::string diagnostic = "installed add-module button must accept semantic press";
        const bool semantic_action_handled =
            (*window).perform_semantic_action("fm.criteria.add", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct ModifiedAdded final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        const std::shared_ptr<gui_forms::InstrumentRack>& rack;
        const std::uint64_t& generation_before_add;
        bool operator()() const {

            const bool ready =
                (*rack).modules().size() == 2U &&
                (*rack).modules()[1].stable_id == "fm.criteria.modified" &&
                (*rack).field_editor("fm.criteria.modified", "value") &&
                file_manager::ApplicationInteractionProbe::search_generation(*application) >
                    generation_before_add &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty();
            return ready;
        }
    };
    require_eventually(
        *application, ModifiedAdded{objects, application, rack, generation_before_add},
        "installed add-module control must restore Modified and settle its catalogue request");
    std::shared_ptr<gui_forms::TextBox> modified_value =
        std::dynamic_pointer_cast<gui_forms::TextBox>(
            (*rack).field_editor("fm.criteria.modified", "value"));
    {
        const std::string diagnostic =
            "installed Criteria probe requires the restored Modified text editor";
        require(modified_value != nullptr, diagnostic);
        const bool focus_requested = (*window).request_focus(modified_value);
        require(focus_requested, diagnostic);
    }
    (*modified_value).set_text("2026-02-30");
    {
        const std::string diagnostic = "installed Modified editor must commit through Return";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct InvalidDateRefused final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        const std::shared_ptr<gui_forms::InstrumentRack>& rack;
        bool operator()() const {

            const std::vector<gui_forms::InstrumentModuleSpec>& modules = (*rack).modules();
            const gui_forms::InstrumentModuleSpec* modified = find_modified_module(modules);
            const bool ready =
                modified != nullptr &&
                (*modified).state == gui_forms::InstrumentModuleState::invalid &&
                !(*modified).fields[2].validation_message.empty() &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                (*objects).items().empty();
            return ready;
        }
    };
    require_eventually(
        *application, InvalidDateRefused{objects, application, rack},
        "installed impossible date must fail locally and clear prior Criteria objects");
    (*modified_value).set_text("2026-01-01");
    {
        const std::string diagnostic =
            "installed Modified editor must recommit a valid date through Return";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct ValidDateRecovered final {
        const std::shared_ptr<gui_forms::ObjectView>& objects;
        const std::shared_ptr<file_manager::Application>& application;
        const std::shared_ptr<gui_forms::InstrumentRack>& rack;
        bool operator()() const {

            const std::vector<gui_forms::InstrumentModuleSpec>& modules = (*rack).modules();
            const gui_forms::InstrumentModuleSpec* modified = find_modified_module(modules);
            const bool ready =
                modified != nullptr &&
                (*modified).state == gui_forms::InstrumentModuleState::live &&
                (*modified).fields[2].validation_message.empty() &&
                !file_manager::ApplicationInteractionProbe::criteria_loading(*application) &&
                !(*objects).items().empty() &&
                std::all_of((*objects).items().begin(), (*objects).items().end(), is_folder_item);
            return ready;
        }
    };
    require_eventually(*application, ValidDateRecovered{objects, application, rack},
                       "installed valid date must recover real catalogue folder results");
    const gui_forms::ObjectViewItem selected_result = (*objects).items().front();
    (*objects).set_selected_id(selected_result.stable_id);
    const std::string expected_location =
        (std::filesystem::path(root_value) / selected_result.name).string();
    struct CatalogueInspectorReady final {
        const std::shared_ptr<gui_forms::PropertyList>& properties;
        const gui_forms::ObjectViewItem& selected_result;
        const std::string& expected_location;
        bool operator()() const {

            const bool ready = (*properties).value("fm.property.name") == selected_result.name &&
                               (*properties).value("fm.property.kind") == "Folder" &&
                               (*properties).value("fm.property.location") == expected_location;
            return ready;
        }
    };
    require_eventually(*application,
                       CatalogueInspectorReady{properties, selected_result, expected_location},
                       "installed Criteria selection must populate the factual retained inspector");
    require_unique_semantic_ids(*window);
    std::cout << "installed criteria route passed: results=" << (*objects).items().size()
              << " first_file=" << first_file << " first_folder=" << first_folder
              << " derivation=\"" << (*title).text() << "\"\n";
    (*application).stop();
}

void test_installed_daily_navigation_when_requested() {
    const char* home_value = std::getenv("FILE_MANAGER_INSTALLED_DAILY_HOME");
    const char* descendant_value = std::getenv("FILE_MANAGER_INSTALLED_DAILY_DESCENDANT");
    if (home_value == nullptr && descendant_value == nullptr)
        return;
    require(home_value != nullptr && descendant_value != nullptr,
            "installed daily probe requires both Home and descendant environment variables");

    const std::filesystem::path home = file_manager::canonical_existing_directory(home_value);
    const std::filesystem::path descendant =
        file_manager::canonical_existing_directory(descendant_value);
    require(file_manager::path_is_within(home, descendant) && descendant != home,
            "installed daily descendant must be a real directory below Home");
    const char* process_home = std::getenv("HOME");
    require(process_home != nullptr &&
                file_manager::canonical_existing_directory(process_home) == home,
            "installed daily Home must match the production process HOME");

    std::shared_ptr<file_manager::Application> application =
        std::make_shared<file_manager::Application>(home, std::nullopt, false, std::string{});
    ApplicationStopGuard stop_guard(*application);
    std::unique_ptr<gui_forms::Window> window = (*application).make_window();
    (*application).bind_host(host_noop, host_noop);
    std::shared_ptr<gui_forms::BreadcrumbTrail> breadcrumb =
        std::dynamic_pointer_cast<gui_forms::BreadcrumbTrail>((*window).find("fm.path.breadcrumb"));
    std::shared_ptr<gui_forms::TextBox> path_editor =
        std::dynamic_pointer_cast<gui_forms::TextBox>((*window).find("fm.path.editor"));
    std::shared_ptr<gui_forms::TreeView> tree =
        std::dynamic_pointer_cast<gui_forms::TreeView>((*window).find("fm.navigation.tree"));
    std::shared_ptr<gui_forms::Button> root_mode =
        std::dynamic_pointer_cast<gui_forms::Button>((*window).find("fm.navigation.root-mode"));
    std::shared_ptr<gui_forms::Command> home_command =
        file_manager::ApplicationInteractionProbe::command(*application, "go.home");
    require(breadcrumb && path_editor && tree && root_mode && home_command,
            "installed daily probe requires the production navigation surface");
    struct DailyHomeReady final {
        const std::shared_ptr<gui_forms::Button>& root_mode;
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& home;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::applied_generation(*application) > 0U &&
                file_manager::ApplicationInteractionProbe::location(*application) == home &&
                (*root_mode).text() == "Home";
            return ready;
        }
    };
    require_eventually(*application, DailyHomeReady{root_mode, application, home},
                       "installed daily startup must settle at the real user Home");
    (*window).perform_layout();

    const std::uint64_t home_generation =
        file_manager::ApplicationInteractionProbe::applied_generation(*application);
    {
        const std::string diagnostic =
            "installed daily exact-path editor must open through its retained actuator";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            (*breadcrumb).edit_stable_id(), gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        require((*breadcrumb).editing(), diagnostic);
    }
    (*path_editor).set_text(descendant.string());
    {
        const std::string diagnostic = "installed daily descendant must commit through Return";
        const bool key_handled =
            (*window).dispatch_key({gui_forms::KeyAction::down, gui_forms::PhysicalKey::enter});
        require(key_handled, diagnostic);
    }
    struct DailyDescendantReady final {
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& descendant;
        const std::uint64_t& home_generation;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::applied_generation(*application) >
                    home_generation &&
                file_manager::ApplicationInteractionProbe::location(*application) == descendant;
            return ready;
        }
    };
    require_eventually(*application, DailyDescendantReady{application, descendant, home_generation},
                       "installed daily direct path must navigate to the real Home descendant");

    {
        const std::string diagnostic =
            "installed daily Back control must be enabled after real navigation";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.location.navigation.back", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct DailyBackReady final {
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& home;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::location(*application) == home;
            return ready;
        }
    };
    require_eventually(*application, DailyBackReady{application, home},
                       "installed daily Back must restore real Home");
    {
        const std::string diagnostic = "installed daily Forward control must be enabled after Back";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "file-manager-app.shell.location.navigation.forward", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
    }
    struct DailyForwardReady final {
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& descendant;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::location(*application) == descendant;
            return ready;
        }
    };
    require_eventually(*application, DailyForwardReady{application, descendant},
                       "installed daily Forward must restore the real descendant");
    {
        const std::string diagnostic =
            "installed daily Home command must remain available from a descendant";
        const bool command_executed = (*home_command).execute("installed.daily.home");
        require(command_executed, diagnostic);
    }
    struct DailyHomeCommandReady final {
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& home;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::location(*application) == home;
            return ready;
        }
    };
    require_eventually(*application, DailyHomeCommandReady{application, home},
                       "installed daily Home command must return to the real user Home");

    {
        const std::string diagnostic = "installed daily root-mode menu must expose real Volumes";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.navigation.root-mode", gui_forms::SemanticAction::press);
        require(semantic_action_handled, diagnostic);
        const bool semantic_action_handled_2 = (*window).perform_semantic_action(
            "fm.navigation.root-mode-menu.popup.row.volumes", gui_forms::SemanticAction::press);
        require(semantic_action_handled_2, diagnostic);
    }
    struct VolumesTreeReady final {
        const std::shared_ptr<gui_forms::TreeView>& tree;
        const std::shared_ptr<gui_forms::Button>& root_mode;
        bool operator()() const {

            const std::optional<gui_forms::TreeViewItem> volumes = tree_item(*tree, "Volumes");
            const bool ready =
                volumes && (*volumes).depth == 0U && (*root_mode).text() == "Volumes";
            return ready;
        }
    };
    require_eventually(*application, VolumesTreeReady{tree, root_mode},
                       "installed daily Volumes mode must retain the real root row");
    {
        const std::string diagnostic =
            "installed daily Volumes tree root must accept one-click selection";
        const bool semantic_action_handled = (*window).perform_semantic_action(
            "fm.location.volumes", gui_forms::SemanticAction::select);
        require(semantic_action_handled, diagnostic);
    }
    struct VolumesLocationReady final {
        const std::shared_ptr<file_manager::Application>& application;
        bool operator()() const {

            const bool ready = file_manager::ApplicationInteractionProbe::location(*application) ==
                               std::filesystem::path("/Volumes");
            return ready;
        }
    };
    require_eventually(*application, VolumesLocationReady{application},
                       "installed daily Volumes selection must navigate the real mount root");
    {
        const std::string diagnostic =
            "installed daily Home command must remain available from Volumes";
        const bool command_executed = (*home_command).execute("installed.daily.return-home");
        require(command_executed, diagnostic);
    }
    struct DailyReturnHomeReady final {
        const std::shared_ptr<file_manager::Application>& application;
        const std::filesystem::path& home;
        bool operator()() const {

            const bool ready =
                file_manager::ApplicationInteractionProbe::location(*application) == home;
            return ready;
        }
    };
    require_eventually(*application, DailyReturnHomeReady{application, home},
                       "installed daily Home command must return from Volumes");
    require_unique_semantic_ids(*window);
    std::cout << "installed daily Home/descendant/Volumes navigation passed\n";
    (*application).stop();
}

} // namespace

int main() {
    try {
        test_stop_during_ui_drain_revokes_remaining_callbacks();
        test_application_controls_navigate_real_directories();
        test_selected_file_previews_reach_visible_layout();
        test_application_command_surfaces_and_house_mark();
        test_application_command_truth_across_files_search_and_settings();
        test_criteria_virtual_folder_is_retained_exact_and_catalogue_only();
        test_settings_tabs_and_transaction_actions_are_truthful();
        test_property_name_rename_is_protected_and_collision_safe();
        test_adaptive_layout_preserves_fields_commands_and_selection();
        test_installed_criteria_route_when_requested();
        test_installed_daily_navigation_when_requested();
        std::cout << "file manager application interaction tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "file manager application interaction tests failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
