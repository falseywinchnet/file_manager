#include "file_manager/platform_paths.hpp"
#include "file_manager/document_picker_view.hpp"

#include <algorithm>
#include <utility>

namespace file_manager {
namespace {

// The view owns the referenced flag. Nested updates restore their caller's
// state; exceptions unwind the scope before the host can retry the operation.
class PickerUpdateScope final {
  public:
    explicit PickerUpdateScope(bool& updating) noexcept : updating_(updating), previous_(updating) {
        updating_ = true;
    }
    ~PickerUpdateScope() {
        updating_ = previous_;
    }
    PickerUpdateScope(const PickerUpdateScope&) = delete;
    PickerUpdateScope& operator=(const PickerUpdateScope&) = delete;

  private:
    bool& updating_;
    const bool previous_;
};

// Keep controls at usable text/input sizes while the object field absorbs resize.
class PickerPanel final : public gui_forms::ScaledPanel {
  public:
    explicit PickerPanel(gui_forms::StableId id) : ScaledPanel(std::move(id)) {}
    void arrange(gui_forms::Rect bounds) override {
        arrange_self(bounds);
        const double width = std::max(540.0, bounds.width);
        const double height = std::max(400.0, bounds.height);
        const bool compact = width < 700.0;
        const double fields = height - (compact ? 125.0 : 83.0);
        for (const gui_forms::Control::Ptr& child : children()) {
            const std::optional<gui_forms::Rect> slot = design_bounds(*child);
            if (!slot)
                continue;
            gui_forms::Rect r = *slot;
            if (r.y == 1 || r.y == 41)
                r.width = width - 2;
            else if (r.y == 50 && r.x == 156)
                r.width = width - 168;
            else if (r.y == 101 && r.x == 244)
                r.width = width - 256;
            else if (r.y == 143) {
                r.width = width - 24;
                r.height = fields - 153;
            } else if (r.y == 477) {
                r.y = fields;
                if (r.x == 386) {
                    r.x = compact ? 12 : 386;
                    r.y += compact ? 42 : 0;
                    r.width = width - r.x - 12;
                }
            } else if (r.y >= 517) {
                r.y = height - (r.y == 519 ? 41 : 43);
                if (r.x == 12)
                    r.width = width - 202;
                else
                    r.x = width - (r.x == 584 ? 176 : 90);
            }
            if (r.y == 12 && r.x == 272)
                r.width = std::max(0.0, width - 284);
            set_child_layout(child, r);
        }
    }
    void on_key_preview(gui_forms::KeyEvent& event) override {
        if (event.action != gui_forms::KeyAction::down || !attached_window())
            return;
        gui_forms::Window& window = *attached_window();
        if (event.physical_key == gui_forms::PhysicalKey::l &&
            gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::control)) {
            window.request_focus(window.find("file-manager.picker.path"));
            event.handled = true;
        } else if (event.physical_key == gui_forms::PhysicalKey::up &&
                   gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::alt)) {
            const std::shared_ptr<gui_forms::Button> button =
                std::dynamic_pointer_cast<gui_forms::Button>(window.find("file-manager.picker.up"));
            if (button)
                event.handled = (*button).perform_click();
        }
    }
};

gui_forms::SurfaceMaterial watercolor_title_material() {
    using gui_forms::Color;
    using gui_forms::GradientStop;
    using gui_forms::MaterialFillLayer;
    gui_forms::SurfaceMaterial material{};
    material.fills = {
        MaterialFillLayer::linear_css_angle(92.0,
                                            {
                                                GradientStop{0.0, Color::rgba(23, 52, 127)},
                                                GradientStop{0.55, Color::rgba(58, 104, 203)},
                                                GradientStop{1.0, Color::rgba(217, 104, 114)},
                                            }),
        MaterialFillLayer::radial({0.18, -0.90}, {0.42, 1.35},
                                  {
                                      GradientStop{0.0, Color::rgba(146, 217, 255, 116)},
                                      GradientStop{1.0, Color::rgba(146, 217, 255, 0)},
                                  }),
        MaterialFillLayer::radial({0.62, 1.60}, {0.38, 1.10},
                                  {
                                      GradientStop{0.0, Color::rgba(214, 178, 255, 106)},
                                      GradientStop{1.0, Color::rgba(214, 178, 255, 0)},
                                  }),
        MaterialFillLayer::radial({0.95, 1.0}, {0.28, 0.80},
                                  {
                                      GradientStop{0.0, Color::rgba(255, 195, 142, 100)},
                                      GradientStop{1.0, Color::rgba(255, 195, 142, 0)},
                                  }),
    };
    material.border_edges.bottom = gui_forms::MaterialBorder{Color::rgba(23, 45, 105), 1.0};
    return material;
}

gui_forms::SurfaceMaterial graphite_navigation_material() {
    using gui_forms::Color;
    using gui_forms::GradientStop;
    gui_forms::SurfaceMaterial material{};
    material.fills = {gui_forms::MaterialFillLayer::linear_css_angle(
        180.0, {
                   GradientStop{0.0, Color::rgba(98, 108, 115)},
                   GradientStop{0.52, Color::rgba(75, 84, 90)},
                   GradientStop{1.0, Color::rgba(66, 74, 79)},
               })};
    material.border_edges.bottom = gui_forms::MaterialBorder{Color::rgba(32, 45, 53), 1.0};
    return material;
}

gui_forms::ObjectGlyph glyph(const DirectoryEntry& entry) {
    if (entry.directory)
        return gui_forms::ObjectGlyph::folder;
    switch (entry.kind) {
    case EntryKind::image:
        return gui_forms::ObjectGlyph::image;
    case EntryKind::archive:
        return gui_forms::ObjectGlyph::archive;
    case EntryKind::audio:
        return gui_forms::ObjectGlyph::audio;
    case EntryKind::code:
        return gui_forms::ObjectGlyph::code;
    case EntryKind::folder:
        return gui_forms::ObjectGlyph::folder;
    case EntryKind::document:
    case EntryKind::symlink:
    case EntryKind::other:
        return gui_forms::ObjectGlyph::document;
    }
    return gui_forms::ObjectGlyph::document;
}

bool save_profile(const DocumentPickerProfile profile) {
    const bool saves =
        profile == DocumentPickerProfile::save_as || profile == DocumentPickerProfile::export_file;
    return saves;
}

bool multiple_profile(const DocumentPickerProfile profile) {
    const bool multiple = profile == DocumentPickerProfile::open_files ||
                          profile == DocumentPickerProfile::import_files;
    return multiple;
}

std::string accept_title(const DocumentPickerProfile profile) {
    switch (profile) {
    case DocumentPickerProfile::open_file:
    case DocumentPickerProfile::open_files:
        return "Open";
    case DocumentPickerProfile::select_folder:
        return "Select";
    case DocumentPickerProfile::save_as:
        return "Save";
    case DocumentPickerProfile::import_files:
        return "Import";
    case DocumentPickerProfile::export_file:
        return "Export";
    }
    return "Accept";
}

} // namespace

DocumentPickerView::DocumentPickerView(DocumentPickerRequest request)
    : controller_(std::move(request)),
      root_(std::make_shared<PickerPanel>(gui_forms::StableId("file-manager.picker"))),
      title_bar_(
          std::make_shared<gui_forms::Control>(gui_forms::StableId("file-manager.picker.title"))),
      title_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.picker.title.name"), "Open a file")),
      subtitle_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.picker.title.subtitle"),
          "local filesystem · bounded selection authority")),
      navigation_bar_(std::make_shared<gui_forms::Control>(
          gui_forms::StableId("file-manager.picker.navigation"))),
      back_to_root_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.picker.root"), "Home")),
      up_(std::make_shared<gui_forms::Button>(gui_forms::StableId("file-manager.picker.up"), "Up")),
      path_(std::make_shared<gui_forms::TextBox>(gui_forms::StableId("file-manager.picker.path"))),
      locations_(std::make_shared<gui_forms::ComboBox>(
          gui_forms::StableId("file-manager.picker.locations"))),
      name_filter_(std::make_shared<gui_forms::TextBox>(
          gui_forms::StableId("file-manager.picker.name-filter"))),
      objects_(std::make_shared<gui_forms::ObjectView>(
          gui_forms::StableId("file-manager.picker.objects"))),
      filter_(
          std::make_shared<gui_forms::ComboBox>(gui_forms::StableId("file-manager.picker.filter"))),
      hidden_(std::make_shared<gui_forms::CheckBox>(
          gui_forms::StableId("file-manager.picker.hidden"), "Show hidden")),
      filename_(std::make_shared<gui_forms::TextBox>(
          gui_forms::StableId("file-manager.picker.filename"))),
      status_(
          std::make_shared<gui_forms::Label>(gui_forms::StableId("file-manager.picker.status"))),
      accept_(std::make_shared<gui_forms::Button>(gui_forms::StableId("file-manager.picker.accept"),
                                                  accept_title(controller_.request().profile))),
      cancel_(std::make_shared<gui_forms::Button>(gui_forms::StableId("file-manager.picker.cancel"),
                                                  "Cancel")) {
    (*root_).set_requested_bounds({0, 0, 760, 560});
    (*root_).set_design_size({760, 560});
    (*root_).set_border_style(gui_forms::BorderStyle::line);
    (*root_).set_background(gui_forms::Color::rgba(231, 237, 246));

    (*title_bar_).set_authored_surface_material(watercolor_title_material());
    (*title_bar_).set_accessible_name("Open dialog title bar");
    (*title_).set_text(accept_title(controller_.request().profile) + " a " +
                       (controller_.request().profile == DocumentPickerProfile::select_folder
                            ? "folder"
                            : "file"));
    (*title_).set_font({gui_forms::FontRole::control, 15.0, 700, false});
    (*title_).set_foreground(gui_forms::Color::rgba(255, 255, 255));
    (*title_).set_hit_test_transparent(true);
    (*subtitle_).set_font({gui_forms::FontRole::control, 10.0, 400, false});
    (*subtitle_).set_foreground(gui_forms::Color::rgba(225, 237, 255));
    (*subtitle_).set_hit_test_transparent(true);
    (*navigation_bar_).set_authored_surface_material(graphite_navigation_material());

    (*back_to_root_).set_accessible_name("Go to Home");
    (*up_).set_accessible_name("Go to parent folder");
    (*path_).set_accessible_name("Picker current location");
    (*path_).set_placeholder_text("Location inside the selection root");
    (*objects_).set_accessible_name("Picker objects");
    (*objects_).set_view_mode(gui_forms::ObjectViewMode::details);
    (*objects_).set_selection_mode(multiple_profile(controller_.request().profile)
                                       ? gui_forms::ObjectSelectionMode::multiple
                                       : gui_forms::ObjectSelectionMode::single);
    (*filter_).set_accessible_name("File type filter");
    (*hidden_).set_checked(controller_.show_hidden());
    (*hidden_).set_enabled(controller_.request().allow_hidden_toggle);
    (*filename_).set_accessible_name("File name");
    (*filename_).set_placeholder_text("File name");
    (*filename_).set_maximum_length(255);
    (*filename_).set_visible(save_profile(controller_.request().profile));
    (*accept_).set_accessible_description(
        "Acceptance revalidates filesystem identity and Orchestrator session");

    (*root_).add_at(title_bar_, {1, 1, 758, 40});
    (*root_).add_at(title_, {76, 9, 190, 23});
    (*root_).add_at(subtitle_, {272, 12, 390, 18});
    (*root_).add_at(navigation_bar_, {1, 41, 758, 50});
    (*root_).add_at(back_to_root_, {12, 50, 70, 32});
    (*root_).add_at(up_, {90, 50, 58, 32});
    (*root_).add_at(path_, {156, 50, 592, 32});
    (*locations_).set_accessible_name("Available selection roots");
    (*name_filter_).set_accessible_name("Filename filter");
    (*name_filter_).set_placeholder_text("Filter names (for example *.txt)");
    (*root_).add_at(locations_, {12, 101, 220, 32});
    (*root_).add_at(name_filter_, {244, 101, 504, 32});
    (*root_).add_at(objects_, {12, 143, 736, 324});
    (*root_).add_at(filter_, {12, 477, 220, 32});
    (*root_).add_at(hidden_, {244, 477, 130, 32});
    (*root_).add_at(filename_, {386, 477, 362, 32});
    (*root_).add_at(status_, {12, 519, 470, 30});
    (*root_).add_at(cancel_, {584, 517, 78, 34});
    (*root_).add_at(accept_, {670, 517, 78, 34});

    populate_options();
    connect_callbacks();

    reload();
}

// Delegates borrow this view. Tokens own connections and are revoked before any
// control/controller member is destroyed. The host retains the view throughout
// a control dispatch. Completion publication is the final access to the view,
// permitting terminal listeners to dispose it; callers must not use that view
// afterward. GUI.Forms retains event state for the active emission.
DocumentPickerView::~DocumentPickerView() {
    for (gui_forms::SubscriptionToken& subscription : subscriptions_)
        subscription.disconnect();
    subscriptions_.clear();
}

void DocumentPickerView::populate_options() {
    // Request option arrays are immutable for this view. GUI.Forms takes ownership
    // of the prepared vectors; do not reconstruct them on each directory refresh.
    const DocumentPickerRequest& request = controller_.request();
    std::vector<std::string> locations{};
    locations.reserve(request.admitted_roots.size());
    for (const std::filesystem::path& root : request.admitted_roots) {
        const std::string label = path_utf8(root);
        locations.push_back(label);
    }
    (*locations_).set_items(std::move(locations));
    std::vector<std::string> filters{};
    filters.reserve(request.filters.size());
    for (const DocumentTypeFilter& filter : request.filters)
        filters.push_back(filter.label);
    (*filter_).set_items(std::move(filters));
    (*filter_).set_visible(!request.filters.empty());
}

void DocumentPickerView::connect_callbacks() {
    subscriptions_.reserve(14U);
    {
        const gui_forms::Delegate<std::optional<std::size_t>> callback =
            gui_forms::Delegate<std::optional<std::size_t>>::bind<
                DocumentPickerView, &DocumentPickerView::location_changed>(*this);
        gui_forms::Event<std::optional<std::size_t>>& event =
            (*locations_).selected_index_changed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const std::string&> callback =
            gui_forms::Delegate<const std::string&>::bind<
                DocumentPickerView, &DocumentPickerView::name_filter_committed>(*this);
        gui_forms::Event<const std::string&>& event = (*name_filter_).committed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<gui_forms::ButtonBase&> callback =
            gui_forms::Delegate<gui_forms::ButtonBase&>::bind<DocumentPickerView,
                                                              &DocumentPickerView::home_clicked>(
                *this);
        gui_forms::Event<gui_forms::ButtonBase&>& event = (*back_to_root_).clicked();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<gui_forms::ButtonBase&> callback =
            gui_forms::Delegate<gui_forms::ButtonBase&>::bind<DocumentPickerView,
                                                              &DocumentPickerView::up_clicked>(
                *this);
        gui_forms::Event<gui_forms::ButtonBase&>& event = (*up_).clicked();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const std::string&> callback =
            gui_forms::Delegate<const std::string&>::bind<DocumentPickerView,
                                                          &DocumentPickerView::path_committed>(
                *this);
        gui_forms::Event<const std::string&>& event = (*path_).committed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<> callback =
            gui_forms::Delegate<>::bind<DocumentPickerView, &DocumentPickerView::path_cancelled>(
                *this);
        gui_forms::Event<>& event = (*path_).cancelled();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const gui_forms::ObjectSelectionChange&> callback =
            gui_forms::Delegate<const gui_forms::ObjectSelectionChange&>::bind<
                DocumentPickerView, &DocumentPickerView::selection_changed>(*this);
        gui_forms::Event<const gui_forms::ObjectSelectionChange&>& event =
            (*objects_).selection_changed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const std::string&> callback =
            gui_forms::Delegate<const std::string&>::bind<DocumentPickerView,
                                                          &DocumentPickerView::item_activated>(
                *this);
        gui_forms::Event<const std::string&>& event = (*objects_).item_activated();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<std::optional<std::size_t>> callback =
            gui_forms::Delegate<std::optional<std::size_t>>::bind<
                DocumentPickerView, &DocumentPickerView::filter_changed>(*this);
        gui_forms::Event<std::optional<std::size_t>>& event = (*filter_).selected_index_changed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<bool> callback =
            gui_forms::Delegate<bool>::bind<DocumentPickerView,
                                            &DocumentPickerView::hidden_changed>(*this);
        gui_forms::Event<bool>& event = (*hidden_).checked_changed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const std::string&> callback =
            gui_forms::Delegate<const std::string&>::bind<DocumentPickerView,
                                                          &DocumentPickerView::filename_changed>(
                *this);
        gui_forms::Event<const std::string&>& event = (*filename_).text_changed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<const std::string&> callback =
            gui_forms::Delegate<const std::string&>::bind<DocumentPickerView,
                                                          &DocumentPickerView::filename_committed>(
                *this);
        gui_forms::Event<const std::string&>& event = (*filename_).committed();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<gui_forms::ButtonBase&> callback =
            gui_forms::Delegate<gui_forms::ButtonBase&>::bind<DocumentPickerView,
                                                              &DocumentPickerView::accept_clicked>(
                *this);
        gui_forms::Event<gui_forms::ButtonBase&>& event = (*accept_).clicked();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
    {
        const gui_forms::Delegate<gui_forms::ButtonBase&> callback =
            gui_forms::Delegate<gui_forms::ButtonBase&>::bind<DocumentPickerView,
                                                              &DocumentPickerView::cancel_clicked>(
                *this);
        gui_forms::Event<gui_forms::ButtonBase&>& event = (*cancel_).clicked();
        gui_forms::SubscriptionToken subscription = event.subscribe(callback);
        subscriptions_.push_back(std::move(subscription));
    }
}

void DocumentPickerView::location_changed(const std::optional<std::size_t> index) {
    if (!reloading_ && index && *index < controller_.request().admitted_roots.size() &&
        controller_.request().admitted_roots[*index] != controller_.request().protected_root)
        navigate_path(controller_.request().admitted_roots[*index]);
}

void DocumentPickerView::name_filter_committed(const std::string& value) {
    if (controller_.set_name_filter(value))
        reload();
    else
        (*status_).set_text(controller_.last_error());
}

void DocumentPickerView::home_clicked(gui_forms::ButtonBase&) {
    navigate_path(controller_.request().home_location);
}

void DocumentPickerView::up_clicked(gui_forms::ButtonBase&) {
    const std::filesystem::path& location = controller_.browser().location;
    if (location != controller_.request().protected_root) {
        const std::filesystem::path parent = location.parent_path();
        navigate_path(parent);
    }
}

void DocumentPickerView::path_committed(const std::string& value) {
    const std::filesystem::path location = path_from_utf8(value);
    navigate_path(location);
}

void DocumentPickerView::path_cancelled() {
    const std::string location_text = path_utf8(controller_.browser().location);
    (*path_).set_text(location_text);
}

void DocumentPickerView::selection_changed(const gui_forms::ObjectSelectionChange&) {
    if (reloading_)
        return;
    {
        PickerUpdateScope update(reloading_);
        std::vector<std::string> ids((*objects_).selected_ids().begin(),
                                     (*objects_).selected_ids().end());
        if (!controller_.set_selection(std::move(ids))) {
            (*objects_).clear_selection();
            (*status_).set_text("Selection is outside this picker profile");
        } else {
            (*filename_).set_text(controller_.filename());
        }
    }
    publish_pending_completion();
}

void DocumentPickerView::item_activated(const std::string& stable_id) {
    const std::vector<DirectoryEntry>& entries = controller_.browser().entries;
    for (const DirectoryEntry& entry : entries) {
        if (entry.stable_id != stable_id)
            continue;
        if (entry.directory) {
            const std::filesystem::path destination = entry.path;
            navigate_path(destination);
        } else if (!save_profile(controller_.request().profile)) {
            accept();
        }
        return;
    }
}

void DocumentPickerView::filter_changed(const std::optional<std::size_t> index) {
    if (reloading_)
        return;
    if (!index || *index >= controller_.request().filters.size())
        return;
    if (controller_.request().filters[*index].id == controller_.active_filter_id())
        return;
    if (controller_.set_active_filter(controller_.request().filters[*index].id)) {
        reload();
    }
}

void DocumentPickerView::hidden_changed(const bool checked) {
    if (reloading_)
        return;
    if (checked == controller_.show_hidden())
        return;
    if (controller_.set_show_hidden(checked))
        reload();
}

void DocumentPickerView::filename_changed(const std::string& value) {
    (void)controller_.set_filename(value);
}

void DocumentPickerView::filename_committed(const std::string&) {
    accept();
}

void DocumentPickerView::accept_clicked(gui_forms::ButtonBase&) {
    accept();
}

void DocumentPickerView::cancel_clicked(gui_forms::ButtonBase&) {
    DocumentPickerResult result = controller_.cancel();
    publish(std::move(result));
}

std::shared_ptr<gui_forms::Control> DocumentPickerView::root_control() const {
    const std::shared_ptr<gui_forms::Control> root = root_;
    return root;
}

FileSelectionController& DocumentPickerView::controller() noexcept {
    return controller_;
}

const FileSelectionController& DocumentPickerView::controller() const noexcept {
    return controller_;
}

gui_forms::Event<const DocumentPickerResult&>& DocumentPickerView::completed() noexcept {
    return completed_;
}

void DocumentPickerView::set_orchestrator_session_valid(const bool valid) {
    controller_.set_orchestrator_session_valid(valid);
    update_status();
}

void DocumentPickerView::set_authority_valid(const bool valid) {
    controller_.set_authority_valid(valid);
    update_status();
}

void DocumentPickerView::cancel() {
    DocumentPickerResult result = controller_.cancel();
    publish(std::move(result));
}

void DocumentPickerView::confirm_overwrite() {
    accept(true);
}

void DocumentPickerView::present(const std::filesystem::path& initial_location) {
    {
        PickerUpdateScope update(reloading_);
        finished_ = false;
        navigate_path(initial_location);
        (*objects_).clear_selection();
        gui_forms::Window* window = (*root_).attached_window();
        if (window != nullptr)
            attach_dialog(*window);
    }
    publish_pending_completion();
}

void DocumentPickerView::attach_dialog(gui_forms::Window& window) {
    window.set_accept_button(accept_);
    window.set_cancel_button(cancel_);
    window.request_focus(save_profile(controller_.request().profile)
                             ? std::static_pointer_cast<gui_forms::Control>(filename_)
                             : std::static_pointer_cast<gui_forms::Control>(objects_));
}

void DocumentPickerView::update_status() {
    {
        PickerUpdateScope update(reloading_);
        const bool valid = controller_.session_valid();
        const bool enabled = valid && !finished_;
        (*accept_).set_enabled(enabled);
        std::string text = "Selection session unavailable - browsing only";
        if (valid) {
            text = std::to_string(controller_.browser().entries.size());
            text += " visible objects";
            if (controller_.request().authority == DocumentPickerAuthority::trusted_local_host) {
                text += " - local host selection";
            }
        }
        (*status_).set_text(text);
    }
    publish_pending_completion();
}

void DocumentPickerView::reload() {
    {
        PickerUpdateScope update(reloading_);
        const std::string location_text = path_utf8(controller_.browser().location);
        (*path_).set_text(location_text);
        const DocumentPickerRequest& request = controller_.request();
        std::size_t current_root = 0;
        for (std::size_t index = 0; index < request.admitted_roots.size(); ++index) {
            if (request.admitted_roots[index] == request.protected_root)
                current_root = index;
        }
        (*locations_).set_selected_index(current_root);
        const std::optional<NavigationTarget> home =
            resolve_navigation_target(request.admitted_roots, controller_.browser().location,
                                      request.home_location, request.home_location);
        (*back_to_root_).set_enabled(home.has_value());
        (*up_).set_enabled(controller_.browser().location != controller_.request().protected_root);
        std::vector<gui_forms::ObjectViewItem> items{};
        items.reserve(controller_.browser().entries.size());
        for (const DirectoryEntry& entry : controller_.browser().entries) {
            gui_forms::ObjectViewItem item{
                .stable_id = entry.stable_id,
                .name = entry.name,
                .secondary_text = entry.secondary_text,
                .description = entry.directory ? "Folder" : "File",
                .glyph = glyph(entry),
                .enabled = entry.kind != EntryKind::symlink,
                .image_key = {},
            };
            items.push_back(std::move(item));
        }
        (*objects_).set_items(std::move(items));
        (*objects_).clear_selection();
        (void)controller_.set_selection({});
        (*filename_).set_text(controller_.filename());
        std::optional<std::size_t> selected_filter{};
        for (std::size_t index = 0; index < controller_.request().filters.size(); ++index) {
            const DocumentTypeFilter& value = controller_.request().filters[index];
            if (value.id == controller_.active_filter_id())
                selected_filter = index;
        }
        if (selected_filter)
            (*filter_).set_selected_index(*selected_filter);
        (*hidden_).set_checked(controller_.show_hidden());
        update_status();
    }
    publish_pending_completion();
}

void DocumentPickerView::navigate_path(std::filesystem::path path) {
    {
        PickerUpdateScope update(reloading_);
        if (controller_.navigate(path)) {
            reload();
        } else {
            const std::string location_text = path_utf8(controller_.browser().location);
            (*path_).set_text(location_text);
            (*status_).set_text("Location unavailable · " + controller_.last_error());
        }
    }
    publish_pending_completion();
}

void DocumentPickerView::accept(const bool overwrite_confirmed) {
    if (finished_ || reloading_)
        return;
    if (controller_.selected_ids().size() == 1 &&
        controller_.request().profile != DocumentPickerProfile::select_folder) {
        const std::string id = controller_.selected_ids().front();
        for (const DirectoryEntry& entry : controller_.browser().entries) {
            if (entry.stable_id == id && entry.directory) {
                navigate_path(entry.path);
                return;
            }
        }
    }
    if (save_profile(controller_.request().profile)) {
        const std::string filename((*filename_).text());
        const bool filename_valid = controller_.set_filename(filename);
        if (!filename_valid) {
            (*status_).set_text("Filename exceeds the supported byte limit");
            return;
        }
    }
    DocumentPickerResult result = controller_.accept(overwrite_confirmed);
    publish(std::move(result));
}

void DocumentPickerView::publish_pending_completion() {
    if (reloading_ || !pending_completion_)
        return;
    DocumentPickerResult result = std::move(*pending_completion_);
    pending_completion_.reset();
    // Last operation: completion may destroy the view.
    publish(std::move(result));
}

void DocumentPickerView::publish(DocumentPickerResult result) {
    if (finished_)
        return;
    if (reloading_) {
        // Only cancellation can reach here while controls are being synchronized;
        // accept is refused until the snapshot and control selection agree.
        // Preserve the first completion and defer notification until outermost exit.
        if (!pending_completion_)
            pending_completion_ = std::move(result);
        controller_.set_authority_valid(false);
        return;
    }
    {
        PickerUpdateScope update(reloading_);
        pending_completion_.reset();
        if (result.accepted() || result.terminal == DocumentPickerTerminal::cancelled) {
            finished_ = true;
            controller_.set_authority_valid(false);
            (*accept_).set_enabled(false);
        }
        if (result.terminal == DocumentPickerTerminal::validation_error ||
            result.terminal == DocumentPickerTerminal::unavailable) {
            (*status_).set_text(result.message);
        } else if (result.terminal == DocumentPickerTerminal::overwrite_confirmation_required) {
            (*status_).set_text("Overwrite confirmation required");
        }
    }
    if (pending_completion_) {
        publish_pending_completion();
        return;
    }
    // No access to this view after emission: a terminal host may destroy it.
    completed_.emit(result);
}

} // namespace file_manager
