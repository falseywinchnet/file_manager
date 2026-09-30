#include "file_manager/platform_paths.hpp"
#include "file_manager/document_picker_view.hpp"

#include <algorithm>
#include <utility>

namespace file_manager {
namespace {

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
        for (const auto& child : children()) {
            auto slot = design_bounds(*child);
            if (!slot) continue;
            auto r = *slot;
            if (r.y == 1 || r.y == 41) r.width = width - 2;
            else if (r.y == 50 && r.x == 156) r.width = width - 168;
            else if (r.y == 101 && r.x == 244) r.width = width - 256;
            else if (r.y == 143) { r.width = width - 24; r.height = fields - 153; }
            else if (r.y == 477) {
                r.y = fields;
                if (r.x == 386) {
                    r.x = compact ? 12 : 386;
                    r.y += compact ? 42 : 0;
                    r.width = width - r.x - 12;
                }
            } else if (r.y >= 517) {
                r.y = height - (r.y == 519 ? 41 : 43);
                if (r.x == 12) r.width = width - 202;
                else r.x = width - (r.x == 584 ? 176 : 90);
            }
            if (r.y == 12 && r.x == 272) r.width = std::max(0.0, width - 284);
            set_child_layout(child, r);
        }
    }
    void on_key_preview(gui_forms::KeyEvent& event) override {
        if (event.action != gui_forms::KeyAction::down || !attached_window()) return;
        auto& window = *attached_window();
        if (event.physical_key == gui_forms::PhysicalKey::l &&
            gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::control)) {
            window.request_focus(window.find("file-manager.picker.path"));
            event.handled = true;
        } else if (event.physical_key == gui_forms::PhysicalKey::up &&
                   gui_forms::has_modifier(event.modifiers, gui_forms::Modifier::alt)) {
            auto button = std::dynamic_pointer_cast<gui_forms::Button>(
                window.find("file-manager.picker.up"));
            if (button) event.handled = button->perform_click();
        }
    }
};

gui_forms::SurfaceMaterial watercolor_title_material() {
    using gui_forms::Color;
    using gui_forms::GradientStop;
    using gui_forms::MaterialFillLayer;
    gui_forms::SurfaceMaterial material;
    material.fills = {
        MaterialFillLayer::linear_css_angle(92.0, {
            GradientStop{0.0, Color::rgba(23, 52, 127)},
            GradientStop{0.55, Color::rgba(58, 104, 203)},
            GradientStop{1.0, Color::rgba(217, 104, 114)},
        }),
        MaterialFillLayer::radial({0.18, -0.90}, {0.42, 1.35}, {
            GradientStop{0.0, Color::rgba(146, 217, 255, 116)},
            GradientStop{1.0, Color::rgba(146, 217, 255, 0)},
        }),
        MaterialFillLayer::radial({0.62, 1.60}, {0.38, 1.10}, {
            GradientStop{0.0, Color::rgba(214, 178, 255, 106)},
            GradientStop{1.0, Color::rgba(214, 178, 255, 0)},
        }),
        MaterialFillLayer::radial({0.95, 1.0}, {0.28, 0.80}, {
            GradientStop{0.0, Color::rgba(255, 195, 142, 100)},
            GradientStop{1.0, Color::rgba(255, 195, 142, 0)},
        }),
    };
    material.border_edges.bottom =
        gui_forms::MaterialBorder{Color::rgba(23, 45, 105), 1.0};
    return material;
}

gui_forms::SurfaceMaterial graphite_navigation_material() {
    using gui_forms::Color;
    using gui_forms::GradientStop;
    gui_forms::SurfaceMaterial material;
    material.fills = {gui_forms::MaterialFillLayer::linear_css_angle(180.0, {
        GradientStop{0.0, Color::rgba(98, 108, 115)},
        GradientStop{0.52, Color::rgba(75, 84, 90)},
        GradientStop{1.0, Color::rgba(66, 74, 79)},
    })};
    material.border_edges.bottom =
        gui_forms::MaterialBorder{Color::rgba(32, 45, 53), 1.0};
    return material;
}

gui_forms::ObjectGlyph glyph(const DirectoryEntry& entry) {
    if (entry.directory) return gui_forms::ObjectGlyph::folder;
    switch (entry.kind) {
        case EntryKind::image: return gui_forms::ObjectGlyph::image;
        case EntryKind::archive: return gui_forms::ObjectGlyph::archive;
        case EntryKind::audio: return gui_forms::ObjectGlyph::audio;
        case EntryKind::code: return gui_forms::ObjectGlyph::code;
        case EntryKind::folder: return gui_forms::ObjectGlyph::folder;
        case EntryKind::document:
        case EntryKind::symlink:
        case EntryKind::other:
            return gui_forms::ObjectGlyph::document;
    }
    return gui_forms::ObjectGlyph::document;
}

bool save_profile(const DocumentPickerProfile profile) {
    return profile == DocumentPickerProfile::save_as ||
        profile == DocumentPickerProfile::export_file;
}

bool multiple_profile(const DocumentPickerProfile profile) {
    return profile == DocumentPickerProfile::open_files ||
        profile == DocumentPickerProfile::import_files;
}

std::string accept_title(const DocumentPickerProfile profile) {
    switch (profile) {
        case DocumentPickerProfile::open_file:
        case DocumentPickerProfile::open_files: return "Open";
        case DocumentPickerProfile::select_folder: return "Select";
        case DocumentPickerProfile::save_as: return "Save";
        case DocumentPickerProfile::import_files: return "Import";
        case DocumentPickerProfile::export_file: return "Export";
    }
    return "Accept";
}

} // namespace

DocumentPickerView::DocumentPickerView(DocumentPickerRequest request)
    : controller_(std::move(request)),
      root_(std::make_shared<PickerPanel>(
          gui_forms::StableId("file-manager.picker"))),
      title_bar_(std::make_shared<gui_forms::Control>(
          gui_forms::StableId("file-manager.picker.title"))),
      title_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.picker.title.name"),
          "Open a file")),
      subtitle_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.picker.title.subtitle"),
          "local filesystem · bounded selection authority")),
      navigation_bar_(std::make_shared<gui_forms::Control>(
          gui_forms::StableId("file-manager.picker.navigation"))),
      back_to_root_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.picker.root"), "Home")),
      up_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.picker.up"), "Up")),
      path_(std::make_shared<gui_forms::TextBox>(
          gui_forms::StableId("file-manager.picker.path"))),
      locations_(std::make_shared<gui_forms::ComboBox>(
          gui_forms::StableId("file-manager.picker.locations"))),
      name_filter_(std::make_shared<gui_forms::TextBox>(
          gui_forms::StableId("file-manager.picker.name-filter"))),
      objects_(std::make_shared<gui_forms::ObjectView>(
          gui_forms::StableId("file-manager.picker.objects"))),
      filter_(std::make_shared<gui_forms::ComboBox>(
          gui_forms::StableId("file-manager.picker.filter"))),
      hidden_(std::make_shared<gui_forms::CheckBox>(
          gui_forms::StableId("file-manager.picker.hidden"), "Show hidden")),
      filename_(std::make_shared<gui_forms::TextBox>(
          gui_forms::StableId("file-manager.picker.filename"))),
      status_(std::make_shared<gui_forms::Label>(
          gui_forms::StableId("file-manager.picker.status"))),
      accept_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.picker.accept"),
          accept_title(controller_.request().profile))),
      cancel_(std::make_shared<gui_forms::Button>(
          gui_forms::StableId("file-manager.picker.cancel"), "Cancel")) {
    root_->set_requested_bounds({0, 0, 760, 560});
    root_->set_design_size({760, 560});
    root_->set_border_style(gui_forms::BorderStyle::line);
    root_->set_background(gui_forms::Color::rgba(231, 237, 246));

    title_bar_->set_authored_surface_material(watercolor_title_material());
    title_bar_->set_accessible_name("Open dialog title bar");
    title_->set_text(accept_title(controller_.request().profile) + " a " +
        (controller_.request().profile == DocumentPickerProfile::select_folder ? "folder" : "file"));
    title_->set_font({gui_forms::FontRole::control, 15.0, 700, false});
    title_->set_foreground(gui_forms::Color::rgba(255, 255, 255));
    title_->set_hit_test_transparent(true);
    subtitle_->set_font({gui_forms::FontRole::control, 10.0, 400, false});
    subtitle_->set_foreground(gui_forms::Color::rgba(225, 237, 255));
    subtitle_->set_hit_test_transparent(true);
    navigation_bar_->set_authored_surface_material(
        graphite_navigation_material());

    back_to_root_->set_accessible_name("Go to Home");
    up_->set_accessible_name("Go to parent folder");
    path_->set_accessible_name("Picker current location");
    path_->set_placeholder_text("Location inside the selection root");
    objects_->set_accessible_name("Picker objects");
    objects_->set_view_mode(gui_forms::ObjectViewMode::details);
    objects_->set_selection_mode(multiple_profile(controller_.request().profile)
        ? gui_forms::ObjectSelectionMode::multiple
        : gui_forms::ObjectSelectionMode::single);
    filter_->set_accessible_name("File type filter");
    hidden_->set_checked(controller_.show_hidden());
    hidden_->set_enabled(controller_.request().allow_hidden_toggle);
    filename_->set_accessible_name("File name");
    filename_->set_placeholder_text("File name");
    filename_->set_maximum_length(255);
    filename_->set_visible(save_profile(controller_.request().profile));
    accept_->set_accessible_description(
        "Acceptance revalidates filesystem identity and Orchestrator session");

    root_->add_at(title_bar_, {1, 1, 758, 40});
    root_->add_at(title_, {76, 9, 190, 23});
    root_->add_at(subtitle_, {272, 12, 390, 18});
    root_->add_at(navigation_bar_, {1, 41, 758, 50});
    root_->add_at(back_to_root_, {12, 50, 70, 32});
    root_->add_at(up_, {90, 50, 58, 32});
    root_->add_at(path_, {156, 50, 592, 32});
    locations_->set_accessible_name("Available selection roots");
    name_filter_->set_accessible_name("Filename filter");
    name_filter_->set_placeholder_text("Filter names (for example *.txt)");
    root_->add_at(locations_, {12, 101, 220, 32});
    root_->add_at(name_filter_, {244, 101, 504, 32});
    root_->add_at(objects_, {12, 143, 736, 324});
    root_->add_at(filter_, {12, 477, 220, 32});
    root_->add_at(hidden_, {244, 477, 130, 32});
    root_->add_at(filename_, {386, 477, 362, 32});
    root_->add_at(status_, {12, 519, 470, 30});
    root_->add_at(cancel_, {584, 517, 78, 34});
    root_->add_at(accept_, {670, 517, 78, 34});

    subscriptions_.push_back(locations_->selected_index_changed().subscribe(
        [this](const std::optional<std::size_t> index) {
            if (!reloading_ && index && *index < controller_.request().admitted_roots.size())
                navigate_path(controller_.request().admitted_roots[*index]);
        }));
    subscriptions_.push_back(name_filter_->committed().subscribe(
        [this](const std::string& value) {
            if (controller_.set_name_filter(value)) reload();
            else status_->set_text(controller_.last_error());
        }));
    subscriptions_.push_back(back_to_root_->clicked().subscribe(
        [this](gui_forms::ButtonBase&) {
            navigate_path(controller_.request().home_location);
        }));
    subscriptions_.push_back(up_->clicked().subscribe(
        [this](gui_forms::ButtonBase&) {
            const auto& location = controller_.browser().location;
            if (location != controller_.request().protected_root) {
                navigate_path(location.parent_path());
            }
        }));
    subscriptions_.push_back(path_->committed().subscribe(
        [this](const std::string& value) { navigate_path(path_from_utf8(value)); }));
    subscriptions_.push_back(path_->cancelled().subscribe(
        [this] { path_->set_text(path_utf8(controller_.browser().location)); }));
    subscriptions_.push_back(objects_->selection_changed().subscribe(
        [this](const gui_forms::ObjectSelectionChange&) {
            if (reloading_) return;
            std::vector<std::string> ids(objects_->selected_ids().begin(),
                                         objects_->selected_ids().end());
            if (!controller_.set_selection(std::move(ids))) {
                objects_->clear_selection();
                status_->set_text("Selection is outside this picker profile");
            } else {
                filename_->set_text(controller_.filename());
            }
        }));
    subscriptions_.push_back(objects_->item_activated().subscribe(
        [this](const std::string& stable_id) {
            const auto found = std::find_if(
                controller_.browser().entries.begin(),
                controller_.browser().entries.end(),
                [&stable_id](const DirectoryEntry& entry) {
                    return entry.stable_id == stable_id;
                });
            if (found == controller_.browser().entries.end()) return;
            if (found->directory) {
                navigate_path(found->path);
            } else if (!save_profile(controller_.request().profile)) {
                accept();
            }
        }));
    subscriptions_.push_back(filter_->selected_index_changed().subscribe(
        [this](const std::optional<std::size_t> index) {
            if (reloading_) return;
            if (!index || *index >= controller_.request().filters.size()) return;
            if (controller_.set_active_filter(
                    controller_.request().filters[*index].id)) {
                reload();
            }
        }));
    subscriptions_.push_back(hidden_->checked_changed().subscribe(
        [this](const bool checked) {
            if (reloading_) return;
            if (controller_.set_show_hidden(checked)) reload();
        }));
    subscriptions_.push_back(filename_->text_changed().subscribe(
        [this](const std::string& value) {
            (void)controller_.set_filename(value);
        }));
    subscriptions_.push_back(filename_->committed().subscribe(
        [this](const std::string&) { accept(); }));
    subscriptions_.push_back(accept_->clicked().subscribe(
        [this](gui_forms::ButtonBase&) { accept(); }));
    subscriptions_.push_back(cancel_->clicked().subscribe(
        [this](gui_forms::ButtonBase&) { publish(controller_.cancel()); }));

    reload();
}

std::shared_ptr<gui_forms::Control> DocumentPickerView::root_control() const {
    return root_;
}

FileSelectionController& DocumentPickerView::controller() noexcept {
    return controller_;
}

const FileSelectionController& DocumentPickerView::controller() const noexcept {
    return controller_;
}

gui_forms::Event<const DocumentPickerResult&>&
DocumentPickerView::completed() noexcept {
    return completed_;
}

void DocumentPickerView::set_orchestrator_session_valid(const bool valid) {
    controller_.set_orchestrator_session_valid(valid);
    status_->set_text(valid ? "Selection session ready"
                            : "Selection session unavailable · browsing only");
    accept_->set_enabled(valid);
}

void DocumentPickerView::set_authority_valid(const bool valid) {
    controller_.set_authority_valid(valid);
    update_status();
}

void DocumentPickerView::cancel() { publish(controller_.cancel()); }

void DocumentPickerView::confirm_overwrite() {
    accept(true);
}

void DocumentPickerView::present(
    const std::filesystem::path& initial_location) {
    navigate_path(initial_location);
    objects_->clear_selection();
    status_->set_text(std::to_string(controller_.browser().entries.size()) +
                      " visible objects · direct filesystem");
}

void DocumentPickerView::attach_dialog(gui_forms::Window& window) {
    window.set_accept_button(accept_);
    window.set_cancel_button(cancel_);
    window.request_focus(save_profile(controller_.request().profile)
        ? std::static_pointer_cast<gui_forms::Control>(filename_)
        : std::static_pointer_cast<gui_forms::Control>(objects_));
}

void DocumentPickerView::update_status() {
    accept_->set_enabled(controller_.session_valid() && !finished_);
    status_->set_text(controller_.session_valid()
        ? std::to_string(controller_.browser().entries.size()) + " visible objects" +
            (controller_.request().authority == DocumentPickerAuthority::trusted_local_host
                ? " � local host selection" : "")
        : "Selection session unavailable � browsing only");
}

void DocumentPickerView::reload() {
    reloading_ = true;
    path_->set_text(path_utf8(controller_.browser().location));
    std::vector<std::string> locations;
    std::size_t current_root = 0;
    for (const auto& root : controller_.request().admitted_roots) {
        if (root == controller_.request().protected_root) current_root = locations.size();
        locations.push_back(path_utf8(root));
    }
    locations_->set_items(std::move(locations));
    locations_->set_selected_index(current_root);
    back_to_root_->set_enabled(resolve_navigation_target(controller_.request().admitted_roots,
        controller_.browser().location, controller_.request().home_location,
        controller_.request().home_location).has_value());
    up_->set_enabled(controller_.browser().location !=
                     controller_.request().protected_root);
    std::vector<gui_forms::ObjectViewItem> items;
    items.reserve(controller_.browser().entries.size());
    for (const auto& entry : controller_.browser().entries) {
        items.push_back({entry.stable_id, entry.name, entry.secondary_text,
                         entry.directory ? "Folder" : "File", glyph(entry),
                         entry.kind != EntryKind::symlink, {}});
    }
    objects_->set_items(std::move(items));
    objects_->clear_selection();
    filename_->set_text(controller_.filename());
    std::vector<std::string> filters;
    filters.reserve(controller_.request().filters.size());
    std::optional<std::size_t> selected_filter;
    for (std::size_t index = 0; index < controller_.request().filters.size();
         ++index) {
        const auto& value = controller_.request().filters[index];
        filters.push_back(value.label);
        if (value.id == controller_.active_filter_id()) selected_filter = index;
    }
    filter_->set_items(std::move(filters));
    filter_->set_visible(!controller_.request().filters.empty());
    if (selected_filter) filter_->set_selected_index(*selected_filter);
    hidden_->set_checked(controller_.show_hidden());
    status_->set_text(std::to_string(controller_.browser().entries.size()) +
                      " visible objects · direct filesystem");
}

void DocumentPickerView::navigate_path(std::filesystem::path path) {
    if (controller_.navigate(path)) {
        reload();
    } else {
        path_->set_text(path_utf8(controller_.browser().location));
        status_->set_text("Location unavailable · " + controller_.last_error());
    }
}

void DocumentPickerView::accept(const bool overwrite_confirmed) {
    if (finished_) return;
    if (controller_.selected_ids().size() == 1 &&
        controller_.request().profile != DocumentPickerProfile::select_folder) {
        const auto id = controller_.selected_ids().front();
        for (const auto& entry : controller_.browser().entries) {
            if (entry.stable_id == id && entry.directory) {
                navigate_path(entry.path);
                return;
            }
        }
    }
    if (save_profile(controller_.request().profile)) {
        (void)controller_.set_filename(std::string(filename_->text()));
    }
    publish(controller_.accept(overwrite_confirmed));
}

void DocumentPickerView::publish(DocumentPickerResult result) {
    if (finished_) return;
    if (result.accepted() || result.terminal == DocumentPickerTerminal::cancelled) {
        finished_ = true;
        accept_->set_enabled(false);
    }
    if (result.terminal == DocumentPickerTerminal::validation_error ||
        result.terminal == DocumentPickerTerminal::unavailable) {
        status_->set_text(result.message);
    } else if (result.terminal ==
               DocumentPickerTerminal::overwrite_confirmation_required) {
        status_->set_text("Overwrite confirmation required");
    }
    completed_.emit(result);
}

} // namespace file_manager
