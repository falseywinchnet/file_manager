#include "file_manager_demoboard/demoboard.hpp"

#include "file_manager_demoboard/fixture_model.hpp"
#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace file_manager_demoboard {
namespace {

using namespace gui_forms;

class NavigationSession;
class PathMatrixController;
class SearchController;
class CriteriaController;

constexpr Color paper = Color::rgba(255, 255, 255);
constexpr Color ink = Color::rgba(29, 45, 75);
constexpr Color muted = Color::rgba(99, 112, 138);
constexpr Color line = Color::rgba(119, 140, 171);
constexpr Color graphite = Color::rgba(75, 84, 89);

struct ProductRefs final {
    std::shared_ptr<TableLayoutPanel> shell;
    Control::Ptr title_band;
    Control::Ptr ribbon_band;
    Control::Ptr navigation_band;
    Control::Ptr status_band;
    std::shared_ptr<Label> title;
    std::shared_ptr<MenuStrip> menu_strip;
    std::shared_ptr<TreeView> tree;
    std::shared_ptr<ObjectView> objects;
    std::shared_ptr<Label> selection_name;
    std::shared_ptr<PropertyList> properties;
    std::shared_ptr<Button> preview_disclosure;
    std::shared_ptr<Button> selection_collapse;
    std::shared_ptr<SplitContainer> selection_split;
    std::shared_ptr<SplitContainer> workspace_split;
    std::shared_ptr<Panel> surface_host;
    Control::Ptr folder_surface;
    Control::Ptr search_surface;
    std::shared_ptr<CorrespondenceView> search_results;
    Control::Ptr criteria_surface;
    std::shared_ptr<Label> criteria_title;
    std::shared_ptr<InstrumentRack> criteria_rack;
    std::shared_ptr<ObjectView> criteria_objects;
    std::shared_ptr<Label> criteria_results_summary;
    std::shared_ptr<Button> criteria_add;
    std::shared_ptr<Button> criteria_apply;
    std::shared_ptr<Label> criteria_action_state;
    std::shared_ptr<ProgressBar> criteria_progress;
    Control::Ptr preview;
    std::shared_ptr<Label> status_summary;
    std::shared_ptr<Label> status_authority;
    std::shared_ptr<Button> ribbon_view;
    std::shared_ptr<Button> ribbon_sort;
    std::shared_ptr<Button> ribbon_move_copy;
    std::shared_ptr<Button> ribbon_delete;
    std::shared_ptr<Button> ribbon_properties;
    std::shared_ptr<Button> status_view;
    std::shared_ptr<Button> nav_back;
    std::shared_ptr<Button> nav_forward;
    std::shared_ptr<Button> nav_up;
    std::vector<std::shared_ptr<Button>> breadcrumbs;
    std::vector<std::shared_ptr<Label>> breadcrumb_separators;
    std::shared_ptr<Button> path_terminal;
    std::shared_ptr<TextBox> search;
};

struct ProductLifetime final {
    std::vector<std::shared_ptr<Command>> commands;
    std::vector<std::shared_ptr<ContextMenu>> menus;
    std::vector<CommandBinding> bindings;
    std::vector<AcceleratorToken> accelerators;
    std::vector<SubscriptionToken> subscriptions;
    std::shared_ptr<SemanticFeedback> feedback;
    std::shared_ptr<NavigationSession> navigation;
    std::shared_ptr<PathMatrixController> path_matrix;
    std::shared_ptr<SearchController> search_controller;
    std::shared_ptr<CriteriaController> criteria_controller;
};

BasicControlStyle house_style() {
    BasicControlStyle style;
    style.face = Color::rgba(231, 237, 246);
    style.face_light = Color::rgba(250, 253, 255);
    style.paper = paper;
    style.highlight = Color::rgba(255, 255, 255);
    style.border = line;
    style.dark_border = Color::rgba(61, 95, 141);
    style.text = ink;
    style.disabled_text = muted;
    style.accent = Color::rgba(62, 97, 163);
    style.accent_light = Color::rgba(219, 230, 255);
    return style;
}

std::shared_ptr<Label> label(std::string id, std::string text, Rect bounds,
                             FontSpec font = {FontRole::content, 11.0, 400, false}) {
    auto result = std::make_shared<Label>(StableId(std::move(id)), std::move(text));
    result->set_requested_bounds(bounds);
    result->set_font(font);
    result->set_foreground(ink);
    return result;
}

std::shared_ptr<Button> button(std::string id, std::string text, Rect bounds,
                               ButtonVisualStyle visual = ButtonVisualStyle::standard) {
    auto result = std::make_shared<Button>(StableId(std::move(id)), std::move(text));
    result->set_requested_bounds(bounds);
    result->set_style(house_style());
    result->set_font({FontRole::control, 10.0, 600, false, 0.18});
    result->set_visual_style(visual);
    return result;
}

Color mix(Color a, Color b, double t) {
    const auto channel = [t](std::uint8_t x, std::uint8_t y) {
        return static_cast<std::uint8_t>(std::lround(
            static_cast<double>(x) + (static_cast<double>(y) - x) * t));
    };
    return Color::rgba(channel(a.red, b.red), channel(a.green, b.green),
                       channel(a.blue, b.blue), channel(a.alpha, b.alpha));
}

std::shared_ptr<DrawingSurface> material(std::string id,
                                         DrawingSurface::PaintCallback paint) {
    auto result = std::make_shared<DrawingSurface>(StableId(std::move(id)));
    result->set_paint_callback(std::move(paint));
    result->set_hit_test_visible(false);
    result->set_semantic_role(SemanticRole::generic);
    return result;
}

std::shared_ptr<ScaledPanel> make_title(ProductRefs& refs) {
    auto panel = std::make_shared<ScaledPanel>(StableId("fm.title.identity"),
                                               Size{1450.0, 40.0});
    panel->set_background(Color::rgba(23, 52, 127));
    auto fresco = material("fm.title.material", [](Painter& painter, Rect bounds, Rect) {
        constexpr Color start = Color::rgba(23, 52, 127);
        constexpr Color middle = Color::rgba(58, 104, 203);
        constexpr Color end = Color::rgba(217, 104, 114);
        const int strips = std::max(1, static_cast<int>(std::ceil(bounds.width / 8.0)));
        for (int index = 0; index < strips; ++index) {
            const double x = bounds.width * index / strips;
            const double next = bounds.width * (index + 1) / strips;
            const double t = (index + 0.5) / strips;
            const Color color = t < 0.62 ? mix(start, middle, t / 0.62)
                                         : mix(middle, end, (t - 0.62) / 0.38);
            painter.fill_rect({x, 0.0, next - x + 0.5, bounds.height}, color);
        }
        painter.fill_rect({0.0, 0.0, bounds.width * 0.24, 2.0},
                          Color::rgba(146, 217, 255, 150));
        painter.fill_rect({0.0, bounds.height - 2.0, bounds.width, 1.0},
                          Color::rgba(241, 249, 255, 210));
        painter.fill_rect({0.0, bounds.height - 1.0, bounds.width, 1.0},
                          Color::rgba(23, 45, 105));
    });
    panel->add_at(fresco, {0.0, 0.0, 1450.0, 40.0});
    auto mark = material("fm.title.icon", [](Painter& painter, Rect bounds, Rect) {
        painter.fill_rect({2.0, 2.0, bounds.width - 4.0, bounds.height - 4.0},
                          Color::rgba(239, 248, 255, 220));
        painter.stroke_rect({2.5, 2.5, bounds.width - 5.0, bounds.height - 5.0},
                            Color::rgba(23, 45, 105), 1.0);
        painter.fill_rect({7.0, 9.0, 17.0, 13.0}, Color::rgba(88, 169, 220));
        painter.fill_rect({12.0, 6.0, 9.0, 5.0}, Color::rgba(142, 217, 244));
    });
    panel->add_at(mark, {7.0, 5.0, 30.0, 30.0});
    auto title = label("fm.title.text", "File Manager  ·  Projects", {45.0, 4.0, 900.0, 32.0},
                       {FontRole::control, 16.0, 700, false, 0.32});
    title->set_foreground(Color::rgba(255, 255, 255));
    refs.title = title;
    panel->add_at(title, title->requested_bounds());
    auto state = label("fm.title.state", "LOCAL · GENERATION 86", {1160.0, 5.0, 274.0, 30.0},
                       {FontRole::control, 9.0, 600, false, 0.42});
    state->set_alignment(HorizontalAlignment::far);
    state->set_foreground(Color::rgba(245, 249, 255));
    panel->add_at(state, state->requested_bounds());
    return panel;
}

std::shared_ptr<MenuStrip> make_tabs(ProductRefs& refs) {
    auto strip = std::make_shared<MenuStrip>(StableId("fm.ribbon.tabs"));
    strip->set_accessible_name("File Manager application menu");
    refs.menu_strip = strip;
    return strip;
}

std::shared_ptr<ScaledPanel> make_ribbon(ProductRefs& refs) {
    auto panel = std::make_shared<ScaledPanel>(StableId("fm.ribbon.shelf"),
                                               Size{1450.0, 66.0});
    panel->set_background(Color::rgba(231, 237, 246));
    auto pearl = material("fm.ribbon.material", [](Painter& painter, Rect b, Rect) {
        painter.fill_rect(b, Color::rgba(231, 237, 246));
        painter.fill_rect({0.0, 0.0, b.width, 19.0}, Color::rgba(250, 253, 255));
        painter.fill_rect({0.0, 0.0, b.width, 2.0}, Color::rgba(146, 217, 255));
        painter.draw_line({0.0, b.height - 1.0}, {b.width, b.height - 1.0},
                          Color::rgba(119, 140, 171), 1.0);
    });
    panel->add_at(pearl, {0.0, 0.0, 1450.0, 66.0});
    refs.ribbon_move_copy = button("fm.ribbon.move_copy", "Move / copy", {10, 6, 96, 42}, ButtonVisualStyle::command);
    panel->add_at(refs.ribbon_move_copy, {10, 6, 96, 42});
    refs.ribbon_delete = button("fm.ribbon.delete", "Delete", {110, 6, 72, 42}, ButtonVisualStyle::command);
    panel->add_at(refs.ribbon_delete, {110, 6, 72, 42});
    panel->add_at(label("fm.ribbon.group.selection", "SELECTION", {10, 49, 172, 14}, {FontRole::control, 8, 600, false, .35}), {10, 49, 172, 14});
    refs.ribbon_view = button("fm.ribbon.view_mode", "Icons  ▼", {202, 6, 85, 42}, ButtonVisualStyle::command);
    panel->add_at(refs.ribbon_view, {202, 6, 85, 42});
    refs.ribbon_sort = button("fm.ribbon.sort", "Sort A→Z  ▼", {291, 6, 116, 42}, ButtonVisualStyle::command);
    panel->add_at(refs.ribbon_sort, {291, 6, 116, 42});
    refs.ribbon_properties = button("fm.ribbon.properties", "Properties", {411, 6, 92, 42}, ButtonVisualStyle::command);
    panel->add_at(refs.ribbon_properties, {411, 6, 92, 42});
    panel->add_at(label("fm.ribbon.group.arrange", "ARRANGE & INSPECT", {202, 49, 301, 14}, {FontRole::control, 8, 600, false, .35}), {202, 49, 301, 14});
    return panel;
}

std::shared_ptr<ScaledPanel> make_navigation(ProductRefs& refs) {
    auto panel = std::make_shared<ScaledPanel>(StableId("fm.navigation"),
                                               Size{1450.0, 40.0});
    panel->set_background(graphite);
    refs.nav_back = button("fm.nav.back", "←", {7, 6, 30, 28});
    refs.nav_back->set_enabled(false);
    panel->add_at(refs.nav_back, {7, 6, 30, 28});
    refs.nav_forward = button("fm.nav.forward", "→", {42, 6, 30, 28});
    refs.nav_forward->set_enabled(false);
    panel->add_at(refs.nav_forward, {42, 6, 30, 28});
    refs.nav_up = button("fm.nav.up", "↑", {77, 6, 30, 28});
    panel->add_at(refs.nav_up, {77, 6, 30, 28});
    auto well = std::make_shared<Panel>(StableId("fm.path.breadcrumb"));
    well->set_background(paper);
    well->set_border_style(BorderStyle::sunken);
    panel->add_at(well, {112, 6, 860, 28});
    const char* crumbs[] = {"quentin", "›", "Work", "›", "Projects"};
    double x = 120;
    for (std::size_t i = 0; i < std::size(crumbs); ++i) {
        if (i % 2U == 0U) {
            auto crumb = button("fm.path.segment." + std::to_string(i), crumbs[i],
                                {x, 7, 74.0, 26.0}, ButtonVisualStyle::flat);
            crumb->set_font({FontRole::content, 10, 600, false});
            refs.breadcrumbs.push_back(crumb);
            panel->add_at(crumb, crumb->requested_bounds());
            x += 74.0;
        } else {
            auto separator = label("fm.path.segment." + std::to_string(i), crumbs[i],
                                   {x, 7, 16.0, 26.0},
                                   {FontRole::control, 10, 400, false});
            refs.breadcrumb_separators.push_back(separator);
            panel->add_at(separator, separator->requested_bounds());
            x += 16.0;
        }
    }
    refs.path_terminal = button("fm.path.terminal", "./", {936, 7, 35, 26},
                                ButtonVisualStyle::flat);
    refs.path_terminal->set_accessible_name("Open complete path matrix");
    refs.path_terminal->set_accessible_description(
        "Browse full fixture paths or edit a path directly");
    refs.path_terminal->set_expanded_state(false);
    panel->add_at(refs.path_terminal, {936, 7, 35, 26});
    auto search = std::make_shared<TextBox>(StableId("fm.search.editor"));
    search->set_requested_bounds({980, 6, 463, 28});
    search->set_placeholder_text("Search Projects and descendants");
    search->set_style(house_style());
    refs.search = search;
    panel->add_at(search, search->requested_bounds());
    return panel;
}

ObjectGlyph object_glyph(std::string_view role) {
    if (role.find("folder") != std::string_view::npos) return ObjectGlyph::folder;
    if (role == "image") return ObjectGlyph::image;
    if (role == "music") return ObjectGlyph::audio;
    if (role == "archive") return ObjectGlyph::archive;
    if (role == "code") return ObjectGlyph::code;
    return ObjectGlyph::document;
}

std::shared_ptr<ObjectView> make_object_field(ProductRefs& refs) {
    auto field = std::make_shared<ObjectView>(StableId("fm.folder.objects"));
    field->set_requested_bounds({0.0, 0.0, 935.0, 657.0});
    field->set_background(paper);
    field->set_style(house_style());
    field->set_font({FontRole::content, 9.5, 400, false});
    field->set_icon_cell_size({112.0, 91.0});
    field->set_accessible_name("Projects objects");
    const auto& objects = FixtureCatalogue::instance().project_objects();
    std::vector<ObjectViewItem> items;
    items.reserve(objects.size());
    std::string selected;
    for (const auto& object : objects) {
        const std::string id = "fm.object." + object.id;
        items.push_back({id, object.name, object.size,
                         object.kind + " · " + object.size,
                         object_glyph(object.icon_role)});
        if (object.selected) selected = id;
    }
    field->set_items(std::move(items));
    field->set_selected_id(selected);
    refs.objects = field;
    return field;
}

std::shared_ptr<Panel> make_search_surface(ProductRefs& refs) {
    auto surface = std::make_shared<Panel>(StableId("fm.search.surface"));
    surface->set_background(paper);
    surface->set_accessible_name("Search correspondence surface");
    auto results = std::make_shared<CorrespondenceView>(
        StableId("fm.search.results"));
    results->set_style(house_style());
    results->set_font({FontRole::content, 11.0, 400, false});
    results->set_compact_height(45.0);
    results->set_expanded_height(126.0);
    results->set_hover_intent_delay(std::chrono::milliseconds(180));
    results->set_accessible_name("Search results");
    results->set_accessible_description(
        "Seven factual fixture correspondences with one pinned row and bounded virtual realization");
    std::vector<CorrespondenceItem> items;
    std::string pinned;
    for (const FixtureSearchResult& fixture :
         FixtureCatalogue::instance().search_results()) {
        const std::string id = "fm.result." + fixture.id;
        CorrespondenceItem item{
            id, fixture.name, fixture.location, fixture.metadata,
            fixture.excerpt, fixture.metric, "Plugin information",
            fixture.providers, fixture.provider_detail,
            object_glyph(fixture.icon_role), true, !fixture.available,
            fixture.stale};
        item.emphasis_terms = {"invoice", "quartz"};
        items.push_back(std::move(item));
        if (fixture.default_pinned) pinned = id;
    }
    results->set_items(std::move(items));
    results->set_selected_id(pinned);
    results->set_pinned_id(pinned);
    results->set_dock(DockStyle::fill);
    surface->add_child(results);
    refs.search_results = results;
    refs.search_surface = surface;
    surface->set_visible(false);
    return surface;
}

class CriteriaSurfacePanel final : public Panel {
public:
    explicit CriteriaSurfacePanel(StableId stable_id)
        : Panel(std::move(stable_id)) {
        set_background(paper);
        set_accessible_name("Criteria virtual folder surface");
    }

    std::shared_ptr<Label> title;
    std::shared_ptr<InstrumentRack> rack;
    Control::Ptr results;

    void arrange(Rect final_bounds) override {
        arrange_self(final_bounds);
        const double scale = effective_text_scale();
        const double width = std::max(0.0, final_bounds.width);
        const double height = std::max(0.0, final_bounds.height);
        const double rack_width = std::max(1.0, width - 18.0 * scale);
        const double desired_console = 30.0 * scale +
            (rack ? rack->preferred_height(rack_width) : 0.0) + 8.0 * scale;
        const double minimum_console = 104.0 * scale;
        const double maximum_console = std::max(
            minimum_console, height * (height < 280.0 * scale ? 0.72 : 0.58));
        const double console_height = std::min(
            height, std::clamp(desired_console, minimum_console,
                               maximum_console));
        if (title) {
            set_child_layout(title, {9.0 * scale, 5.0 * scale,
                                     std::max(0.0, width - 18.0 * scale),
                                     21.0 * scale});
        }
        if (rack) {
            set_child_layout(rack, {9.0 * scale, 30.0 * scale, rack_width,
                                    std::max(0.0, console_height -
                                                      38.0 * scale)});
        }
        if (results) {
            set_child_layout(results, {0.0, console_height, width,
                                       std::max(0.0, height - console_height)});
        }
    }

    void on_paint(Painter& painter, Rect damage) override {
        Panel::on_paint(painter, damage);
        const double scale = effective_text_scale();
        const double rack_width = std::max(
            1.0, committed_arranged_bounds().width - 18.0 * scale);
        const double desired_console = 30.0 * scale +
            (rack ? rack->preferred_height(rack_width) : 0.0) + 8.0 * scale;
        const double minimum_console = 104.0 * scale;
        const double maximum_console = std::max(
            minimum_console, committed_arranged_bounds().height *
                (committed_arranged_bounds().height < 280.0 * scale
                    ? 0.72 : 0.58));
        const double console_height = std::min(
            committed_arranged_bounds().height,
            std::clamp(desired_console, minimum_console, maximum_console));
        painter.fill_rect({0.0, 0.0, committed_arranged_bounds().width,
                           console_height}, Color::rgba(228, 237, 243));
        painter.fill_rect({0.0, 0.0, committed_arranged_bounds().width,
                           1.0 * scale}, Color::rgba(250, 253, 255));
        painter.draw_line({0.0, console_height - 0.5 * scale},
                          {committed_arranged_bounds().width,
                           console_height - 0.5 * scale},
                          Color::rgba(92, 119, 140), scale);
    }
};

InstrumentFieldSpec criterion_field(const FixtureCriterionField& fixture) {
    InstrumentFieldSpec field;
    field.stable_id = fixture.id;
    field.name = fixture.name;
    field.value = fixture.value;
    field.editor = fixture.editable_text ? InstrumentFieldEditor::text
                                         : InstrumentFieldEditor::choice;
    field.choices = fixture.choices;
    field.required = fixture.required;
    field.width_weight = fixture.width_weight;
    return field;
}

InstrumentModuleSpec criterion_module(const FixtureCriterionModule& fixture,
                                      std::string stable_id = {}) {
    InstrumentModuleSpec module;
    module.stable_id = stable_id.empty()
        ? "fm.criteria.module." + fixture.id : std::move(stable_id);
    module.name = fixture.name;
    module.status_text = fixture.status;
    module.state = fixture.staged ? InstrumentModuleState::staged
                                  : InstrumentModuleState::live;
    module.priority = fixture.priority;
    module.enabled = fixture.enabled;
    for (const FixtureCriterionField& field : fixture.fields) {
        module.fields.push_back(criterion_field(field));
    }
    return module;
}

std::vector<ObjectViewItem> criteria_object_items(std::size_t limit = 31U) {
    std::vector<ObjectViewItem> items;
    const auto fixtures = FixtureCatalogue::instance().criteria_objects();
    limit = std::min(limit, fixtures.size());
    items.reserve(limit);
    for (std::size_t index = 0; index < limit; ++index) {
        const FixtureObject& object = fixtures[index];
        items.push_back({"fm.object." + object.id, object.name, object.size,
                         object.kind + " · Projects and descendants",
                         object_glyph(object.icon_role)});
    }
    return items;
}

std::shared_ptr<CriteriaSurfacePanel> make_criteria_surface(ProductRefs& refs) {
    auto surface = make_control<CriteriaSurfacePanel>(
        StableId("fm.criteria.surface"));
    surface->title = label(
        "fm.criteria.console.title",
        "PROJECTS   ·   VIRTUAL FOLDER   ·   TWO LIVE MODULES   ·   ONE STAGED",
        {9.0, 5.0, 900.0, 21.0},
        {FontRole::control, 9.0, 700, false, .30});
    surface->title->set_accessible_name("Projects criteria console");
    surface->add_child(surface->title);
    refs.criteria_title = surface->title;

    auto rack = make_control<InstrumentRack>(StableId("fm.criteria.console"));
    rack->set_accessible_name("Projects predicate rack");
    rack->set_accessible_description(
        "Two live inexpensive modules and one staged expensive module");
    std::vector<InstrumentModuleSpec> modules;
    for (const FixtureCriterionModule& fixture :
         FixtureCatalogue::instance().criteria_modules()) {
        modules.push_back(criterion_module(fixture));
    }
    rack->set_modules(std::move(modules));

    auto actions = make_control<TableLayoutPanel>(
        StableId("fm.criteria.actions"));
    actions->set_accessible_name("Predicate rack actions");
    actions->set_column_count(2);
    actions->set_row_count(3);
    actions->set_column_style(0, {TableSizeMode::percent, 100.0});
    actions->set_column_style(1, {TableSizeMode::absolute, 78.0});
    actions->set_row_style(0, {TableSizeMode::absolute, 20.0});
    actions->set_row_style(1, {TableSizeMode::percent, 100.0});
    actions->set_row_style(2, {TableSizeMode::absolute, 9.0});
    actions->set_grow_style(TableLayoutGrowStyle::fixed_size);
    auto action_title = label("fm.criteria.actions.title", "PREDICATE RACK",
                              {0, 0, 120, 20},
                              {FontRole::control, 8.0, 700, false, .35});
    action_title->set_margin({6, 1, 3, 0});
    action_title->set_dock(DockStyle::fill);
    actions->add_child(action_title);
    actions->set_cell_position(*action_title, {0, 0});
    refs.criteria_add = button("fm.criteria.add", "+ module", {0, 0, 78, 27},
                               ButtonVisualStyle::standard);
    refs.criteria_add->set_margin({2, 1, 2, 1});
    refs.criteria_add->set_dock(DockStyle::fill);
    actions->add_child(refs.criteria_add);
    actions->set_cell_position(*refs.criteria_add, {1, 0});
    refs.criteria_action_state = label(
        "fm.criteria.actions.state", "1 expensive change staged",
        {0, 0, 120, 27}, {FontRole::content, 8.0, 600, false});
    refs.criteria_action_state->set_margin({6, 1, 3, 0});
    refs.criteria_action_state->set_dock(DockStyle::fill);
    actions->add_child(refs.criteria_action_state);
    actions->set_cell_position(*refs.criteria_action_state, {0, 1});
    refs.criteria_apply = button("fm.criteria.apply", "Apply 1", {0, 0, 78, 27},
                                 ButtonVisualStyle::accent);
    refs.criteria_apply->set_default_button(true);
    refs.criteria_apply->set_margin({2, 1, 2, 1});
    refs.criteria_apply->set_dock(DockStyle::fill);
    actions->add_child(refs.criteria_apply);
    actions->set_cell_position(*refs.criteria_apply, {1, 1});
    refs.criteria_progress = make_control<ProgressBar>(
        StableId("fm.criteria.progress"));
    refs.criteria_progress->set_range(0.0, 100.0);
    refs.criteria_progress->set_value(0.0);
    refs.criteria_progress->set_visual_style(ProgressBarVisualStyle::continuous);
    refs.criteria_progress->set_overlay_style(
        ProgressBarOverlayStyle::moving_stripes);
    refs.criteria_progress->set_animation_enabled(false);
    refs.criteria_progress->set_accessible_name("Criteria application progress");
    refs.criteria_progress->set_margin({5, 0, 5, 1});
    refs.criteria_progress->set_dock(DockStyle::fill);
    actions->add_child(refs.criteria_progress);
    actions->set_cell_position(*refs.criteria_progress, {0, 2});
    actions->set_column_span(*refs.criteria_progress, 2);
    rack->set_action_content(actions, 170.0);
    refs.criteria_rack = rack;
    surface->rack = rack;
    surface->add_child(rack);

    auto results = make_control<Panel>(StableId("fm.criteria.results"));
    results->set_background(paper);
    auto virtual_label = label(
        "fm.criteria.results.summary",
        "31 OBJECTS   ·   FROM PROJECTS AND DESCENDANTS   ·   ONE INSPECTABLE VIRTUAL FOLDER",
        {11, 2, 900, 28}, {FontRole::content, 9.0, 600, false});
    virtual_label->set_margin({11, 2, 8, 0});
    virtual_label->set_dock(DockStyle::top);
    refs.criteria_results_summary = virtual_label;
    auto objects = make_control<ObjectView>(StableId("fm.criteria.objects"));
    objects->set_items(criteria_object_items());
    objects->set_selected_id("fm.object.obj-facade-study");
    objects->set_icon_cell_size({112.0, 91.0});
    objects->set_font({FontRole::content, 9.5, 400, false});
    objects->set_style(house_style());
    objects->set_background(paper);
    objects->set_accessible_name("Criteria virtual folder objects");
    objects->set_margin({});
    objects->set_dock(DockStyle::fill);
    results->add_child(objects);
    results->add_child(virtual_label);
    surface->results = results;
    surface->add_child(results);
    refs.criteria_objects = objects;
    refs.criteria_surface = surface;
    surface->set_visible(false);
    return surface;
}

std::shared_ptr<Panel> make_tree_pane(ProductRefs& refs) {
    auto pane = std::make_shared<Panel>(StableId("fm.tree.pane"));
    pane->set_background(Color::rgba(235, 240, 246));
    auto tree = std::make_shared<TreeView>(StableId("fm.tree.view"));
    tree->set_items({
        {"fm.tree.node.local", "Local", 0, true, true},
        {"fm.tree.node.quentin", "quentin", 1, true, true},
        {"fm.tree.node.desktop", "Desktop", 2},
        {"fm.tree.node.documents", "Documents", 2},
        {"fm.tree.node.work", "Work", 2, true, true},
        {"fm.tree.node.projects", "Projects", 3},
        {"fm.tree.node.reference", "Reference", 3},
        {"fm.tree.node.field-notes", "Field Notes", 3},
        {"fm.tree.node.pictures", "Pictures", 2},
        {"fm.tree.node.downloads", "Downloads", 2},
        {"fm.tree.node.volumes", "Volumes", 0, true, true},
        {"fm.tree.node.macintosh-hd", "Macintosh HD", 1},
        {"fm.tree.node.archive-04", "Archive 04  · offline", 1},
    });
    tree->set_item_height(24.0);
    tree->set_font({FontRole::content, 10.5, 400, false});
    tree->set_style(house_style());
    tree->set_accessible_name("Folders");
    tree->set_selected_id("fm.tree.node.projects");
    refs.tree = tree;
    tree->set_dock(DockStyle::fill);
    pane->add_child(tree);
    // Dock layout follows WinForms z-order: the trailing/topmost child claims
    // its edge before the earlier fill child receives the remainder.
    auto caption = label("fm.tree.caption", "FOLDERS   ·   HOME-ROOTED", {7, 4, 204, 27},
                         {FontRole::control, 8.5, 700, false, .28});
    caption->set_margin({});
    caption->set_dock(DockStyle::top);
    pane->add_child(caption);
    return pane;
}

std::shared_ptr<Panel> make_selection_pane(ProductRefs& refs) {
    auto pane = std::make_shared<Panel>(StableId("fm.selection.pane"));
    pane->set_background(Color::rgba(235, 240, 246));
    auto caption = std::make_shared<Panel>(StableId("fm.selection.caption.material"));
    caption->set_background(Color::rgba(201, 212, 229));
    caption->set_border_style(BorderStyle::raised);
    caption->set_margin({});
    caption->set_requested_bounds({0, 0, 288, 27});
    caption->set_dock(DockStyle::top);
    auto caption_text = label("fm.selection.caption", "SELECTION", {8,2,272,23},
                              {FontRole::control,9,700,false,.35});
    caption_text->set_dock(DockStyle::fill);
    caption_text->set_margin({8, 2, 38, 2});
    caption->add_child(caption_text);
    refs.selection_collapse = button("fm.selection.collapse", "▶",
                                     {258, 2, 28, 23},
                                     ButtonVisualStyle::flat);
    refs.selection_collapse->set_accessible_name("Collapse Selection pane");
    refs.selection_collapse->set_accessible_description(
        "Collapse the retained Selection inspector toward the right edge");
    caption->add_child(refs.selection_collapse);
    auto properties = std::make_shared<PropertyList>(
        StableId("fm.selection.properties"));
    properties->set_accessible_name("Selection properties");
    properties->set_label_width(76.0);
    properties->set_margin({});
    properties->set_dock(DockStyle::fill);
    auto header = std::make_shared<ScaledPanel>(
        StableId("fm.selection.summary"), Size{288, 212});
    refs.selection_name = label("fm.selection.object_name", "Facade Study.png", {10,35,268,25}, {FontRole::content,12,700,false});
    header->add_at(refs.selection_name, {10, 4, 224, 25});
    refs.preview_disclosure = button("fm.selection.preview_disclosure", "▼",
                                     {244, 4, 34, 24}, ButtonVisualStyle::flat);
    header->add_at(refs.preview_disclosure, {244, 4, 34, 24});
    auto preview = material("fm.selection.preview", [](Painter& p, Rect b, Rect) {
        p.fill_rect(b, Color::rgba(29, 42, 55));
        p.fill_rect({8,8,b.width-16,b.height-16}, Color::rgba(194, 223, 238));
        p.fill_rect({8,b.height*0.63,b.width-16,b.height*0.37-8}, Color::rgba(119, 157, 105));
        p.fill_rect({52,45,b.width-104,70}, Color::rgba(239, 231, 208));
        p.fill_rect({66,60,42,39}, Color::rgba(88, 142, 181));
        p.fill_rect({b.width-108,60,42,39}, Color::rgba(88, 142, 181));
        p.stroke_rect({52.5,45.5,b.width-105,69}, Color::rgba(120, 105, 87),1);
        p.fill_rect({b.width-46,20,12,12}, Color::rgba(239, 177, 83));
    });
    refs.preview = preview;
    header->add_at(preview, {10, 34, 268, 174});
    properties->set_header_content(header, 212.0);
    properties->set_groups({
        {"fm.property.group.identity", "IDENTITY", {
            {"fm.property.kind", "Kind", "PNG image", "Selected object kind"},
            {"fm.property.location", "Location", "~/Work/Projects",
             "Selected object location"},
            {"fm.property.size", "Size", "2.8 MB on disk", "Selected object size"},
        }},
        {"fm.property.group.image", "IMAGE", {
            {"fm.property.dimensions", "Dimensions", "1600 × 1000",
             "Image dimensions"},
            {"fm.property.profile", "Profile", "Display P3", "Image color profile"},
            {"fm.property.created", "Created", "August 2, 2026", "Creation date"},
        }},
        {"fm.property.group.editable", "EDITABLE", {
            {"fm.property.name", "Name", "Facade Study.png",
             "Session-only fixture name", PropertyEditorKind::text, {}, {},
             true, true},
            {"fm.property.handler", "Opens with", "Preview",
             "Session-only fixture handler", PropertyEditorKind::choice,
             {"Preview", "Image Laboratory", "Fixture viewer"}},
        }},
    });
    refs.properties = properties;
    pane->add_child(properties);
    pane->add_child(caption);
    return pane;
}

std::shared_ptr<SplitContainer> make_workspace(ProductRefs& refs) {
    auto outer = std::make_shared<SplitContainer>(StableId("fm.workspace"));
    refs.workspace_split = outer;
    outer->initialize_control_tree();
    outer->set_splitter_width(3);
    outer->set_splitter_hit_width(9);
    outer->set_splitter_distance(218);
    outer->set_collapse_panel(SplitFixedPanel::first);
    outer->set_automatic_collapse_threshold(700);
    outer->set_first_minimum(120);
    outer->set_second_minimum(320);
    outer->set_first_maximum(360);
    outer->first_panel()->set_background(Color::rgba(235,240,246));
    outer->first_panel()->add_child(make_tree_pane(refs));
    outer->first_panel()->children().back()->set_dock(DockStyle::fill);

    auto inner = std::make_shared<SplitContainer>(StableId("fm.workspace.content_selection"));
    inner->initialize_control_tree();
    refs.selection_split = inner;
    inner->set_splitter_width(3);
    inner->set_splitter_hit_width(9);
    inner->set_fixed_panel(SplitFixedPanel::second);
    inner->set_splitter_distance(935);
    inner->set_collapse_panel(SplitFixedPanel::second);
    inner->set_automatic_collapse_threshold(900);
    inner->set_first_minimum(260);
    inner->set_second_minimum(288);
    inner->set_second_maximum(420);
    inner->first_panel()->set_background(paper);
    auto daily_content = std::make_shared<Panel>(
        StableId("fm.content.host"));
    daily_content->set_background(paper);
    daily_content->set_dock(DockStyle::fill);
    auto objects = make_object_field(refs);
    objects->set_dock(DockStyle::fill);
    daily_content->add_child(objects);
    auto criteria = make_criteria_surface(refs);
    criteria->set_dock(DockStyle::fill);
    daily_content->add_child(criteria);
    inner->first_panel()->add_child(daily_content);
    auto selection = make_selection_pane(refs);
    selection->set_dock(DockStyle::fill);
    inner->second_panel()->add_child(selection);
    inner->set_dock(DockStyle::fill);
    refs.folder_surface = inner;
    auto surface_host = std::make_shared<Panel>(StableId("fm.surface.host"));
    surface_host->set_background(paper);
    surface_host->set_dock(DockStyle::fill);
    surface_host->add_child(inner);
    auto search = make_search_surface(refs);
    search->set_dock(DockStyle::fill);
    surface_host->add_child(search);
    refs.surface_host = surface_host;
    outer->second_panel()->add_child(surface_host);
    return outer;
}

std::shared_ptr<ScaledPanel> make_status(ProductRefs& refs) {
    auto panel = std::make_shared<ScaledPanel>(StableId("fm.status"), Size{1450,24});
    panel->set_background(Color::rgba(231,237,246));
    panel->set_border_style(BorderStyle::raised);
    refs.status_summary = label("fm.status.summary", "1 selected · 2.8 MB · 14 objects", {8,1,590,22}, {FontRole::content,9.5,400,false});
    panel->add_at(refs.status_summary, {8,1,590,22});
    refs.status_authority = label("fm.status.authority", "Local navigation ready · index 86 current", {660,1,520,22}, {FontRole::content,9.5,600,false});
    refs.status_authority->set_alignment(HorizontalAlignment::far);
    panel->add_at(refs.status_authority,refs.status_authority->requested_bounds());
    refs.status_view = button("fm.status.view", "Icons ▼", {1235,2,205,20}, ButtonVisualStyle::flat);
    panel->add_at(refs.status_view, {1235,2,205,20});
    return panel;
}

std::string lower_ascii(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

struct NavigationLocation final {
    std::string id;
    std::string tree_id;
    std::string title;
    std::vector<std::pair<std::string, std::string>> path;
    std::vector<ObjectViewItem> items;
    std::vector<std::string> selected_ids;
    std::string primary_id;
    std::size_t top_row{};
};

std::vector<ObjectViewItem> location_items(std::string_view id) {
    if (id == "projects") {
        std::vector<ObjectViewItem> result;
        for (const auto& object : FixtureCatalogue::instance().project_objects()) {
            result.push_back({"fm.object." + object.id, object.name, object.size,
                              object.kind + " · " + object.size,
                              object_glyph(object.icon_role)});
        }
        return result;
    }
    const auto folder = [](std::string id, std::string name, std::string note = "Folder") {
        return ObjectViewItem{"fm.object.location." + std::move(id),
                              std::move(name), "Folder", std::move(note),
                              ObjectGlyph::folder};
    };
    if (id == "quentin") return {
        folder("desktop", "Desktop"), folder("documents", "Documents"),
        folder("work", "Work"), folder("pictures", "Pictures"),
        folder("downloads", "Downloads")};
    if (id == "work") return {
        folder("projects", "Projects", "14 fixture objects"),
        folder("reference", "Reference", "3 fixture objects"),
        folder("field-notes", "Field Notes", "2 fixture objects")};
    if (id == "reference") return {
        {"fm.object.reference-api", "API Notes.pdf", "840 KB",
         "Reference document", ObjectGlyph::document},
        {"fm.object.reference-colors", "House Colors.ase", "18 KB",
         "Color reference", ObjectGlyph::document},
        {"fm.object.reference-layout", "Layout Grid.png", "1.2 MB",
         "Image reference", ObjectGlyph::image}};
    if (id == "field-notes") return {
        {"fm.object.notes-review", "Review 2026-08-04.txt", "12 KB",
         "Plain text field note", ObjectGlyph::document},
        {"fm.object.notes-audio", "Usability Session.m4a", "8.4 MB",
         "Audio field note", ObjectGlyph::audio}};
    if (id == "orchard-study") return {
        {"fm.object.search-invoice-pdf", "Invoice 0428.pdf", "1.8 MB",
         "PDF document · matched invoice and extracted quartz text",
         ObjectGlyph::document},
        {"fm.object.orchard-billing", "Billing", "Folder",
         "Fixture billing records", ObjectGlyph::folder},
        {"fm.object.orchard-materials", "Material Schedule.xlsx", "96 KB",
         "Fixture material schedule", ObjectGlyph::document}};
    if (id == "north-shore") return {
        {"fm.object.search-quartz-image", "quartz-countertop-final.png", "8.4 MB",
         "PNG image · filename evidence", ObjectGlyph::image},
        {"fm.object.search-correspondence", "Client correspondence.rtf", "118 KB",
         "Rich text · extracted invoice and quartz evidence",
         ObjectGlyph::document},
        {"fm.object.search-stone-schedule", "Stone schedule.csv", "66 KB",
         "Delimited text · extracted quartz evidence", ObjectGlyph::code},
        {"fm.object.search-quartz-order", "Quartz order.msg", "84 KB",
         "Message metadata · stale relation", ObjectGlyph::document}};
    if (id == "local") return {
        folder("quentin", "quentin"), folder("volumes", "Volumes")};
    if (id == "volumes") return {
        folder("macintosh-hd", "Macintosh HD"),
        folder("archive-04", "Archive 04 · offline", "Unavailable fixture volume")};
    return {};
}

NavigationLocation make_location(std::string id) {
    NavigationLocation location;
    location.id = std::move(id);
    location.tree_id = location.id;
    const auto configure = [&location](std::string title,
                                       std::initializer_list<std::pair<std::string,
                                                                       std::string>> path) {
        location.title = std::move(title);
        location.path.assign(path.begin(), path.end());
    };
    if (location.id == "local") configure("Local", {{"local", "Local"}});
    else if (location.id == "volumes") configure("Volumes", {{"volumes", "Volumes"}});
    else if (location.id == "quentin") configure("quentin", {{"quentin", "quentin"}});
    else if (location.id == "work") configure("Work", {{"quentin", "quentin"}, {"work", "Work"}});
    else if (location.id == "projects") configure("Projects", {{"quentin", "quentin"}, {"work", "Work"}, {"projects", "Projects"}});
    else if (location.id == "reference") configure("Reference", {{"quentin", "quentin"}, {"work", "Work"}, {"reference", "Reference"}});
    else if (location.id == "field-notes") configure("Field Notes", {{"quentin", "quentin"}, {"work", "Work"}, {"field-notes", "Field Notes"}});
    else if (location.id == "desktop") configure("Desktop", {{"quentin", "quentin"}, {"desktop", "Desktop"}});
    else if (location.id == "documents") configure("Documents", {{"quentin", "quentin"}, {"documents", "Documents"}});
    else if (location.id == "pictures") configure("Pictures", {{"quentin", "quentin"}, {"pictures", "Pictures"}});
    else if (location.id == "downloads") configure("Downloads", {{"quentin", "quentin"}, {"downloads", "Downloads"}});
    else if (location.id == "macintosh-hd") configure("Macintosh HD", {{"volumes", "Volumes"}, {"macintosh-hd", "Macintosh HD"}});
    else if (location.id == "archive-04") configure("Archive 04", {{"volumes", "Volumes"}, {"archive-04", "Archive 04"}});
    else if (location.id == "orchard-study") {
        location.tree_id = "projects";
        configure("Orchard Study", {{"quentin", "quentin"}, {"work", "Work"},
            {"projects", "Projects"}, {"orchard-study", "Orchard Study"}});
    } else if (location.id == "north-shore") {
        location.tree_id = "projects";
        configure("North Shore", {{"quentin", "quentin"}, {"work", "Work"},
            {"projects", "Projects"}, {"north-shore", "North Shore"}});
    } else if (location.id == "print-masters") {
        location.tree_id = "projects";
        configure("Print Masters", {{"quentin", "quentin"}, {"work", "Work"},
            {"projects", "Projects"}, {"print-masters", "Print Masters"}});
    } else if (location.id == "legal-2026") {
        location.tree_id = "documents";
        configure("2026", {{"quentin", "quentin"}, {"documents", "Documents"},
            {"legal", "Legal"}, {"legal-2026", "2026"}});
    } else if (location.id == "pictures-scans") {
        location.tree_id = "pictures";
        configure("Scans", {{"quentin", "quentin"}, {"pictures", "Pictures"},
            {"pictures-scans", "Scans"}});
    } else if (location.id == "reference-materials") {
        location.tree_id = "reference";
        configure("Materials", {{"quentin", "quentin"}, {"work", "Work"},
            {"reference", "Reference"}, {"reference-materials", "Materials"}});
    } else {
        location.tree_id = "archive-04";
        configure("Material Library", {{"volumes", "Volumes"},
            {"archive-04", "Archive 04"}, {"archive-material-library", "Material Library"}});
    }
    location.items = location_items(location.id);
    if (location.id == "projects") {
        location.primary_id = "fm.object.obj-facade-study";
        location.selected_ids = {location.primary_id};
    }
    return location;
}

std::string parent_location(std::string_view id) {
    if (id == "projects" || id == "reference" || id == "field-notes") return "work";
    if (id == "work" || id == "desktop" || id == "documents" ||
        id == "pictures" || id == "downloads") return "quentin";
    if (id == "quentin" || id == "volumes") return "local";
    if (id == "macintosh-hd" || id == "archive-04") return "volumes";
    return {};
}

class NavigationSession final {
public:
    NavigationSession(ProductRefs refs, std::shared_ptr<Command> back,
                      std::shared_ptr<Command> forward,
                      std::shared_ptr<Command> up,
                      std::function<void()> location_feedback)
        : refs_(std::move(refs)), back_(std::move(back)),
          forward_(std::move(forward)), up_(std::move(up)),
          location_feedback_(std::move(location_feedback)) {
        history_.push_back(make_location("projects"));
        apply();
    }

    void navigate(std::string id) {
        if (applying_ || id.empty() || history_[index_].id == id) return;
        snapshot_current();
        history_.erase(history_.begin() + static_cast<std::ptrdiff_t>(index_ + 1U),
                       history_.end());
        history_.push_back(make_location(std::move(id)));
        index_ = history_.size() - 1U;
        apply();
        if (location_feedback_) location_feedback_();
    }

    void back() {
        if (index_ == 0U) return;
        snapshot_current();
        --index_;
        apply();
        if (location_feedback_) location_feedback_();
    }

    void forward() {
        if (index_ + 1U >= history_.size()) return;
        snapshot_current();
        ++index_;
        apply();
        if (location_feedback_) location_feedback_();
    }

    void up() {
        const std::string parent = parent_location(history_[index_].id);
        if (!parent.empty()) navigate(parent);
    }

    void breadcrumb(std::size_t slot) {
        if (slot >= history_[index_].path.size()) return;
        navigate(history_[index_].path[slot].first);
    }

    [[nodiscard]] const std::string& current_id() const noexcept {
        return history_[index_].id;
    }

    void restore_projection() { apply(); }

private:
    void snapshot_current() {
        NavigationLocation& location = history_[index_];
        location.items.assign(refs_.objects->items().begin(), refs_.objects->items().end());
        location.selected_ids.assign(refs_.objects->selected_ids().begin(),
                                     refs_.objects->selected_ids().end());
        location.primary_id = std::string(refs_.objects->selected_id());
        location.top_row = refs_.objects->top_row();
    }

    void apply() {
        applying_ = true;
        NavigationLocation& location = history_[index_];
        std::optional<UpdateScope> update;
        if (Window* window = refs_.objects->attached_window()) {
            update.emplace(window->begin_update());
        }
        refs_.objects->set_items(location.items);
        if (!location.selected_ids.empty()) {
            refs_.objects->set_selected_ids(location.selected_ids,
                                            location.primary_id);
        } else {
            refs_.objects->clear_selection();
        }
        if (!location.items.empty()) {
            const std::size_t rows = location.items.size();
            refs_.objects->set_top_row(std::min(location.top_row, rows - 1U));
        }
        refs_.objects->set_accessible_name(location.title + " objects");
        refs_.tree->set_selected_id("fm.tree.node." + location.tree_id);
        refs_.title->set_text("File Manager  ·  " + location.title);
        refs_.search->set_placeholder_text("Search " + location.title + " and descendants");
        refs_.status_authority->set_text("Local navigation · " + location.id +
                                         " · index 86 current");
        refs_.status_summary->set_text(location.selected_ids.empty()
            ? std::to_string(location.items.size()) + " objects"
            : std::to_string(location.selected_ids.size()) + " selected · " +
                std::to_string(location.items.size()) + " objects");
        for (std::size_t slot = 0; slot < refs_.breadcrumbs.size(); ++slot) {
            const bool visible = slot < location.path.size();
            refs_.breadcrumbs[slot]->set_visible(visible);
            if (visible) refs_.breadcrumbs[slot]->set_text(location.path[slot].second);
        }
        for (std::size_t slot = 0; slot < refs_.breadcrumb_separators.size(); ++slot) {
            refs_.breadcrumb_separators[slot]->set_visible(slot + 1U < location.path.size());
        }
        back_->set_enabled(index_ > 0U);
        forward_->set_enabled(index_ + 1U < history_.size());
        const bool can_up = !parent_location(location.id).empty();
        up_->set_enabled(can_up);
        applying_ = false;
    }

    ProductRefs refs_;
    std::shared_ptr<Command> back_;
    std::shared_ptr<Command> forward_;
    std::shared_ptr<Command> up_;
    std::vector<NavigationLocation> history_;
    std::size_t index_{};
    bool applying_{};
    std::function<void()> location_feedback_;
};

class PathMatrixContent final : public ScaledPanel {
public:
    explicit PathMatrixContent(StableId stable_id)
        : ScaledPanel(std::move(stable_id), {860.0, 424.0}) {}

    std::function<void(KeyEvent&)> key_preview;

    void on_key_preview(KeyEvent& event) override {
        if (key_preview) key_preview(event);
    }
};

class PathMatrixController final : public Component {
public:
    PathMatrixController(ProductRefs refs,
                         std::shared_ptr<NavigationSession> navigation,
                         std::function<void(std::string)> status)
        : refs_(std::move(refs)), navigation_(std::move(navigation)),
          status_(std::move(status)) {
        terminal_click_ = refs_.path_terminal->clicked().subscribe(
            *this, [this](ButtonBase&) { toggle(); });
    }

    [[nodiscard]] bool open() const noexcept { return popup_token_.connected(); }
    [[nodiscard]] bool editing() const noexcept { return editing_; }
    [[nodiscard]] std::uint64_t completion_generation() const noexcept {
        return completion_generation_;
    }
    void dismiss() { close(); }

private:
    static std::string segment_id(std::string_view text) {
        std::string result;
        result.reserve(text.size());
        for (const unsigned char value : text) {
            if (std::isalnum(value)) result.push_back(
                static_cast<char>(std::tolower(value)));
            else if (result.empty() || result.back() != '-') result.push_back('-');
        }
        while (!result.empty() && result.back() == '-') result.pop_back();
        return result.empty() ? "root" : result;
    }

    void toggle() {
        if (open()) close();
        else open_browse();
    }

    void open_browse() {
        Window* window = refs_.path_terminal->attached_window();
        if (!window || open()) return;
        build_popup();
        AnchoredPopupPlacement placement;
        placement.preferred_size = {860.0, 424.0};
        placement.horizontal_alignment = PopupHorizontalAlignment::far;
        placement.vertical_preference = PopupVerticalPreference::below;
        placement.gap = 7.0;
        placement.viewport_margin = 4.0;
        layer_ = make_control<AnchoredPopupLayer>(
            StableId("fm.path.matrix.layer"), refs_.path_terminal, placement);
        layer_->set_accessible_name("Complete path matrix");
        layer_->set_accessible_description(
            "Drive-rooted current path, direct editor, completions, and five recent paths");
        layer_->set_content(content_);
        layer_->set_requested_bounds(
            {0.0, 0.0, window->client_size().width, window->client_size().height});
        popup_dismissal_ = layer_->dismiss_requested().subscribe(
            *this, [this](PopupDismissReason reason) {
                if (reason == PopupDismissReason::escape_key && editing_) {
                    show_browse();
                    return;
                }
                close();
            });
        popup_token_ = window->open_popup(refs_.path_terminal, layer_);
        if (Event<>* closed = popup_token_.closed_event()) {
            popup_revocation_ = closed->subscribe(*this, [this] {
                on_popup_revoked();
            });
        }
        focus_scope_ = window->begin_focus_scope(layer_, current_tail_);
        refs_.path_terminal->set_expanded_state(true);
        refs_.path_terminal->set_accessible_description(
            "Complete path matrix open · activate to close");
        status_("Path matrix open · full current stack · five recent locations");
    }

    void build_popup() {
        content_ = make_control<PathMatrixContent>(StableId("fm.path.matrix"));
        content_->set_background(Color::rgba(229, 234, 235));
        content_->set_border_style(BorderStyle::raised);
        content_->set_accessible_name("Complete path matrix");
        content_->key_preview = [this](KeyEvent& event) { handle_key(event); };

        auto heading = label("fm.path.matrix.heading", "COMPLETE PATH MATRIX",
                             {10.0, 5.0, 840.0, 34.0},
                             {FontRole::control, 12.0, 700, false, .45});
        heading->set_foreground(Color::rgba(247, 250, 251));
        heading->set_accessible_description(
            "Browse a full drive-rooted stack or edit the current fixture path");
        auto heading_back = material("fm.path.matrix.heading.material",
            [](Painter& painter, Rect bounds, Rect) {
                painter.fill_rect(bounds, Color::rgba(75, 84, 89));
                painter.draw_line({0.0, bounds.height - 1.0},
                                  {bounds.width, bounds.height - 1.0},
                                  Color::rgba(32, 45, 53), 1.0);
            });
        content_->add_at(heading_back, {0.0, 0.0, 860.0, 40.0});
        content_->add_at(heading, heading->requested_bounds());

        current_ = make_control<ScaledPanel>(
            StableId("fm.path.matrix.current"), Size{840.0, 60.0});
        current_->set_background(Color::rgba(251, 248, 237));
        current_->set_border_style(BorderStyle::sunken);
        current_->set_accessible_name("Current full path");
        const auto& current = FixtureCatalogue::instance().current_path_stack();
        double x = 8.0;
        const std::array<double, 5> widths{112.0, 66.0, 80.0, 62.0, 86.0};
        for (std::size_t index = 0; index < current.segments.size(); ++index) {
            const std::string id = segment_id(current.segments[index]);
            auto segment = button("fm.path.matrix.current.segment." + id,
                                  current.segments[index],
                                  {x, 13.0, widths[index], 33.0},
                                  ButtonVisualStyle::flat);
            segment->set_accessible_description(
                "Navigate to this ancestor in the current fixture stack");
            current_->add_at(segment, segment->requested_bounds());
            const std::string destination = index + 1U == current.segments.size()
                ? current.id : index == 0U ? "macintosh-hd"
                : index <= 2U ? "quentin" : "work";
            popup_controls_.push_back(segment->clicked().subscribe(
                *this, [this, destination](ButtonBase&) {
                    navigate_and_close(destination);
                }));
            x += widths[index];
            if (index + 1U < current.segments.size()) {
                auto separator = label(
                    "fm.path.matrix.current.separator." + std::to_string(index),
                    "›", {x, 13.0, 18.0, 33.0},
                    {FontRole::control, 11.0, 500, false});
                separator->set_alignment(HorizontalAlignment::center);
                current_->add_at(separator, separator->requested_bounds());
                x += 18.0;
            }
        }
        current_tail_ = button("fm.path.matrix.current.tail", "… /",
                               {x + 4.0, 13.0, 58.0, 33.0},
                               ButtonVisualStyle::flat);
        current_tail_->set_accessible_name("Edit complete path");
        current_tail_->set_accessible_description(
            "Replace the current stack with a direct path editor");
        current_->add_at(current_tail_, current_tail_->requested_bounds());
        popup_controls_.push_back(current_tail_->clicked().subscribe(
            *this, [this](ButtonBase&) { show_editing(); }));
        content_->add_at(current_, {10.0, 44.0, 840.0, 60.0});

        editor_ = make_control<TextBox>(StableId("fm.path.matrix.editor"));
        editor_->set_style(house_style());
        editor_->set_font({FontRole::content, 11.0, 500, false});
        editor_->set_accessible_name("Direct fixture path");
        editor_->set_accessible_description(
            "Type a fixture path or one of the admitted HOME and PROJECTS variables");
        editor_->set_visible(false);
        content_->add_at(editor_, {10.0, 48.0, 840.0, 33.0});
        popup_controls_.push_back(editor_->text_changed().subscribe(
            *this, [this](const std::string&) { refresh_completions(); }));

        completions_ = make_control<ListBox>(
            StableId("fm.path.matrix.completions"));
        completions_->set_style(house_style());
        completions_->set_font({FontRole::content, 10.0, 500, false});
        completions_->set_item_height(25.0);
        completions_->set_accessible_name("Path completions");
        completions_->set_visible(false);
        content_->add_at(completions_, {10.0, 85.0, 840.0, 79.0});
        popup_controls_.push_back(completions_->item_activated().subscribe(
            *this, [this](std::size_t index) { accept_completion(index); }));

        resolution_ = label("fm.path.matrix.resolution", "", {12.0, 166.0, 836.0, 25.0},
                            {FontRole::content, 9.5, 600, false});
        resolution_->set_text_wrapping(TextWrapping::no_wrap);
        resolution_->set_visible(false);
        content_->add_at(resolution_, resolution_->requested_bounds());

        recents_ = make_control<ScaledPanel>(StableId("fm.path.matrix.recents"),
                                             Size{840.0, 306.0});
        recents_->set_background(Color::rgba(243, 246, 251));
        recents_->set_accessible_name("Five recent complete paths");
        auto recent_heading = label("fm.path.matrix.recents.heading", "RECENT FULL STACKS",
                                    {4.0, 0.0, 832.0, 25.0},
                                    {FontRole::control, 9.0, 700, false, .35});
        recents_->add_at(recent_heading, recent_heading->requested_bounds());
        double recent_y = 27.0;
        for (const FixturePath& path : FixtureCatalogue::instance().recent_paths()) {
            auto row = make_control<ScaledPanel>(
                StableId("fm.path.matrix.recent." + path.id), Size{830.0, 49.0});
            row->set_background(path.available ? Color::rgba(255, 255, 255)
                                               : Color::rgba(238, 238, 235));
            row->set_border_style(BorderStyle::line);
            row->set_accessible_name(path.path);
            row->set_accessible_description(path.available
                ? "Recent fixture path" : "Recent stored path · source volume offline");
            double segment_x = 5.0;
            for (std::size_t index = 0; index < path.segments.size(); ++index) {
                const double width = std::clamp(
                    18.0 + static_cast<double>(path.segments[index].size()) * 6.2,
                    48.0, 146.0);
                auto segment = button(
                    "fm.path.matrix.recent." + path.id + ".segment." +
                        std::to_string(index), path.segments[index],
                    {segment_x, 8.0, width, 31.0}, ButtonVisualStyle::flat);
                segment->set_accessible_description(
                    path.available ? "Navigate to this recent fixture stack"
                                   : "Navigate to stored offline fixture state");
                row->add_at(segment, segment->requested_bounds());
                popup_controls_.push_back(segment->clicked().subscribe(
                    *this, [this, destination = path.id](ButtonBase&) {
                        navigate_and_close(destination);
                    }));
                segment_x += width;
                if (index + 1U < path.segments.size()) {
                    auto separator = label(
                        "fm.path.matrix.recent." + path.id + ".separator." +
                            std::to_string(index), "›",
                        {segment_x, 8.0, 15.0, 31.0},
                        {FontRole::control, 9.0, 500, false});
                    separator->set_alignment(HorizontalAlignment::center);
                    row->add_at(separator, separator->requested_bounds());
                    segment_x += 15.0;
                }
            }
            recents_->add_at(row, {5.0, recent_y, 830.0, 47.0});
            recent_y += 51.0;
        }
        content_->add_at(recents_, {10.0, 110.0, 840.0, 306.0});
    }

    void show_editing() {
        if (!content_ || editing_) return;
        editing_ = true;
        current_->set_visible(false);
        editor_->set_visible(true);
        completions_->set_visible(true);
        resolution_->set_visible(true);
        content_->set_design_bounds(*recents_, {10.0, 194.0, 840.0, 222.0});
        const std::string value =
            std::string(FixtureCatalogue::instance().current_path()) + "/";
        editor_->set_text(value);
        editor_->select(Utf8Offset(value.size()), Utf8Offset(value.size()));
        refresh_completions();
        if (Window* window = editor_->attached_window()) {
            static_cast<void>(window->request_focus(editor_));
        }
        status_("Direct path editing · Tab accepts · Enter resolves · Escape returns to browse");
    }

    void show_browse() {
        if (!content_ || !editing_) return;
        editing_ = false;
        current_->set_visible(true);
        editor_->set_visible(false);
        completions_->set_visible(false);
        resolution_->set_visible(false);
        content_->set_design_bounds(*recents_, {10.0, 110.0, 840.0, 306.0});
        if (Window* window = current_tail_->attached_window()) {
            static_cast<void>(window->request_focus(current_tail_));
        }
        status_("Path matrix browse · Escape closes · choose a full stack");
    }

    void refresh_completions() {
        if (!editor_ || !completions_) return;
        const std::uint64_t generation = ++completion_generation_;
        auto matches = FixtureCatalogue::instance().complete_path(editor_->text());
        static_cast<void>(pending_completion_.cancel());
        pending_completion_ = editor_->begin_invoke(
            [this, generation, matches = std::move(matches)]() mutable {
                apply_completion_generation(generation, std::move(matches));
            });
    }

    void apply_completion_generation(
        std::uint64_t generation,
        std::vector<FixturePathCompletion> matches) {
        if (!editing_ || !editor_ || !completions_ || !resolution_) return;
        // Only the latest posted provider result may reach the retained model.
        // Replacing an in-flight request explicitly cancels its dispatcher
        // operation, while the generation check also rejects work that had
        // already begun before a newer request was issued.
        if (generation != completion_generation_) return;
        std::vector<std::string> values;
        std::vector<std::string> ids;
        for (const auto& match : matches) {
            values.push_back(match.path);
            ids.push_back("fm.path.matrix.completion." + match.id);
        }
        completions_->set_items(std::move(values));
        completions_->set_item_stable_ids(std::move(ids));
        if (!completions_->items().empty()) completions_->select_index(0U);
        resolution_->set_text("Completion generation " +
                              std::to_string(generation) + " · " +
                              std::to_string(completions_->items().size()) +
                              " fixture matches");
        resolution_->set_accessible_description(resolution_->text());
    }

    void accept_completion(std::size_t index) {
        if (!editor_ || !completions_ || index >= completions_->items().size()) return;
        const std::string value(completions_->items()[index]);
        editor_->set_text(value);
        editor_->select(Utf8Offset(value.size()), Utf8Offset(value.size()));
        if (Window* window = editor_->attached_window()) {
            static_cast<void>(window->request_focus(editor_));
        }
        status_("Path completion accepted · Enter navigates");
    }

    void handle_key(KeyEvent& event) {
        if (!editing_ || event.action != KeyAction::down || !editor_ ||
            !completions_) return;
        if (event.physical_key == PhysicalKey::up ||
            event.physical_key == PhysicalKey::down) {
            if (!completions_->items().empty()) {
                std::size_t index = completions_->selected_index().value_or(0U);
                if (event.physical_key == PhysicalKey::up) {
                    index = index == 0U ? 0U : index - 1U;
                } else {
                    index = std::min(index + 1U, completions_->items().size() - 1U);
                }
                completions_->select_index(index);
            }
            event.handled = true;
            return;
        }
        if (event.physical_key == PhysicalKey::tab) {
            if (const auto selected = completions_->selected_index()) {
                accept_completion(*selected);
            }
            event.handled = true;
            return;
        }
        if (event.physical_key == PhysicalKey::enter) {
            const FixturePathResolution result =
                FixtureCatalogue::instance().resolve_path(editor_->text());
            if (result.valid) {
                status_(result.explanation + " · " + result.expanded_path);
                navigate_and_close(result.destination_id);
            } else {
                resolution_->set_text("Path not resolved · " + result.explanation);
                resolution_->set_foreground(Color::rgba(139, 43, 47));
                resolution_->set_accessible_description(
                    "Error: " + result.explanation);
                status_("Path rejected · " + result.explanation);
            }
            event.handled = true;
        }
    }

    void navigate_and_close(const std::string& destination) {
        if (navigation_) navigation_->navigate(destination);
        close();
    }

    void close() {
        if (closing_ || !layer_) return;
        closing_ = true;
        if (Window* window = refs_.path_terminal->attached_window();
            window && focus_scope_) {
            static_cast<void>(window->end_focus_scope(focus_scope_));
        }
        focus_scope_ = {};
        popup_dismissal_.disconnect();
        popup_token_.disconnect();
        popup_revocation_.disconnect();
        reset_popup();
        refs_.path_terminal->set_expanded_state(false);
        refs_.path_terminal->set_accessible_description(
            "Browse full fixture paths or edit a path directly");
        closing_ = false;
    }

    void on_popup_revoked() {
        if (closing_) return;
        closing_ = true;
        if (Window* window = refs_.path_terminal->attached_window();
            window && focus_scope_) {
            static_cast<void>(window->end_focus_scope(
                focus_scope_, FocusScopeCloseReason::owner_unavailable));
        }
        focus_scope_ = {};
        popup_dismissal_.disconnect();
        popup_revocation_.disconnect();
        reset_popup();
        refs_.path_terminal->set_expanded_state(false);
        closing_ = false;
    }

    void reset_popup() {
        static_cast<void>(pending_completion_.cancel());
        pending_completion_ = {};
        popup_controls_.clear();
        layer_.reset();
        content_.reset();
        current_.reset();
        current_tail_.reset();
        editor_.reset();
        completions_.reset();
        resolution_.reset();
        recents_.reset();
        editing_ = false;
    }

    ProductRefs refs_;
    std::shared_ptr<NavigationSession> navigation_;
    std::function<void(std::string)> status_;
    std::shared_ptr<AnchoredPopupLayer> layer_;
    std::shared_ptr<PathMatrixContent> content_;
    std::shared_ptr<ScaledPanel> current_;
    std::shared_ptr<Button> current_tail_;
    std::shared_ptr<TextBox> editor_;
    std::shared_ptr<ListBox> completions_;
    std::shared_ptr<Label> resolution_;
    std::shared_ptr<ScaledPanel> recents_;
    PopupToken popup_token_;
    FocusScopeId focus_scope_;
    SubscriptionToken terminal_click_;
    SubscriptionToken popup_dismissal_;
    SubscriptionToken popup_revocation_;
    std::vector<SubscriptionToken> popup_controls_;
    DispatchOperation pending_completion_;
    std::uint64_t completion_generation_{};
    bool editing_{};
    bool closing_{};
};

class SearchController final : public Component {
public:
    SearchController(ProductRefs refs,
                     std::shared_ptr<NavigationSession> navigation,
                     std::shared_ptr<PathMatrixController> path_matrix,
                     std::function<void(std::string)> status,
                     std::function<void(SemanticFeedbackKind)> feedback)
        : refs_(std::move(refs)), navigation_(std::move(navigation)),
          path_matrix_(std::move(path_matrix)), status_(std::move(status)),
          feedback_(std::move(feedback)) {
        text_changed_ = refs_.search->text_changed().subscribe(
            *this, [this](const std::string& query) {
                if (!applying_) schedule_query(query);
            });
        committed_ = refs_.search->committed().subscribe(
            *this, [this](const std::string& query) {
                if (!query.empty()) show_search(query, true);
            });
        cancelled_ = refs_.search->cancelled().subscribe(
            *this, [this] {
                if (search_visible_) static_cast<void>(select_surface("folder"));
            });
        selected_ = refs_.search_results->selection_changed().subscribe(
            *this, [this](const CorrespondenceSelectionChange& change) {
                update_context_availability(change.current_id);
            });
        pinned_ = refs_.search_results->pin_changed().subscribe(
            *this, [this](const CorrespondencePinChange& change) {
                if (change.current_id.empty()) {
                    status_("Search correspondence unpinned · hover and keyboard inspection remain independent");
                } else {
                    status_("Search correspondence pinned · " +
                            title_for(change.current_id));
                }
                feedback_(SemanticFeedbackKind::pane_changed);
            });
        expanded_ = refs_.search_results->expansion_changed().subscribe(
            *this, [this](const CorrespondenceExpansionChange& change) {
                if (change.reason == CorrespondenceExpansionReason::hover_intent) {
                    status_(change.expanded
                        ? "Search evidence inspected · " + title_for(change.stable_id)
                        : search_status());
                }
            });
        activated_ = refs_.search_results->item_activated().subscribe(
            *this, [this](const std::string& stable_id) {
                activate(stable_id, true);
            });
        context_requested_ = refs_.search_results->context_requested().subscribe(
            *this, [this](const ObjectContextRequest& request) {
                context_id_ = request.stable_id;
                update_context_availability(context_id_);
                context_menu_->show(refs_.search_results,
                                    request.screen_position);
            });
        build_context_menu();
        update_context_availability(refs_.search_results->selected_id());
    }

    [[nodiscard]] bool select_surface(std::string_view surface) {
        const std::string normalized = lower_ascii(surface);
        if (normalized == "folder") {
            pending_query_.disconnect();
            ++query_generation_;
            path_matrix_->dismiss();
            search_visible_ = false;
            refs_.search_surface->set_visible(false);
            refs_.folder_surface->set_visible(true);
            refs_.criteria_surface->set_visible(false);
            refs_.objects->set_visible(true);
            navigation_->restore_projection();
            if (Window* window = refs_.shell->attached_window()) {
                window->perform_layout();
                static_cast<void>(window->request_focus(refs_.objects));
            }
            feedback_(SemanticFeedbackKind::pane_changed);
            return true;
        }
        if (normalized != "search") return false;
        const std::string query(refs_.search->text().empty()
            ? FixtureCatalogue::instance().search_query()
            : refs_.search->text());
        show_search(query, true);
        return true;
    }

    [[nodiscard]] bool search_visible() const noexcept { return search_visible_; }
    [[nodiscard]] bool non_folder_surface_visible() const noexcept {
        return search_visible_ || refs_.criteria_surface->visible();
    }
    void deactivate_for_other_surface() {
        pending_query_.disconnect();
        ++query_generation_;
        search_visible_ = false;
        refs_.search_surface->set_visible(false);
    }
    [[nodiscard]] std::uint64_t query_generation() const noexcept {
        return query_generation_;
    }

private:
    [[nodiscard]] const FixtureSearchResult* fixture_for(
        std::string_view stable_id) const noexcept {
        constexpr std::string_view prefix = "fm.result.";
        const std::string_view fixture_id = stable_id.starts_with(prefix)
            ? stable_id.substr(prefix.size()) : stable_id;
        const auto fixtures = FixtureCatalogue::instance().search_results();
        const auto found = std::find_if(fixtures.begin(), fixtures.end(),
            [fixture_id](const FixtureSearchResult& fixture) {
                return fixture.id == fixture_id;
            });
        return found == fixtures.end() ? nullptr : &*found;
    }

    [[nodiscard]] std::string title_for(std::string_view stable_id) const {
        if (const FixtureSearchResult* fixture = fixture_for(stable_id)) {
            return fixture->name;
        }
        return std::string(stable_id);
    }

    [[nodiscard]] std::string search_status() const {
        return "5 of 7 results shown · 1 unavailable source";
    }

    void schedule_query(std::string query) {
        pending_query_.disconnect();
        const std::uint64_t generation = ++query_generation_;
        if (query.empty()) {
            static_cast<void>(select_surface("folder"));
            return;
        }
        Window* window = refs_.search->attached_window();
        if (!window) return;
        pending_query_ = window->schedule_ui_timer(
            *this, std::chrono::hours(24),
            FrameClock::now() + std::chrono::milliseconds(160),
            [this, generation, query = std::move(query)](FrameTime) {
                pending_query_.disconnect();
                if (generation != query_generation_) return;
                show_search(query, false);
            });
        status_("Search pending · deterministic debounce generation " +
                std::to_string(generation));
    }

    void show_search(const std::string& query, bool immediate) {
        pending_query_.disconnect();
        if (immediate) ++query_generation_;
        path_matrix_->dismiss();
        applying_ = true;
        if (refs_.search->text() != query) refs_.search->set_text(query);
        applying_ = false;
        search_visible_ = true;
        refs_.folder_surface->set_visible(false);
        refs_.search_surface->set_visible(true);
        refs_.title->set_text("File Manager  ·  Search");
        refs_.status_summary->set_text(search_status());
        refs_.status_authority->set_text(
            "Local navigation ready · result generation 86");
        if (Window* window = refs_.shell->attached_window()) {
            window->perform_layout();
            static_cast<void>(window->request_focus(refs_.search_results));
        }
        feedback_(SemanticFeedbackKind::pane_changed);
    }

    void activate(std::string_view stable_id, bool select_object) {
        const FixtureSearchResult* fixture = fixture_for(stable_id);
        if (!fixture) return;
        if (!fixture->available) {
            status_("Cannot open · " + fixture->name +
                    " · source volume unavailable");
            feedback_(SemanticFeedbackKind::operation_failed);
            return;
        }
        const std::string destination = fixture->destination_id;
        const std::string object_id = fixture->object_id;
        static_cast<void>(select_surface("folder"));
        navigation_->navigate(destination);
        if (select_object && !object_id.empty()) {
            const std::string stable_object = "fm.object." + object_id;
            if (std::any_of(refs_.objects->items().begin(), refs_.objects->items().end(),
                            [&stable_object](const ObjectViewItem& item) {
                                return item.stable_id == stable_object;
                            })) {
                refs_.objects->set_selected_id(stable_object);
            }
        }
        status_(select_object ? "Opened search result · " + fixture->name
                              : "Opened containing location · " + fixture->location);
        feedback_(SemanticFeedbackKind::location_changed);
    }

    void build_context_menu() {
        open_ = std::make_shared<Command>("search.open", "Open");
        open_->set_description(
            "Open the selected result through deterministic fixture navigation");
        containing_ = std::make_shared<Command>(
            "search.open_containing", "Open containing location");
        containing_->set_description(
            "Return to Folder and navigate to the result location");
        copy_path_ = std::make_shared<Command>("search.copy_path", "Copy path");
        copy_path_->set_description(
            "Copy a deterministic fixture path description");
        properties_ = std::make_shared<Command>(
            "search.properties", "Properties");
        properties_->set_description(
            "Keep the evidence row pinned for retained inspection");
        command_tokens_.push_back(open_->invoked().subscribe(
            *this, [this](const CommandInvocation&) { activate(context_id_, true); }));
        command_tokens_.push_back(containing_->invoked().subscribe(
            *this, [this](const CommandInvocation&) { activate(context_id_, false); }));
        command_tokens_.push_back(copy_path_->invoked().subscribe(
            *this, [this](const CommandInvocation&) {
                if (const FixtureSearchResult* fixture = fixture_for(context_id_)) {
                    status_("Fixture path copied · " + fixture->location +
                            " / " + fixture->name);
                }
            }));
        command_tokens_.push_back(properties_->invoked().subscribe(
            *this, [this](const CommandInvocation&) {
                if (!context_id_.empty()) {
                    refs_.search_results->set_selected_id(context_id_);
                    refs_.search_results->set_pinned_id(context_id_);
                    status_("Search evidence pinned · " + title_for(context_id_));
                }
            }));
        context_menu_ = std::make_shared<ContextMenu>("fm.search.context");
        context_menu_->set_items({
            {"open", MenuItemKind::command, open_},
            {"open_containing", MenuItemKind::command, containing_},
            {"separator", MenuItemKind::separator},
            {"copy_path", MenuItemKind::command, copy_path_},
            {"properties", MenuItemKind::command, properties_},
        });
    }

    void update_context_availability(std::string_view stable_id) {
        const FixtureSearchResult* fixture = fixture_for(stable_id);
        const bool present = fixture != nullptr;
        const bool available = present && fixture->available;
        if (open_) {
            open_->set_enabled(available);
            open_->set_availability_reason(available ? std::string{}
                : "The source volume is unavailable");
        }
        if (containing_) containing_->set_enabled(present);
        if (copy_path_) copy_path_->set_enabled(present);
        if (properties_) properties_->set_enabled(present);
    }

    ProductRefs refs_;
    std::shared_ptr<NavigationSession> navigation_;
    std::shared_ptr<PathMatrixController> path_matrix_;
    std::function<void(std::string)> status_;
    std::function<void(SemanticFeedbackKind)> feedback_;
    std::shared_ptr<ContextMenu> context_menu_;
    std::shared_ptr<Command> open_;
    std::shared_ptr<Command> containing_;
    std::shared_ptr<Command> copy_path_;
    std::shared_ptr<Command> properties_;
    std::vector<SubscriptionToken> command_tokens_;
    SubscriptionToken text_changed_;
    SubscriptionToken committed_;
    SubscriptionToken cancelled_;
    SubscriptionToken selected_;
    SubscriptionToken pinned_;
    SubscriptionToken expanded_;
    SubscriptionToken activated_;
    SubscriptionToken context_requested_;
    FrameRequestToken pending_query_;
    std::string context_id_;
    std::uint64_t query_generation_{};
    bool applying_{};
    bool search_visible_{};
};

class CriteriaController final : public Component {
public:
    CriteriaController(ProductRefs refs,
                       std::shared_ptr<NavigationSession> navigation,
                       std::shared_ptr<PathMatrixController> path_matrix,
                       std::shared_ptr<SearchController> search,
                       std::function<void(std::string)> status,
                       std::function<void(SemanticFeedbackKind)> feedback)
        : refs_(std::move(refs)), navigation_(std::move(navigation)),
          path_matrix_(std::move(path_matrix)), search_(std::move(search)),
          status_(std::move(status)), feedback_(std::move(feedback)),
          modules_(refs_.criteria_rack->modules()) {
        const auto fixtures = FixtureCatalogue::instance().criteria_modules();
        for (std::size_t index = 0; index < fixtures.size() &&
                                    index < modules_.size(); ++index) {
            expensive_[modules_[index].stable_id] = fixtures[index].expensive;
        }
        toggled_ = refs_.criteria_rack->module_toggled().subscribe(
            *this, [this](const InstrumentModuleToggle& change) {
                adopt_rack_model();
                changed(change.module_id);
            });
        field_committed_ = refs_.criteria_rack->field_committed().subscribe(
            *this, [this](const InstrumentFieldChange& change) {
                adopt_rack_model();
                const auto module = find_module(change.module_id);
                if (module == modules_.end()) return;
                const auto field = std::find_if(
                    module->fields.begin(), module->fields.end(),
                    [&change](const InstrumentFieldSpec& candidate) {
                        return candidate.stable_id == change.field_id;
                    });
                if (field != module->fields.end() && field->required &&
                    field->value.empty()) {
                    module->state = InstrumentModuleState::invalid;
                    module->status_text = field->name + " is required";
                    static_cast<void>(refs_.criteria_rack->set_field_validation(
                        module->stable_id, field->stable_id,
                        module->status_text));
                    static_cast<void>(refs_.criteria_rack->set_module_state(
                        module->stable_id, module->state, module->status_text));
                    update_actions();
                    status_("Criterion rejected · " + module->status_text);
                    feedback_(SemanticFeedbackKind::operation_failed);
                    return;
                }
                if (field != module->fields.end()) {
                    static_cast<void>(refs_.criteria_rack->set_field_validation(
                        module->stable_id, field->stable_id, {}));
                }
                changed(change.module_id);
            });
        remove_ = refs_.criteria_rack->remove_requested().subscribe(
            *this, [this](const InstrumentModuleRequest& request) {
                remove_module(request.module_id);
            });
        moved_ = refs_.criteria_rack->move_requested().subscribe(
            *this, [this](const InstrumentModuleMoveRequest& request) {
                if (request.previous_index >= modules_.size() ||
                    request.requested_index >= modules_.size()) return;
                InstrumentModuleSpec moved =
                    std::move(modules_[request.previous_index]);
                modules_.erase(modules_.begin() +
                               static_cast<std::ptrdiff_t>(request.previous_index));
                modules_.insert(modules_.begin() +
                                static_cast<std::ptrdiff_t>(request.requested_index),
                                std::move(moved));
                refs_.criteria_rack->set_modules(modules_);
                status_("Criterion reordered · " + request.module_id);
                feedback_(SemanticFeedbackKind::option_committed);
            });
        add_clicked_ = refs_.criteria_add->clicked().subscribe(
            *this, [this](ButtonBase&) { show_add_menu(); });
        apply_clicked_ = refs_.criteria_apply->clicked().subscribe(
            *this, [this](ButtonBase&) { apply_staged(); });
        selection_ = refs_.criteria_objects->selection_changed().subscribe(
            *this, [this](const ObjectSelectionChange&) {
                project_selection();
            });
        build_add_menu();
        update_actions();
    }

    bool show() {
        path_matrix_->dismiss();
        search_->deactivate_for_other_surface();
        refs_.search_surface->set_visible(false);
        refs_.folder_surface->set_visible(true);
        refs_.objects->set_visible(false);
        refs_.criteria_surface->set_visible(true);
        refs_.title->set_text("File Manager  ·  Criteria");
        refs_.search->set_placeholder_text("Search this virtual folder");
        update_actions();
        update_status();
        refs_.status_authority->set_text(
            "Local navigation ready · index " + std::to_string(generation_) +
            " current");
        project_selection();
        if (Window* window = refs_.shell->attached_window()) {
            window->perform_layout();
            Control::Ptr focus;
            if (!modules_.empty() && !modules_.front().fields.empty()) {
                focus = refs_.criteria_rack->field_editor(
                    modules_.front().stable_id,
                    modules_.front().fields.front().stable_id);
            }
            static_cast<void>(window->request_focus(
                focus ? focus : std::static_pointer_cast<Control>(
                    refs_.criteria_rack)));
        }
        feedback_(SemanticFeedbackKind::pane_changed);
        return true;
    }

    [[nodiscard]] bool visible() const noexcept {
        return refs_.criteria_surface->visible();
    }

private:
    using ModuleIterator = std::vector<InstrumentModuleSpec>::iterator;

    ModuleIterator find_module(std::string_view id) {
        return std::find_if(modules_.begin(), modules_.end(),
            [id](const InstrumentModuleSpec& module) {
                return module.stable_id == id;
            });
    }

    void adopt_rack_model() {
        modules_ = refs_.criteria_rack->modules();
    }

    [[nodiscard]] bool is_expensive(std::string_view id) const {
        const auto found = expensive_.find(std::string(id));
        return found != expensive_.end() && found->second;
    }

    [[nodiscard]] std::size_t staged_count() const {
        return static_cast<std::size_t>(std::count_if(
            modules_.begin(), modules_.end(), [](const InstrumentModuleSpec& module) {
                return module.state == InstrumentModuleState::staged;
            }));
    }

    [[nodiscard]] std::size_t invalid_count() const {
        return static_cast<std::size_t>(std::count_if(
            modules_.begin(), modules_.end(), [](const InstrumentModuleSpec& module) {
                return module.state == InstrumentModuleState::invalid;
            }));
    }

    [[nodiscard]] std::size_t enabled_count() const {
        return static_cast<std::size_t>(std::count_if(
            modules_.begin(), modules_.end(), [](const InstrumentModuleSpec& module) {
                return module.enabled;
            }));
    }

    [[nodiscard]] std::size_t live_count() const {
        return static_cast<std::size_t>(std::count_if(
            modules_.begin(), modules_.end(), [](const InstrumentModuleSpec& module) {
                return module.enabled &&
                       module.state == InstrumentModuleState::live;
            }));
    }

    void changed(std::string_view module_id) {
        auto module = find_module(module_id);
        if (module == modules_.end()) return;
        if (is_expensive(module_id)) {
            module->state = InstrumentModuleState::staged;
            module->status_text = "Expensive · staged until Apply";
            static_cast<void>(refs_.criteria_rack->set_module_state(
                module->stable_id, module->state, module->status_text));
            status_("Expensive criterion staged · Apply explicitly to evaluate");
        } else {
            module->state = InstrumentModuleState::live;
            module->status_text = "Live · inexpensive";
            static_cast<void>(refs_.criteria_rack->set_module_state(
                module->stable_id, module->state, module->status_text));
            update_live_projection();
            status_("Live criterion committed · deterministic local projection updated");
        }
        update_actions();
        feedback_(SemanticFeedbackKind::option_committed);
    }

    void update_live_projection() {
        const std::size_t enabled_cheap = static_cast<std::size_t>(std::count_if(
            modules_.begin(), modules_.end(), [this](const InstrumentModuleSpec& module) {
                return module.enabled && !is_expensive(module.stable_id);
            }));
        const std::size_t limit = enabled_cheap >= 2U ? 31U
                                : enabled_cheap == 1U ? 18U : 7U;
        const std::string selected(refs_.criteria_objects->selected_id());
        refs_.criteria_objects->set_items(criteria_object_items(limit));
        const bool survives = std::any_of(
            refs_.criteria_objects->items().begin(),
            refs_.criteria_objects->items().end(),
            [&selected](const ObjectViewItem& item) {
                return item.stable_id == selected;
            });
        if (survives) refs_.criteria_objects->set_selected_id(selected);
        else if (!refs_.criteria_objects->items().empty()) {
            refs_.criteria_objects->set_selected_id(
                refs_.criteria_objects->items().front().stable_id);
        }
        ++generation_;
        update_status();
        refs_.status_authority->set_text(
            "Local navigation ready · index " + std::to_string(generation_) +
            " current");
    }

    void update_status() {
        const std::size_t staged = staged_count();
        const std::size_t shown = refs_.criteria_objects->items().size();
        refs_.criteria_results_summary->set_text(
            std::to_string(shown) +
            " OBJECTS   ·   FROM PROJECTS AND DESCENDANTS   ·   ONE INSPECTABLE VIRTUAL FOLDER");
        refs_.status_summary->set_text(
            std::to_string(shown) + " objects · " +
            std::to_string(live_count()) + " live criteria" +
            (staged == 0U ? std::string{}
                          : " · " + std::to_string(staged) +
                            (staged == 1U ? " expensive criterion staged"
                                          : " expensive criteria staged")));
    }

    void update_actions() {
        const std::size_t staged = staged_count();
        const std::size_t invalid = invalid_count();
        refs_.criteria_title->set_text(
            "PROJECTS   ·   VIRTUAL FOLDER   ·   " +
            std::to_string(live_count()) + " LIVE MODULES   ·   " +
            (pending_ ? std::string("APPLYING")
                      : std::to_string(staged) + " STAGED"));
        refs_.criteria_apply->set_enabled(!pending_ && staged > 0U && invalid == 0U);
        refs_.criteria_apply->set_text(staged == 0U ? "Applied"
            : "Apply " + std::to_string(staged));
        if (pending_) {
            refs_.criteria_action_state->set_text("Applying staged criteria");
        } else if (invalid > 0U) {
            refs_.criteria_action_state->set_text(
                std::to_string(invalid) + " invalid criterion");
        } else if (staged > 0U) {
            refs_.criteria_action_state->set_text(
                std::to_string(staged) + " expensive change staged");
        } else {
            refs_.criteria_action_state->set_text("All criteria live");
        }
    }

    void remove_module(std::string_view module_id) {
        const auto module = find_module(module_id);
        if (module == modules_.end()) return;
        const std::string name = module->name;
        expensive_.erase(module->stable_id);
        modules_.erase(module);
        refs_.criteria_rack->set_modules(modules_);
        update_live_projection();
        update_actions();
        status_("Criterion removed · " + name +
                " · focus transferred within the predicate rack");
        feedback_(SemanticFeedbackKind::option_committed);
    }

    void build_add_menu() {
        add_menu_ = std::make_shared<ContextMenu>("fm.criteria.add.menu");
        std::vector<MenuItemSpec> items;
        const auto fixtures = FixtureCatalogue::instance().criterion_templates();
        for (std::size_t index = 0; index < fixtures.size(); ++index) {
            auto command = std::make_shared<Command>(
                "criteria.add." + fixtures[index].id, fixtures[index].name);
            command->set_description(fixtures[index].expensive
                ? "Add an explicitly staged expensive criterion"
                : "Add an inexpensive live criterion");
            add_tokens_.push_back(command->invoked().subscribe(
                *this, [this, index](const CommandInvocation&) {
                    add_template(index);
                }));
            items.push_back({"add." + fixtures[index].id,
                             MenuItemKind::command, command});
            add_commands_.push_back(std::move(command));
        }
        add_menu_->set_items(std::move(items));
    }

    void show_add_menu() {
        const Rect bounds = refs_.criteria_add->absolute_bounds();
        add_menu_->show(refs_.criteria_add,
                        {bounds.x, bounds.y + bounds.height});
    }

    void project_selection() {
        if (!visible()) return;
        const std::string selected(refs_.criteria_objects->selected_id());
        constexpr std::string_view prefix = "fm.object.";
        const std::string_view fixture_id = selected.starts_with(prefix)
            ? std::string_view(selected).substr(prefix.size())
            : std::string_view(selected);
        const auto fixtures = FixtureCatalogue::instance().criteria_objects();
        const auto fixture = std::find_if(
            fixtures.begin(), fixtures.end(), [fixture_id](const FixtureObject& item) {
                return item.id == fixture_id;
            });
        if (fixture == fixtures.end()) {
            refs_.selection_name->set_text("No selection");
            return;
        }
        refs_.selection_name->set_text(fixture->name);
        static_cast<void>(refs_.properties->set_value(
            "fm.property.kind", fixture->kind));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.location", "Projects virtual folder"));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.size", fixture->size));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.dimensions", "fixture dimensions"));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.profile", "criteria generation " +
            std::to_string(generation_)));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.created", "August 2026"));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.name", fixture->name));
        static_cast<void>(refs_.properties->set_value(
            "fm.property.handler", "Preview"));
    }

    void add_template(std::size_t index) {
        const auto fixtures = FixtureCatalogue::instance().criterion_templates();
        if (index >= fixtures.size()) return;
        const FixtureCriterionModule& fixture = fixtures[index];
        const std::string id = "fm.criteria.module.session-" +
                               std::to_string(session_id_++);
        InstrumentModuleSpec module = criterion_module(fixture, id);
        module.state = fixture.expensive ? InstrumentModuleState::staged
                                         : InstrumentModuleState::live;
        module.status_text = fixture.expensive
            ? "Expensive · staged until Apply" : "Live · inexpensive";
        expensive_[id] = fixture.expensive;
        modules_.push_back(std::move(module));
        refs_.criteria_rack->set_modules(modules_);
        if (!fixture.expensive) update_live_projection();
        update_actions();
        status_(fixture.expensive
            ? "Criterion added · expensive evaluation staged"
            : "Criterion added · live projection updated");
        feedback_(SemanticFeedbackKind::option_committed);
    }

    void apply_staged() {
        if (pending_ || staged_count() == 0U || invalid_count() != 0U) return;
        pending_ = true;
        for (InstrumentModuleSpec& module : modules_) {
            if (module.state != InstrumentModuleState::staged) continue;
            module.state = InstrumentModuleState::pending;
            module.status_text = "Evaluating · deterministic fixture";
            static_cast<void>(refs_.criteria_rack->set_module_state(
                module.stable_id, module.state, module.status_text));
        }
        refs_.criteria_progress->set_value(0.0);
        refs_.criteria_progress->set_animation_enabled(true);
        update_actions();
        refs_.status_summary->set_text(
            "Applying staged criteria · selection remains stable");
        status_("Criteria evaluation started · deterministic local fixture");
        feedback_(SemanticFeedbackKind::operation_started);
        Window* window = refs_.criteria_apply->attached_window();
        if (!window) return;
        pending_apply_ = window->schedule_ui_timer(
            *this, std::chrono::milliseconds(90),
            FrameClock::now() + std::chrono::milliseconds(90),
            [this](FrameTime) {
                progress_ = std::min(100, progress_ + 25);
                refs_.criteria_progress->set_value(
                    static_cast<double>(progress_));
                if (progress_ >= 100) finish_apply();
            });
    }

    void finish_apply() {
        pending_apply_.disconnect();
        pending_ = false;
        progress_ = 0;
        for (InstrumentModuleSpec& module : modules_) {
            if (module.state != InstrumentModuleState::pending) continue;
            module.state = InstrumentModuleState::live;
            module.status_text = "Applied · indexed fixture";
            static_cast<void>(refs_.criteria_rack->set_module_state(
                module.stable_id, module.state, module.status_text));
        }
        const std::string selected(refs_.criteria_objects->selected_id());
        refs_.criteria_objects->set_items(criteria_object_items(5U));
        if (std::any_of(refs_.criteria_objects->items().begin(),
                        refs_.criteria_objects->items().end(),
                        [&selected](const ObjectViewItem& item) {
                            return item.stable_id == selected;
                        })) {
            refs_.criteria_objects->set_selected_id(selected);
        } else if (!refs_.criteria_objects->items().empty()) {
            refs_.criteria_objects->set_selected_id(
                refs_.criteria_objects->items().front().stable_id);
        }
        ++generation_;
        refs_.criteria_progress->set_animation_enabled(false);
        update_actions();
        update_status();
        refs_.status_summary->set_text(
            "5 objects · " + std::to_string(enabled_count()) +
            " criteria applied");
        status_("Criteria evaluation complete · deterministic generation " +
                std::to_string(generation_));
        refs_.status_authority->set_text(
            "Local navigation ready · index " + std::to_string(generation_) +
            " current");
        feedback_(SemanticFeedbackKind::operation_completed);
    }

    ProductRefs refs_;
    std::shared_ptr<NavigationSession> navigation_;
    std::shared_ptr<PathMatrixController> path_matrix_;
    std::shared_ptr<SearchController> search_;
    std::function<void(std::string)> status_;
    std::function<void(SemanticFeedbackKind)> feedback_;
    std::vector<InstrumentModuleSpec> modules_;
    std::unordered_map<std::string, bool> expensive_;
    std::shared_ptr<ContextMenu> add_menu_;
    std::vector<std::shared_ptr<Command>> add_commands_;
    std::vector<SubscriptionToken> add_tokens_;
    SubscriptionToken toggled_;
    SubscriptionToken field_committed_;
    SubscriptionToken remove_;
    SubscriptionToken moved_;
    SubscriptionToken add_clicked_;
    SubscriptionToken apply_clicked_;
    SubscriptionToken selection_;
    FrameRequestToken pending_apply_;
    std::uint64_t session_id_{1U};
    std::uint64_t generation_{86U};
    int progress_{};
    bool pending_{};
};

std::shared_ptr<ProductLifetime> wire_product(const ProductRefs& refs) {
    auto lifetime = std::make_shared<ProductLifetime>();
    if (Window* product_window = refs.shell->attached_window()) {
        lifetime->feedback = std::make_shared<SemanticFeedback>(*product_window);
    }
    const auto emit_feedback =
        [feedback = std::weak_ptr<SemanticFeedback>(lifetime->feedback)](
            SemanticFeedbackKind kind) {
            if (const auto channel = feedback.lock()) {
                static_cast<void>(channel->emit(kind));
            }
        };

    auto view = std::make_shared<Command>("view.mode", "Icons  ▼");
    lifetime->commands.push_back(view);
    lifetime->bindings.emplace_back(view, refs.ribbon_view);
    lifetime->bindings.emplace_back(view, refs.status_view);
    lifetime->subscriptions.push_back(view->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects),
         command = std::weak_ptr<Command>(view), emit_feedback](
            const CommandInvocation&) {
            const auto field = objects.lock();
            const auto authority = command.lock();
            if (!field || !authority) return;
            const ObjectViewMode next = field->view_mode() == ObjectViewMode::icons
                ? ObjectViewMode::details : ObjectViewMode::icons;
            field->set_view_mode(next);
            authority->set_text(next == ObjectViewMode::icons ? "Icons  ▼"
                                                               : "Details  ▼");
            emit_feedback(SemanticFeedbackKind::option_committed);
        }));

    auto sort = std::make_shared<Command>("view.sort_name", "Sort A→Z  ▼");
    lifetime->commands.push_back(sort);
    lifetime->bindings.emplace_back(sort, refs.ribbon_sort);
    auto descending = std::make_shared<bool>(false);
    lifetime->subscriptions.push_back(sort->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects),
         command = std::weak_ptr<Command>(sort), descending, emit_feedback](
            const CommandInvocation&) {
            const auto field = objects.lock();
            const auto authority = command.lock();
            if (!field || !authority) return;
            const bool use_descending = *descending;
            std::vector<ObjectViewItem> items(field->items().begin(), field->items().end());
            std::stable_sort(items.begin(), items.end(), [use_descending](const auto& left,
                                                                          const auto& right) {
                const std::string a = lower_ascii(left.name);
                const std::string b = lower_ascii(right.name);
                return use_descending ? a > b : a < b;
            });
            field->set_items(std::move(items));
            *descending = !use_descending;
            authority->set_text(*descending ? "Sort Z→A  ▼" : "Sort A→Z  ▼");
            emit_feedback(SemanticFeedbackKind::option_committed);
        }));

    const auto make_command = [&lifetime](std::string id, std::string text,
                                          std::string description = {}) {
        auto command = std::make_shared<Command>(std::move(id), std::move(text));
        command->set_description(std::move(description));
        lifetime->commands.push_back(command);
        return command;
    };
    auto open = make_command("selection.open", "Open",
                             "Open the selected fixture without touching disk");
    open->set_shortcut("Enter");
    open->set_default_action(true);
    auto open_preview = make_command("selection.open_with.preview", "Preview",
                                     "Choose the fixture Preview handler");
    open_preview->set_checked(true);
    auto open_image_lab = make_command(
        "selection.open_with.image_lab", "Image Laboratory",
        "Choose the fixture Image Laboratory handler");
    auto cut = make_command("selection.cut", "Cut",
                            "Mark fixture references for a later fake move");
    cut->set_shortcut("Ctrl+X");
    auto copy = make_command("selection.copy", "Copy",
                             "Copy deterministic fixture references");
    copy->set_shortcut("Ctrl+C");
    auto move_orchard = make_command(
        "selection.move_copy.orchard", "Orchard Study",
        "Queue a same-volume fixture copy to Orchard Study");
    auto move_reference = make_command(
        "selection.move_copy.reference", "Reference",
        "Queue a same-volume fixture copy to Reference");
    auto rename = make_command("selection.rename", "Rename",
                               "Begin session-only inline label editing");
    rename->set_shortcut("F2");
    auto erase = make_command("selection.delete", "Delete",
                              "Hide selected fixtures until reset or restart");
    erase->set_shortcut("Delete");
    erase->set_destructive(true);
    auto properties = make_command(
        "view.properties", "Properties",
        "Reveal the retained Selection property projection");
    auto toggle_selection = make_command(
        "view.selection_pane", "Selection pane",
        "Show or hide the retained Selection inspector");
    toggle_selection->set_checked(true);
    auto folder_properties = make_command(
        "view.folder_properties", "Properties",
        "Inspect deterministic current-location fixture properties");
    auto new_folder = make_command(
        "selection.new_folder", "New folder",
        "Create a deterministic session-only folder fixture");
    auto paste = make_command("selection.paste", "Paste",
                              "Paste fixture references from the test clipboard");
    paste->set_shortcut("Ctrl+V");
    paste->set_enabled(false);
    paste->set_availability_reason("The fixture clipboard is empty");
    auto select_all = make_command("selection.select_all", "Select all",
                                   "Select every object in the current location");
    select_all->set_shortcut("Ctrl+A");
    auto nav_back = make_command("nav.back", "Back",
                                 "Return to the previous retained location state");
    nav_back->set_shortcut("Alt+Left");
    nav_back->set_enabled(false);
    auto nav_forward = make_command("nav.forward", "Forward",
                                    "Advance to the next retained location state");
    nav_forward->set_shortcut("Alt+Right");
    nav_forward->set_enabled(false);
    auto nav_up = make_command("nav.up", "Up",
                               "Navigate to the parent location");
    nav_up->set_shortcut("Alt+Up");
    auto about = make_command("help.about_fixture", "About this fixture",
                              "Explain the native deterministic dogfood surface");

    lifetime->bindings.emplace_back(nav_back, refs.nav_back,
                                    CommandBindingOptions{false, true, true});
    lifetime->bindings.emplace_back(nav_forward, refs.nav_forward,
                                    CommandBindingOptions{false, true, true});
    lifetime->bindings.emplace_back(nav_up, refs.nav_up,
                                    CommandBindingOptions{false, true, true});
    lifetime->bindings.emplace_back(erase, refs.ribbon_delete,
                                    CommandBindingOptions{false, true, true});
    lifetime->bindings.emplace_back(toggle_selection, refs.ribbon_properties,
                                    CommandBindingOptions{false, true, true});
    lifetime->bindings.emplace_back(move_reference, refs.ribbon_move_copy,
                                    CommandBindingOptions{false, true, true});

    refs.menu_strip->set_items({
        {"fm.menu.file", "File", {
            {"file.new_folder", MenuItemKind::command, new_folder},
            {"file.open", MenuItemKind::command, open},
            {"file.separator.properties", MenuItemKind::separator},
            {"file.properties", MenuItemKind::command, folder_properties},
        }},
        {"fm.menu.home", "Home", {
            {"home.open", MenuItemKind::command, open},
            {"home.separator.clipboard", MenuItemKind::separator},
            {"home.cut", MenuItemKind::command, cut},
            {"home.copy", MenuItemKind::command, copy},
            {"home.move_copy", MenuItemKind::submenu, {}, "Move / copy", {
                {"home.move.orchard", MenuItemKind::command, move_orchard},
                {"home.move.reference", MenuItemKind::command, move_reference},
            }},
            {"home.separator.edit", MenuItemKind::separator},
            {"home.delete", MenuItemKind::command, erase},
            {"home.rename", MenuItemKind::command, rename},
            {"home.properties", MenuItemKind::command, properties},
        }},
        {"fm.menu.edit", "Edit", {
            {"edit.select_all", MenuItemKind::command, select_all},
            {"edit.separator.clipboard", MenuItemKind::separator},
            {"edit.cut", MenuItemKind::command, cut},
            {"edit.copy", MenuItemKind::command, copy},
            {"edit.paste", MenuItemKind::command, paste},
        }},
        {"fm.menu.view", "View", {
            {"view.mode", MenuItemKind::command, view},
            {"view.sort", MenuItemKind::command, sort},
            {"view.separator.properties", MenuItemKind::separator},
            {"view.properties", MenuItemKind::check, toggle_selection},
        }},
        {"fm.menu.go", "Go", {
            {"go.back", MenuItemKind::command, nav_back},
            {"go.forward", MenuItemKind::command, nav_forward},
            {"go.up", MenuItemKind::command, nav_up},
        }},
        {"fm.menu.commands", "Commands", {
            {"commands.new_folder", MenuItemKind::command, new_folder},
            {"commands.select_all", MenuItemKind::command, select_all},
            {"commands.separator.properties", MenuItemKind::separator},
            {"commands.folder_properties", MenuItemKind::command,
             folder_properties},
        }},
        {"fm.menu.help", "Help", {
            {"help.about", MenuItemKind::command, about},
        }},
    });

    auto object_menu = std::make_shared<ContextMenu>("fm.context.object");
    object_menu->set_items({
        {"open", MenuItemKind::command, open},
        {"open_with", MenuItemKind::submenu, {}, "Open with", {
            {"open_with.preview", MenuItemKind::radio, open_preview},
            {"open_with.image_lab", MenuItemKind::radio, open_image_lab},
        }},
        {"separator.clipboard", MenuItemKind::separator},
        {"cut", MenuItemKind::command, cut},
        {"copy", MenuItemKind::command, copy},
        {"move_copy", MenuItemKind::submenu, {}, "Move / copy", {
            {"move_copy.orchard", MenuItemKind::command, move_orchard},
            {"move_copy.reference", MenuItemKind::command, move_reference},
        }},
        {"separator.edit", MenuItemKind::separator},
        {"rename", MenuItemKind::command, rename},
        {"delete", MenuItemKind::command, erase},
        {"separator.properties", MenuItemKind::separator},
        {"properties", MenuItemKind::command, properties},
    });
    auto background_menu = std::make_shared<ContextMenu>("fm.context.background");
    background_menu->set_items({
        {"new_folder", MenuItemKind::command, new_folder},
        {"paste", MenuItemKind::command, paste},
        {"separator.properties", MenuItemKind::separator},
        {"properties", MenuItemKind::command, folder_properties},
    });
    lifetime->menus.push_back(object_menu);
    lifetime->menus.push_back(background_menu);

    const auto status_message = [authority = std::weak_ptr<Label>(refs.status_authority)](
        std::string text) {
        if (const auto label = authority.lock()) label->set_text(std::move(text));
    };
    auto apply_accommodation = std::make_shared<std::function<void(Rect)>>();
    *apply_accommodation =
        [shell = std::weak_ptr<TableLayoutPanel>(refs.shell),
         title = std::weak_ptr<Control>(refs.title_band),
         ribbon = std::weak_ptr<Control>(refs.ribbon_band),
         navigation = std::weak_ptr<Control>(refs.navigation_band),
         status = std::weak_ptr<Control>(refs.status_band),
         workspace_split = std::weak_ptr<SplitContainer>(refs.workspace_split),
         selection_split = std::weak_ptr<SplitContainer>(refs.selection_split)](
            Rect bounds) {
            const auto layout = shell.lock();
            if (!layout) return;
            const double text_scale = layout->effective_text_scale();
            const bool show_title = bounds.height >= 240.0 * text_scale;
            const bool show_ribbon = bounds.width >= 900.0 * text_scale &&
                                     bounds.height >= 430.0 * text_scale;
            const bool show_status = bounds.height >= 220.0 * text_scale;
            std::optional<UpdateScope> update;
            if (Window* window = layout->attached_window()) {
                update.emplace(window->begin_update());
            }
            layout->set_row_style(0, {TableSizeMode::absolute,
                                      show_title ? 40.0 * text_scale : 0.0});
            layout->set_row_style(1, {TableSizeMode::absolute,
                                      23.0 * text_scale});
            layout->set_row_style(2, {TableSizeMode::absolute,
                                      show_ribbon ? 66.0 * text_scale : 0.0});
            layout->set_row_style(3, {TableSizeMode::absolute,
                                      40.0 * text_scale});
            layout->set_row_style(5, {TableSizeMode::absolute,
                                      show_status ? 24.0 * text_scale : 0.0});
            if (const auto band = title.lock()) {
                band->set_requested_bounds({0.0, 0.0, 1450.0,
                                            40.0 * text_scale});
                band->set_visible(show_title);
            }
            if (const auto band = ribbon.lock()) {
                band->set_requested_bounds({0.0, 0.0, 1450.0,
                                            66.0 * text_scale});
                band->set_visible(show_ribbon);
            }
            if (const auto band = navigation.lock()) {
                band->set_requested_bounds({0.0, 0.0, 1450.0,
                                            40.0 * text_scale});
            }
            if (const auto band = status.lock()) {
                band->set_requested_bounds({0.0, 0.0, 1450.0,
                                            24.0 * text_scale});
                band->set_visible(show_status);
            }
            if (const auto split = workspace_split.lock()) {
                split->set_automatic_collapse_threshold(700.0 * text_scale);
            }
            if (const auto split = selection_split.lock()) {
                split->set_automatic_collapse_threshold(900.0 * text_scale);
            }
        };
    lifetime->subscriptions.push_back(refs.shell->arranged_bounds_changed().subscribe(
        [apply_accommodation](Rect bounds) { (*apply_accommodation)(bounds); }));
    if (Window* product_window = refs.shell->attached_window()) {
        lifetime->subscriptions.push_back(
            product_window->presentation_changed().subscribe(
                *refs.shell, [apply_accommodation,
                              shell = std::weak_ptr<TableLayoutPanel>(refs.shell)](
                    const PresentationSettings&) {
                    if (const auto layout = shell.lock()) {
                        (*apply_accommodation)(layout->committed_arranged_bounds());
                    }
                }));
    }
    lifetime->subscriptions.push_back(open->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message](
            const CommandInvocation&) {
            if (const auto field = objects.lock()) {
                status_message("Fixture open · " +
                    std::to_string(field->selected_ids().size()) +
                    " selected · no filesystem action");
            }
        }));
    lifetime->subscriptions.push_back(copy->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message](
            const CommandInvocation&) {
            if (const auto field = objects.lock()) {
                status_message("Fixture references copied · " +
                    std::to_string(field->selected_ids().size()) +
                    " item(s) · session only");
            }
        }));
    lifetime->subscriptions.push_back(cut->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message](
            const CommandInvocation&) {
            if (const auto field = objects.lock()) {
                status_message("Fixture cut marked · " +
                    std::to_string(field->selected_ids().size()) +
                    " item(s) · no filesystem action");
            }
        }));
    for (const auto& destination : {
             std::pair{move_orchard, std::string("Orchard Study")},
             std::pair{move_reference, std::string("Reference")}}) {
        lifetime->subscriptions.push_back(destination.first->invoked().subscribe(
            [target = destination.second, status_message](const CommandInvocation&) {
                status_message("Fixture move / copy queued · destination " + target +
                               " · no filesystem action");
            }));
    }
    lifetime->subscriptions.push_back(erase->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message](
            const CommandInvocation&) {
            const auto field = objects.lock();
            if (!field || field->selected_ids().empty()) return;
            const std::vector<std::string> removed(field->selected_ids().begin(),
                                                   field->selected_ids().end());
            std::vector<ObjectViewItem> remaining;
            remaining.reserve(field->items().size());
            for (const ObjectViewItem& item : field->items()) {
                if (std::find(removed.begin(), removed.end(), item.stable_id) ==
                    removed.end()) {
                    remaining.push_back(item);
                }
            }
            field->set_items(std::move(remaining));
            status_message("Fixture delete committed · " +
                           std::to_string(removed.size()) +
                           " hidden · reset restores");
        }));
    lifetime->subscriptions.push_back(properties->invoked().subscribe(
        [split = std::weak_ptr<SplitContainer>(refs.selection_split),
         property_list = std::weak_ptr<PropertyList>(refs.properties),
         toggle = std::weak_ptr<Command>(toggle_selection), status_message](
            const CommandInvocation&) {
            if (const auto surface = split.lock()) {
                surface->set_second_collapsed(false, SplitCollapseOrigin::user);
            }
            if (const auto command = toggle.lock()) command->set_checked(true);
            if (const auto list = property_list.lock()) {
                if (Window* window = list->attached_window()) {
                    static_cast<void>(window->request_focus(list));
                }
            }
            status_message("Selection properties active · retained fixture projection");
        }));
    lifetime->subscriptions.push_back(toggle_selection->invoked().subscribe(
        [split = std::weak_ptr<SplitContainer>(refs.selection_split),
         command = std::weak_ptr<Command>(toggle_selection), status_message,
         emit_feedback](
            const CommandInvocation&) {
            const auto surface = split.lock();
            const auto authority = command.lock();
            if (!surface || !authority) return;
            const bool show = surface->second_collapsed();
            surface->set_second_collapsed(!show, SplitCollapseOrigin::user);
            authority->set_checked(show);
            status_message(show ? "Selection pane restored · retained extent"
                                : "Selection pane collapsed · use View or Properties to restore");
            emit_feedback(SemanticFeedbackKind::pane_changed);
        }));
    lifetime->subscriptions.push_back(refs.selection_collapse->clicked().subscribe(
        [split = std::weak_ptr<SplitContainer>(refs.selection_split),
         command = std::weak_ptr<Command>(toggle_selection), status_message,
         emit_feedback](
            ButtonBase&) {
            if (const auto surface = split.lock()) {
                surface->set_second_collapsed(true, SplitCollapseOrigin::user);
            }
            if (const auto authority = command.lock()) authority->set_checked(false);
            status_message("Selection pane collapsed · use View or Properties to restore");
            emit_feedback(SemanticFeedbackKind::pane_changed);
        }));
    lifetime->subscriptions.push_back(rename->invoked().subscribe(
        [editor = std::weak_ptr<TextBox>(
             std::dynamic_pointer_cast<TextBox>(
                 refs.properties->editor("fm.property.name"))),
         status_message](const CommandInvocation&) {
            const auto field = editor.lock();
            if (!field || !field->enabled()) return;
            if (Window* window = field->attached_window()) {
                if (window->request_focus(field)) {
                    field->select_all();
                    status_message("Rename active · commit with Enter · cancel with Escape");
                }
            }
        }));
    lifetime->subscriptions.push_back(folder_properties->invoked().subscribe(
        [status_message](const CommandInvocation&) {
            status_message("Projects fixture properties · local catalogue generation 86");
        }));
    if (refs.properties) {
        lifetime->subscriptions.push_back(open_preview->invoked().subscribe(
            [property_list = std::weak_ptr<PropertyList>(refs.properties),
             open_preview = std::weak_ptr<Command>(open_preview),
             open_image_lab = std::weak_ptr<Command>(open_image_lab),
             status_message, emit_feedback](
                const CommandInvocation&) {
                if (const auto list = property_list.lock()) {
                    static_cast<void>(list->set_value("fm.property.handler", "Preview"));
                }
                if (const auto command = open_preview.lock()) command->set_checked(true);
                if (const auto command = open_image_lab.lock()) command->set_checked(false);
                status_message("Fixture handler committed · Preview");
                emit_feedback(SemanticFeedbackKind::option_committed);
            }));
        lifetime->subscriptions.push_back(open_image_lab->invoked().subscribe(
            [property_list = std::weak_ptr<PropertyList>(refs.properties),
             open_preview = std::weak_ptr<Command>(open_preview),
             open_image_lab = std::weak_ptr<Command>(open_image_lab),
             status_message, emit_feedback](
                const CommandInvocation&) {
                if (const auto list = property_list.lock()) {
                    static_cast<void>(list->set_value(
                        "fm.property.handler", "Image Laboratory"));
                }
                if (const auto command = open_preview.lock()) command->set_checked(false);
                if (const auto command = open_image_lab.lock()) command->set_checked(true);
                status_message("Fixture handler committed · Image Laboratory");
                emit_feedback(SemanticFeedbackKind::option_committed);
            }));
    }
    auto session_folder_counter = std::make_shared<std::size_t>(1U);
    lifetime->subscriptions.push_back(new_folder->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message,
         session_folder_counter](const CommandInvocation&) {
            const auto field = objects.lock();
            if (!field) return;
            const std::string suffix = std::to_string((*session_folder_counter)++);
            const std::string id = "fm.object.session-new-folder-" + suffix;
            std::vector<ObjectViewItem> items(field->items().begin(),
                                              field->items().end());
            items.push_back({id, *session_folder_counter == 2U
                                     ? "New Folder" : "New Folder " + suffix,
                             "Folder", "Session-only fixture folder",
                             ObjectGlyph::folder});
            field->set_items(std::move(items));
            field->set_selected_id(id);
            status_message("Fixture folder created · reset or restart restores catalogue");
        }));
    lifetime->subscriptions.push_back(select_all->invoked().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects), status_message](
            const CommandInvocation&) {
            if (const auto field = objects.lock()) {
                field->select_all();
                status_message("All current fixture objects selected");
            }
        }));
    lifetime->subscriptions.push_back(about->invoked().subscribe(
        [status_message](const CommandInvocation&) {
            status_message("Native GUI.Forms dogfood · deterministic local fixture · D2");
        }));

    lifetime->subscriptions.push_back(refs.objects->context_requested().subscribe(
        [object_menu, background_menu, objects = std::weak_ptr<ObjectView>(refs.objects)](
            const ObjectContextRequest& request) {
            if (const auto field = objects.lock()) {
                if (request.stable_id.empty()) {
                    background_menu->show(field, request.screen_position);
                } else {
                    object_menu->show(field, request.screen_position);
                }
            }
        }));

    auto fixture_handlers =
        std::make_shared<std::unordered_map<std::string, std::string>>();
    lifetime->subscriptions.push_back(refs.objects->selection_changed().subscribe(
        [objects_view = std::weak_ptr<ObjectView>(refs.objects),
         name = std::weak_ptr<Label>(refs.selection_name),
         properties = std::weak_ptr<PropertyList>(refs.properties),
         status = std::weak_ptr<Label>(refs.status_summary), fixture_handlers](
            const ObjectSelectionChange& change) {
            const auto set_properties = [&properties](
                std::string kind, std::string location, std::string size,
                std::string dimensions, std::string profile,
                std::string created, std::string name_value,
                std::string handler) {
                const auto list = properties.lock();
                if (!list) return;
                static_cast<void>(list->set_value("fm.property.kind", std::move(kind)));
                static_cast<void>(list->set_value("fm.property.location", std::move(location)));
                static_cast<void>(list->set_value("fm.property.size", std::move(size)));
                static_cast<void>(list->set_value("fm.property.dimensions", std::move(dimensions)));
                static_cast<void>(list->set_value("fm.property.profile", std::move(profile)));
                static_cast<void>(list->set_value("fm.property.created", std::move(created)));
                static_cast<void>(list->set_value("fm.property.name", std::move(name_value)));
                static_cast<void>(list->set_value("fm.property.handler", std::move(handler)));
                static_cast<void>(list->set_validation("fm.property.name", {}));
            };
            if (change.current_ids.empty()) {
                if (const auto label = name.lock()) label->set_text("No selection");
                set_properties("—", "—", "—", "—", "—", "—", "—", "Preview");
                if (const auto summary = status.lock()) {
                    summary->set_text("No selection · 14 objects");
                }
                return;
            }
            if (change.current_ids.size() > 1U) {
                if (const auto label = name.lock()) {
                    label->set_text(std::to_string(change.current_ids.size()) +
                                    " objects selected");
                }
                set_properties("Multiple kinds", "~/Work/Projects",
                    "Multiple values", "—", "—", "Multiple values",
                    "Multiple values", "Preview");
                if (const auto summary = status.lock()) {
                    summary->set_text(std::to_string(change.current_ids.size()) +
                                      " selected · 14 objects");
                }
                return;
            }
            const std::string prefix = "fm.object.";
            const std::string fixture_id = change.current_id.starts_with(prefix)
                ? change.current_id.substr(prefix.size()) : std::string{};
            const auto objects = FixtureCatalogue::instance().project_objects();
            const auto found = std::find_if(objects.begin(), objects.end(),
                [&](const FixtureObject& object) { return object.id == fixture_id; });
            if (found == objects.end()) {
                const auto field = objects_view.lock();
                if (!field) return;
                const auto session_item = std::find_if(
                    field->items().begin(), field->items().end(),
                    [&](const ObjectViewItem& item) {
                        return item.stable_id == change.current_id;
                    });
                if (session_item == field->items().end()) return;
                if (const auto label = name.lock()) label->set_text(session_item->name);
                const std::string kind = session_item->glyph == ObjectGlyph::folder
                    ? "Folder" : session_item->glyph == ObjectGlyph::image
                    ? "Image" : session_item->glyph == ObjectGlyph::audio
                    ? "Audio" : session_item->glyph == ObjectGlyph::code
                    ? "Structured text" : "Document";
                set_properties(kind, "Fixture navigation result",
                    session_item->secondary_text, "—", "fixture metadata",
                    "August 2026", session_item->name, "Preview");
                if (const auto summary = status.lock()) {
                    summary->set_text("1 selected · " + session_item->secondary_text + " · " +
                                      std::to_string(field->items().size()) +
                                      " objects");
                }
                return;
            }
            const auto field = objects_view.lock();
            if (!field) return;
            const auto active_item = std::find_if(
                field->items().begin(), field->items().end(),
                [&](const ObjectViewItem& item) {
                    return item.stable_id == change.current_id;
                });
            const std::string display_name = active_item == field->items().end()
                ? found->name : active_item->name;
            const auto remembered_handler = fixture_handlers->find(change.current_id);
            const std::string handler = remembered_handler == fixture_handlers->end()
                ? (found->kind.find("image") != std::string::npos
                       ? "Preview" : "Fixture viewer")
                : remembered_handler->second;
            if (const auto label = name.lock()) label->set_text(display_name);
            set_properties(found->kind, "~/Work/Projects", found->size,
                found->kind.find("image") != std::string::npos
                    ? "fixture dimensions" : "—",
                "fixture metadata", "August 2026", display_name, handler);
            if (const auto summary = status.lock()) {
                summary->set_text("1 selected · " + found->size + " · 14 objects");
            }
        }));

    lifetime->subscriptions.push_back(refs.objects->selection_changed().subscribe(
        [open = std::weak_ptr<Command>(open),
         open_preview = std::weak_ptr<Command>(open_preview),
         open_image_lab = std::weak_ptr<Command>(open_image_lab),
         cut = std::weak_ptr<Command>(cut),
         copy = std::weak_ptr<Command>(copy),
         erase = std::weak_ptr<Command>(erase),
         properties = std::weak_ptr<Command>(properties),
         rename = std::weak_ptr<Command>(rename),
         move_orchard = std::weak_ptr<Command>(move_orchard),
         move_reference = std::weak_ptr<Command>(move_reference)](
            const ObjectSelectionChange& change) {
            const bool any = !change.current_ids.empty();
            const bool one = change.current_ids.size() == 1U;
            if (const auto command = open.lock()) command->set_enabled(one);
            if (const auto command = open_preview.lock()) command->set_enabled(one);
            if (const auto command = open_image_lab.lock()) command->set_enabled(one);
            if (const auto command = cut.lock()) command->set_enabled(any);
            if (const auto command = copy.lock()) command->set_enabled(any);
            if (const auto command = erase.lock()) command->set_enabled(any);
            if (const auto command = properties.lock()) command->set_enabled(any);
            if (const auto command = rename.lock()) command->set_enabled(one);
            if (const auto command = move_orchard.lock()) command->set_enabled(any);
            if (const auto command = move_reference.lock()) command->set_enabled(any);
        }));

    auto preview_expanded = std::make_shared<bool>(true);
    lifetime->subscriptions.push_back(refs.preview_disclosure->clicked().subscribe(
        [property_list = std::weak_ptr<PropertyList>(refs.properties),
         preview = std::weak_ptr<Control>(refs.preview),
         disclosure = std::weak_ptr<Button>(refs.preview_disclosure),
         status_message, preview_expanded, emit_feedback](ButtonBase&) {
            *preview_expanded = !*preview_expanded;
            if (const auto control = preview.lock()) {
                control->set_visible(*preview_expanded);
            }
            if (const auto list = property_list.lock()) {
                list->set_header_height(*preview_expanded ? 212.0 : 34.0);
            }
            if (const auto button = disclosure.lock()) {
                button->set_text(*preview_expanded ? "▼" : "▶");
                button->set_accessible_description(*preview_expanded
                    ? "Collapse fixture preview" : "Expand fixture preview");
            }
            status_message(*preview_expanded ? "Selection preview expanded"
                                             : "Selection preview collapsed");
            emit_feedback(SemanticFeedbackKind::pane_changed);
        }));

    lifetime->subscriptions.push_back(refs.properties->value_committed().subscribe(
        [objects = std::weak_ptr<ObjectView>(refs.objects),
         name_label = std::weak_ptr<Label>(refs.selection_name),
         property_list = std::weak_ptr<PropertyList>(refs.properties),
         open_preview = std::weak_ptr<Command>(open_preview),
         open_image_lab = std::weak_ptr<Command>(open_image_lab),
         status_message, fixture_handlers](const PropertyValueChange& change) {
            const auto field = objects.lock();
            const auto properties = property_list.lock();
            if (!field || !properties) return;
            if (change.row_id == "fm.property.handler") {
                if (field->selected_ids().size() == 1U) {
                    (*fixture_handlers)[std::string(field->selected_id())] =
                        change.current_value;
                }
                if (const auto command = open_preview.lock()) {
                    command->set_checked(change.current_value == "Preview");
                }
                if (const auto command = open_image_lab.lock()) {
                    command->set_checked(change.current_value ==
                                         "Image Laboratory");
                }
                status_message("Fixture handler committed · " +
                               change.current_value + " · session only");
                return;
            }
            if (change.row_id != "fm.property.name" ||
                field->selected_ids().size() != 1U) return;
            const std::string& candidate = change.current_value;
            const bool separator = candidate.find('/') != std::string::npos ||
                                   candidate.find('\\') != std::string::npos;
            const bool duplicate = std::any_of(
                field->items().begin(), field->items().end(),
                [&](const ObjectViewItem& item) {
                    return item.stable_id != field->selected_id() &&
                           lower_ascii(item.name) == lower_ascii(candidate);
                });
            if (candidate.empty() || separator || duplicate) {
                const std::string message = candidate.empty()
                    ? "Name is required"
                    : separator ? "Name may not contain a path separator"
                                : "That name already exists here";
                static_cast<void>(properties->set_value(
                    "fm.property.name", change.previous_value));
                static_cast<void>(properties->set_validation(
                    "fm.property.name", message));
                status_message("Fixture rename rejected · " + message);
                return;
            }
            std::vector<ObjectViewItem> items(field->items().begin(),
                                               field->items().end());
            const auto selected = std::find_if(items.begin(), items.end(),
                [&](const ObjectViewItem& item) {
                    return item.stable_id == field->selected_id();
                });
            if (selected == items.end()) return;
            selected->name = candidate;
            const std::string selected_id(field->selected_id());
            field->set_items(std::move(items));
            field->set_selected_id(selected_id);
            static_cast<void>(properties->set_validation("fm.property.name", {}));
            if (const auto label = name_label.lock()) label->set_text(candidate);
            status_message("Fixture renamed · " + candidate + " · session only");
        }));

    auto navigation = std::make_shared<NavigationSession>(
        refs, nav_back, nav_forward, nav_up,
        [emit_feedback] {
            emit_feedback(SemanticFeedbackKind::location_changed);
        });
    lifetime->navigation = navigation;
    lifetime->path_matrix = std::make_shared<PathMatrixController>(
        refs, navigation, status_message);
    lifetime->search_controller = std::make_shared<SearchController>(
        refs, navigation, lifetime->path_matrix, status_message, emit_feedback);
    lifetime->criteria_controller = std::make_shared<CriteriaController>(
        refs, navigation, lifetime->path_matrix, lifetime->search_controller,
        status_message, emit_feedback);
    const std::weak_ptr<NavigationSession> weak_navigation = navigation;
    const std::weak_ptr<SearchController> weak_search =
        lifetime->search_controller;
    lifetime->subscriptions.push_back(nav_back->invoked().subscribe(
        [weak_navigation](const CommandInvocation&) {
            if (const auto session = weak_navigation.lock()) session->back();
        }));
    lifetime->subscriptions.push_back(nav_forward->invoked().subscribe(
        [weak_navigation](const CommandInvocation&) {
            if (const auto session = weak_navigation.lock()) session->forward();
        }));
    lifetime->subscriptions.push_back(nav_up->invoked().subscribe(
        [weak_navigation](const CommandInvocation&) {
            if (const auto session = weak_navigation.lock()) session->up();
        }));
    lifetime->subscriptions.push_back(refs.tree->selection_changed().subscribe(
        [weak_navigation, weak_search](const TreeSelectionChange& change) {
            if (const auto search = weak_search.lock()) {
                if (search->non_folder_surface_visible()) {
                    static_cast<void>(search->select_surface("folder"));
                }
            }
            if (const auto session = weak_navigation.lock()) {
                constexpr std::string_view prefix = "fm.tree.node.";
                session->navigate(change.current_id.starts_with(prefix)
                    ? change.current_id.substr(prefix.size()) : change.current_id);
            }
        }));
    lifetime->subscriptions.push_back(refs.objects->item_activated().subscribe(
        [weak_navigation](const std::string& id) {
            constexpr std::string_view prefix = "fm.object.location.";
            if (id.starts_with(prefix)) {
                if (const auto session = weak_navigation.lock()) {
                    session->navigate(id.substr(prefix.size()));
                }
            }
        }));
    for (std::size_t slot = 0; slot < refs.breadcrumbs.size(); ++slot) {
        lifetime->subscriptions.push_back(refs.breadcrumbs[slot]->clicked().subscribe(
            [weak_navigation, slot](ButtonBase&) {
                if (const auto session = weak_navigation.lock()) {
                    session->breadcrumb(slot);
                }
            }));
    }
    if (Window* window = refs.objects->attached_window()) {
        const auto register_command = [window, &lifetime](
            const std::shared_ptr<Command>& command, KeyGesture gesture,
            bool object_scope_only = false, bool preemptive = false) {
            const std::weak_ptr<Command> weak_command = command;
            lifetime->accelerators.push_back(window->register_accelerator(
                *command, gesture,
                [window, weak_command, object_scope_only] {
                    if (object_scope_only) {
                        const Control::Ptr focused = window->focused_control();
                        if (!focused || focused->stable_id().value() !=
                            "fm.folder.objects") return false;
                    }
                    if (const auto authority = weak_command.lock()) {
                        return authority->execute("fm.window.accelerator");
                    }
                    return false;
                }, AcceleratorOptions{preemptive}));
        };
        register_command(nav_back, {PhysicalKey::left, Modifier::alt}, false, true);
        register_command(nav_forward, {PhysicalKey::right, Modifier::alt}, false, true);
        register_command(nav_up, {PhysicalKey::up, Modifier::alt}, false, true);
        register_command(select_all, {PhysicalKey::a, Modifier::control}, true);
        register_command(select_all, {PhysicalKey::a, Modifier::meta}, true);
        register_command(copy, {PhysicalKey::c, Modifier::control}, true);
        register_command(copy, {PhysicalKey::c, Modifier::meta}, true);
        register_command(cut, {PhysicalKey::x, Modifier::control}, true);
        register_command(cut, {PhysicalKey::x, Modifier::meta}, true);
        register_command(paste, {PhysicalKey::v, Modifier::control}, true);
        register_command(paste, {PhysicalKey::v, Modifier::meta}, true);
        register_command(erase, {PhysicalKey::delete_forward, Modifier::none}, true);
        register_command(rename, {PhysicalKey::f2, Modifier::none}, true);
    }
    return lifetime;
}

} // namespace

std::unique_ptr<gui_forms::Window> make_product_window() {
    using namespace gui_forms;
    ProductRefs refs;
    auto root = std::make_shared<TableLayoutPanel>(StableId("fm.window.primary"));
    refs.shell = root;
    root->set_column_count(1);
    root->set_row_count(6);
    root->set_column_style(0, {TableSizeMode::percent, 100});
    root->set_row_style(0, {TableSizeMode::absolute, 40});
    root->set_row_style(1, {TableSizeMode::absolute, 23});
    root->set_row_style(2, {TableSizeMode::absolute, 66});
    root->set_row_style(3, {TableSizeMode::absolute, 40});
    root->set_row_style(4, {TableSizeMode::percent, 100});
    root->set_row_style(5, {TableSizeMode::absolute, 24});
    root->set_grow_style(TableLayoutGrowStyle::fixed_size);

    const Control::Ptr bands[] = {make_title(refs), make_tabs(refs), make_ribbon(refs),
                                  make_navigation(refs), make_workspace(refs), make_status(refs)};
    refs.title_band = bands[0];
    refs.ribbon_band = bands[2];
    refs.navigation_band = bands[3];
    refs.status_band = bands[5];
    constexpr double band_heights[] = {40, 23, 66, 40, 657, 24};
    for (std::size_t row = 0; row < std::size(bands); ++row) {
        bands[row]->set_margin({});
        bands[row]->set_dock(DockStyle::fill);
        bands[row]->set_requested_bounds({0, 0, 1450, band_heights[row]});
        root->add_child(bands[row]);
        root->set_cell_position(*bands[row], {0,row});
    }
    auto window = std::make_unique<Window>(root, Size{1450,850});
    window->perform_layout();
    root->set_tag(wire_product(refs));
    return window;
}

bool apply_capture_state(gui_forms::Window& product, std::string_view state) {
    if (state == "folder") return true;
    if (state == "path-matrix-browse") {
        return product.perform_semantic_action(
            "fm.path.terminal", gui_forms::SemanticAction::press);
    }
    if (state == "path-matrix-editing") {
        return product.perform_semantic_action(
                   "fm.path.terminal", gui_forms::SemanticAction::press) &&
               product.perform_semantic_action(
                   "fm.path.matrix.current.tail",
                   gui_forms::SemanticAction::press);
    }
    if (state == "search-pinned") {
        return set_product_surface(product, "search");
    }
    if (state == "search-offline-expanded") {
        return set_product_surface(product, "search") &&
               product.perform_semantic_action(
                   "fm.result.result-pages-offline",
                   gui_forms::SemanticAction::expand);
    }
    if (state == "criteria-default") {
        return set_product_surface(product, "criteria");
    }
    if (state == "criteria-staged-progress") {
        return set_product_surface(product, "criteria") &&
               product.perform_semantic_action(
                   "fm.criteria.apply", gui_forms::SemanticAction::press);
    }
    return false;
}

bool set_product_surface(gui_forms::Window& product,
                         std::string_view surface) {
    const gui_forms::Control::Ptr root = product.root();
    if (!root) return false;
    const auto lifetime = std::any_cast<std::shared_ptr<ProductLifetime>>(
        &root->tag());
    if (lifetime == nullptr || *lifetime == nullptr) return false;
    const std::string normalized = lower_ascii(surface);
    if (normalized == "criteria") {
        return (*lifetime)->criteria_controller != nullptr &&
               (*lifetime)->criteria_controller->show();
    }
    return (*lifetime)->search_controller != nullptr &&
           (*lifetime)->search_controller->select_surface(normalized);
}

} // namespace file_manager_demoboard
