#include "gui_forms/collection_controls.hpp"
#include "gui_forms/commands.hpp"
#include "gui_forms/window.hpp"
#include "support/named_callbacks.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace gui_forms;
using namespace std::chrono_literals;

class ImageRecordingPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void clip_rounded_rect(Rect, double) override {}
    void fill_rect(Rect bounds, Color) override {
        fills.push_back(bounds);
        paint_order.push_back('F');
    }
    void fill_rounded_rect(Rect, double, Color) override {}
    void stroke_rect(Rect bounds, Color, double) override {
        strokes.push_back(bounds);
        paint_order.push_back('S');
    }
    void stroke_rounded_rect(Rect, double, Color, double) override {}
    void fill_linear_gradient(Rect, Point, Point,
                              std::span<const GradientStop>) override {}
    void draw_line(Point from, Point to, Color, double) override {
        lines.emplace_back(from, to);
        paint_order.push_back('L');
    }
    void draw_text_utf8(Point origin, std::string_view text, FontSpec, Color) override {
        texts.emplace_back(text);
        text_origins.push_back(origin);
        paint_order.push_back('T');
    }
    void draw_image(ImageId image, Rect destination, double opacity) override {
        images.push_back(image);
        destinations.push_back(destination);
        opacities.push_back(opacity);
    }

    std::vector<ImageId> images;
    std::vector<Rect> destinations;
    std::vector<double> opacities;
    std::vector<Rect> fills;
    std::vector<Rect> strokes;
    std::vector<std::string> texts;
    std::vector<Point> text_origins;
    std::vector<std::pair<Point, Point>> lines;
    std::vector<char> paint_order;
};

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

Point semantic_center(const ObjectView& view, std::string_view stable_id);

class AppendTreeExpansion final {
public:
    explicit AppendTreeExpansion(std::string& trace) : trace_(trace) {}

    void operator()(const TreeExpansionChange& change) const {
        trace_ += change.stable_id +
            (change.expanded ? ":open\n" : ":closed\n");
    }

private:
    std::string& trace_;
};

class AppendTreeSelection final {
public:
    explicit AppendTreeSelection(std::string& trace) : trace_(trace) {}

    void operator()(const TreeSelectionChange& change) const {
        trace_ += change.current_id + ":selected\n";
    }

private:
    std::string& trace_;
};

class SemanticNodeHasStableId final {
public:
    explicit SemanticNodeHasStableId(std::string_view stable_id)
        : stable_id_(stable_id) {}

    bool operator()(const SemanticNode& node) const {
        return node.stable_id == stable_id_;
    }

private:
    std::string_view stable_id_;
};

class RecordObjectContextId final {
public:
    explicit RecordObjectContextId(std::string& stable_id)
        : stable_id_(stable_id) {}

    void operator()(const ObjectContextRequest& request) const {
        stable_id_ = request.stable_id;
    }

private:
    std::string& stable_id_;
};

bool semantic_node_is_selected(const SemanticNode& node) {
    return has_semantic_state(node.states, SemanticState::selected);
}

class AppendCommandInvocation final {
public:
    explicit AppendCommandInvocation(std::string& trace) : trace_(trace) {}

    void operator()(const CommandInvocation& invocation) const {
        trace_ += invocation.command_id + "@" + invocation.source_id + "\n";
    }

private:
    std::string& trace_;
};

void test_breadcrumb_identity_overflow_edit_and_input() {
    std::shared_ptr<BreadcrumbTrail> trail = make_control<BreadcrumbTrail>(
        StableId("breadcrumb"), "breadcrumb.exact-editor");
    (*trail).set_requested_bounds({0.0, 0.0, 160.0, 28.0});
    (*trail).set_accessible_name("Current location breadcrumb");
    (*trail).set_segments({
        {"path.root", "Home", "Navigate to Home"},
        {"path.one", "Projects", "Navigate to Projects"},
        {"path.two", "House Composite", "Navigate to House Composite"},
        {"path.three", "Frontend", "Navigate to Frontend"},
        {"path.current", "Source", "Current location"},
    });
    Window window(trail, {160.0, 28.0});
    window.perform_layout();

    require((*trail).children().size() == 1U &&
                (*trail).children().front() == (*trail).editor(),
            "BreadcrumbTrail must own exactly one retained same-row editor rather than per-segment controls");
    require(!(*trail).hidden_segment_ids().empty(),
            "constrained BreadcrumbTrail must retain explicit middle-overflow identities");
    const std::vector<SemanticNode> constrained =
        (*trail).semantic_virtual_children();
    require(constrained.size() >= 4U &&
                constrained.front().stable_id == "path.root" &&
                constrained[1].stable_id == (*trail).overflow_stable_id() &&
                constrained[constrained.size() - 2U].stable_id == "path.current" &&
                constrained.back().stable_id == (*trail).edit_stable_id(),
            "BreadcrumbTrail overflow must preserve root/current identities and expose overflow/edit actuators");

    std::string activated;
    std::size_t overflow_count{};
    std::string committed;
    std::size_t cancelled{};
    std::size_t starts{};
    SubscriptionToken activation = (*trail).segment_activated().subscribe(
        test_support::RecordValue<std::string>(activated));
    SubscriptionToken overflow = (*trail).overflow_activated().subscribe(
        [&overflow_count] { ++overflow_count; });
    SubscriptionToken commit = (*trail).edit_committed().subscribe(
        test_support::RecordValue<std::string>(committed));
    SubscriptionToken cancel = (*trail).edit_cancelled().subscribe(
        [&cancelled] { ++cancelled; });
    SubscriptionToken start = (*trail).edit_started().subscribe(
        [&starts](const std::string&) { ++starts; });
    std::size_t completions{};
    SubscriptionToken completion = (*trail).edit_completion_requested().subscribe(
        [&completions] { ++completions; });
    require((*trail).on_semantic_child_action(
                "path.root", SemanticAction::press, {}) &&
                activated == "path.root",
            "breadcrumb semantic segment press must share typed activation");
    require((*trail).on_semantic_child_action(
                (*trail).overflow_stable_id(), SemanticAction::show_menu, {}) &&
                overflow_count == 1U,
            "breadcrumb overflow semantic action must enter the operational overflow path");

    require(window.request_focus(trail),
            "BreadcrumbTrail must accept retained keyboard focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::home}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                activated == "path.root",
            "Home and Enter must activate the first stable breadcrumb segment");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::end}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                (*trail).editing() && (*trail).editor()->visible() &&
                starts == 1U,
            "End and Enter must replace the trail presentation with its owned editor");
    window.perform_layout();
    const Rect trail_bounds = (*trail).committed_arranged_bounds();
    const Rect editor_bounds = (*trail).editor()->committed_arranged_bounds();
    require(editor_bounds.x == 1.0 && editor_bounds.y == 1.0 &&
                editor_bounds.width == trail_bounds.width - 2.0 &&
                editor_bounds.height == trail_bounds.height - 2.0,
            "breadcrumb editing must preserve outer identity and row geometry");
    (*trail).editor()->set_text("/Users/example/Projects");
    (*trail).set_tab_completion_available(true);
    require(window.dispatch_key({KeyAction::down, PhysicalKey::tab}) &&
                completions == 1U && (*trail).editing(),
            "Tab must enter the typed completion route without traversing focus when a suggestion is available");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                committed == "/Users/example/Projects" && (*trail).editing(),
            "path commit must publish exact editor text while the caller decides admissibility");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::escape}) &&
                !(*trail).editing() && cancelled == 1U,
            "Escape must roll back presentation and restore the breadcrumb row");

    ImageRecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 160.0, 28.0});
    const auto final_face = std::find(
        painter.paint_order.rbegin(), painter.paint_order.rend(), 'F').base();
    const auto joint_lines = static_cast<std::size_t>(std::count(
        final_face, painter.paint_order.end(), 'L'));
    require(final_face != painter.paint_order.begin() &&
                joint_lines == (constrained.size() - 1U) * 2U &&
                std::find(final_face, painter.paint_order.end(), 'F') ==
                    painter.paint_order.end(),
            "BreadcrumbTrail must paint two connected lines per shared chevron edge after every overlapping face");

    bool duplicate_rejected{};
    try {
        (*trail).set_segments({{"duplicate", "One", {}},
                               {"duplicate", "Two", {}}});
    } catch (const std::invalid_argument&) {
        duplicate_rejected = true;
    }
    require(duplicate_rejected,
            "BreadcrumbTrail must reject duplicate caller identities");
}

void test_tree_visibility_identity_and_navigation() {
    std::shared_ptr<gui_forms::TreeView> tree = make_control<TreeView>(StableId("tree"));
    (*tree).set_show_expanders(false);
    require(!(*tree).show_expanders(),
            "TreeView must retain explicit expander visibility policy");
    (*tree).set_show_expanders(true);
    (*tree).set_requested_bounds({0.0, 0.0, 240.0, 140.0});
    (*tree).set_items({
        {"tree.local", "Local", 0, true, true},
        {"tree.home", "quentin", 1, true, true},
        {"tree.desktop", "Desktop", 2},
        {"tree.work", "Work", 2, true, false},
        {"tree.projects", "Projects", 3},
        {"tree.reference", "Reference", 3},
        {"tree.volumes", "Volumes", 0, true, false},
        {"tree.disk", "Macintosh HD", 1},
    });
    (*tree).set_selected_id("tree.work");
    Window window(tree, {240.0, 140.0});
    require((*tree).children().empty(),
            "TreeView must not allocate one retained control per logical item");
    const std::vector<SemanticNode> collapsed = (*tree).semantic_virtual_children();
    require(collapsed.size() == 5U && collapsed.back().stable_id == "tree.volumes",
            "collapsed TreeView descendants must leave the visible semantic window");

    std::string trace;
    SubscriptionToken expansion = (*tree).expansion_changed().subscribe(
        AppendTreeExpansion(trace));
    SubscriptionToken selection = (*tree).selection_changed().subscribe(
        AppendTreeSelection(trace));
    (*tree).set_expanded("tree.work", true);
    require((*tree).semantic_virtual_children().size() == 5U,
            "TreeView semantic realization must remain clipped to visible rows");
    require(window.request_focus(tree), "TreeView must accept retained focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                (*tree).selected_id() == "tree.projects",
            "TreeView Right must enter the first expanded child");
    require(window.dispatch_text({"ref"}) && (*tree).selected_id() == "tree.reference",
            "TreeView type-to-select must use stable visible-row navigation");
    require(trace == "tree.work:open\ntree.projects:selected\ntree.reference:selected\n",
            "TreeView expansion and selection events must be deterministic");
}

void test_tree_model_validation() {
    std::shared_ptr<gui_forms::TreeView> tree = make_control<TreeView>(StableId("tree.invalid"));
    bool rejected{};
    try {
        (*tree).set_items({{"duplicate", "One", 0},
                         {"duplicate", "Two", 2}});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "TreeView must reject duplicate IDs and invalid depth jumps");
}

void test_tree_and_object_view_consume_keyed_image_list() {
    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("collections.images.root"));
    std::shared_ptr<gui_forms::TreeView> tree = make_control<TreeView>(StableId("collections.images.tree"));
    std::shared_ptr<gui_forms::ObjectView> objects = make_control<ObjectView>(StableId("collections.images.objects"));
    (*tree).set_requested_bounds({0.0, 0.0, 180.0, 80.0});
    (*objects).set_requested_bounds({180.0, 0.0, 180.0, 100.0});
    (*root).add_child(tree);
    (*root).add_child(objects);
    Window window(root, {360.0, 100.0});
    const std::array<std::byte, 4> pixel{
        std::byte{0x33}, std::byte{0x77}, std::byte{0xcc}, std::byte{0xff}};
    const ImageLoadResult loaded = window.load_bgra32_premultiplied(
        1U, 1U, 4U, pixel);
    require(static_cast<bool>(loaded), "collection image fixture must load");
    std::shared_ptr<gui_forms::ImageList> images = std::make_shared<ImageList>(window, Size{18.0, 18.0});
    (*images).add_image("folder", loaded.image);
    (*tree).set_image_list(images);
    (*objects).set_image_list(images);
    (*tree).set_items({{"tree.image", "Folder", 0U, false, false, true,
                      "FOLDER"}});
    (*objects).set_items({{"object.image",
                         "An impossibly long local object name that cannot fit",
                         "1 item", "Fixture",
                         ObjectGlyph::folder, true, "folder"}});
    window.perform_layout();
    ImageRecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 360.0, 100.0});
    require(painter.images.size() == 2U &&
                painter.images[0] == loaded.image &&
                painter.images[1] == loaded.image &&
                !painter.destinations[0].empty() &&
                !painter.destinations[1].empty(),
            "TreeView and ObjectView must paint shared keyed ImageList resources instead of private glyph paths");
    require(painter.texts.size() == 3U && painter.texts.front() == "Folder" &&
                painter.texts[1] + " " + painter.texts[2] ==
                    "An impossibly long local object name that cannot fit",
            "ObjectView icon labels must use the admitted second line before eliding complete text");
}

void test_object_label_wrapping_focus_and_full_name_inspection() {
    std::shared_ptr<gui_forms::ObjectView> objects =
        make_control<ObjectView>(StableId("objects.labels"));
    (*objects).set_requested_bounds({0.0, 0.0, 190.0, 170.0});
    (*objects).set_icon_cell_size({90.0, 78.0});
    (*objects).set_show_secondary_text(false);
    const std::string complete_name =
        "Résumé資料 archive evidence package with immutable identity.txt";
    (*objects).set_items({
        {"labels.long", complete_name, {}, "Complete UTF-8 fixture name",
         ObjectGlyph::document},
        {"labels.wrap", "North Shore Recordings", {}, "Two-line fixture",
         ObjectGlyph::folder},
    });
    Window window(objects, {190.0, 170.0});
    window.perform_layout();

    ImageRecordingPainter rest;
    window.paint(rest, {0.0, 0.0, 190.0, 170.0});
    require(rest.texts.size() == 4U &&
                rest.texts[0] == "Résumé資料" &&
                rest.texts[1].ends_with("…") &&
                rest.texts[2] == "North Shore" &&
                rest.texts[3] == "Recordings",
            "icon labels must paint at most two centered UTF-8-safe lines and elide only retained overflow");
    require(rest.text_origins[1].y > rest.text_origins[0].y &&
                rest.text_origins[3].y > rest.text_origins[2].y &&
                std::all_of(rest.text_origins.begin(), rest.text_origins.end(),
                    [](const Point point) {
                        return point.x >= 4.0 && point.x < 186.0;
                    }),
            "two-line icon labels must retain stable baselines inside their cells");

    const Point first = semantic_center(*objects, "labels.long");
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, first}));
    ImageRecordingPainter hovered;
    window.paint(hovered, {0.0, 0.0, 190.0, 170.0});
    std::string inspected_name;
    for (std::size_t index = 4U; index < hovered.texts.size(); ++index) {
        if (!inspected_name.empty()) inspected_name += ' ';
        inspected_name += hovered.texts[index];
    }
    const Rect inspection_border = hovered.strokes.back();
    require(inspected_name == complete_name && hovered.strokes.size() == 3U &&
                inspection_border.x >= 4.0 &&
                inspection_border.x + inspection_border.width <= 186.0 &&
                inspection_border.y >= 4.0 &&
                inspection_border.y + inspection_border.height <= 166.0,
            "hovered truncated item must disclose its complete name in a bounded wrapped inspection surface");
    require((*objects).selected_ids().empty(),
            "full-name hover inspection must not mutate selection authority");

    PointerEvent down{PointerAction::down, PointerButton::primary, first};
    PointerEvent up{PointerAction::up, PointerButton::primary, first};
    require(window.dispatch_pointer(down) && window.dispatch_pointer(up),
            "pointer fixture must focus and select its item");
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::leave, PointerButton::none, {189.0, 169.0}}));
    ImageRecordingPainter pointer_focused;
    window.paint(pointer_focused, {0.0, 0.0, 190.0, 170.0});
    require(pointer_focused.lines.empty() && pointer_focused.strokes.size() == 3U,
            "pointer focus must preserve the selection boundary without masquerading as keyboard focus");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::control}) &&
                (*objects).focused_id() == "labels.wrap" &&
                (*objects).selected_id() == "labels.long",
            "modified keyboard navigation must move focus independently of stable selection");
    ImageRecordingPainter keyboard_short;
    window.paint(keyboard_short, {0.0, 0.0, 190.0, 170.0});
    require(keyboard_short.lines.size() >= 40U &&
                keyboard_short.strokes.size() == 3U,
            "selection boundary and dotted keyboard focus must remain visibly distinct on separate cells");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::left,
                                 Modifier::control}) &&
                (*objects).focused_id() == "labels.long" &&
                (*objects).selected_id() == "labels.long",
            "keyboard fixture must return independent focus without rewriting selection");
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none,
         semantic_center(*objects, "labels.wrap")}));
    ImageRecordingPainter keyboard_long;
    window.paint(keyboard_long, {0.0, 0.0, 190.0, 170.0});
    std::string keyboard_inspected_name;
    for (std::size_t index = 4U; index < keyboard_long.texts.size(); ++index) {
        if (!keyboard_inspected_name.empty()) keyboard_inspected_name += ' ';
        keyboard_inspected_name += keyboard_long.texts[index];
    }
    require(keyboard_long.lines.size() >= 40U &&
                keyboard_inspected_name == complete_name,
            "non-truncated hover must not suppress complete-name inspection for the keyboard-focused item");

    (*objects).set_view_mode(ObjectViewMode::details);
    ImageRecordingPainter details;
    window.paint(details, {0.0, 0.0, 190.0, 170.0});
    require(!details.texts.empty() && details.texts.front().ends_with("…") &&
                details.text_origins.front().x == 42.0,
            "details-mode names must remain single-line and width-bounded before the metadata column");

    std::shared_ptr<gui_forms::ObjectView> grapheme_objects =
        make_control<ObjectView>(StableId("objects.graphemes"));
    (*grapheme_objects).set_requested_bounds({0.0, 0.0, 94.0, 90.0});
    (*grapheme_objects).set_icon_cell_size({90.0, 78.0});
    (*grapheme_objects).set_show_secondary_text(false);
    const std::string joined_family =
        "👨‍👩‍👧‍👦‍👨‍👩‍👧‍👦";
    const TextStore joined_family_store(joined_family);
    require(joined_family_store.grapheme_count() == GraphemeIndex(1U),
            "Unicode 17 fixture must be one extended grapheme cluster");
    (*grapheme_objects).set_items({
        {"graphemes.long", "Prefix " + joined_family + " suffix", {},
         "ZWJ truncation fixture", ObjectGlyph::document},
    });
    Window grapheme_window(grapheme_objects, {94.0, 90.0});
    grapheme_window.perform_layout();
    ImageRecordingPainter graphemes;
    grapheme_window.paint(graphemes, {0.0, 0.0, 94.0, 90.0});
    require(graphemes.texts.size() == 2U &&
                graphemes.texts[0] == "Prefix" &&
                graphemes.texts[1] == "…",
            "icon-label elision must drop an overwide grapheme whole rather than split its ZWJ sequence");
}

void test_object_virtualization_view_preservation_and_input() {
    std::shared_ptr<gui_forms::ObjectView> objects = make_control<ObjectView>(StableId("objects"));
    (*objects).set_show_secondary_text(false);
    require(!(*objects).show_secondary_text(),
            "ObjectView must retain secondary-text visibility policy");
    (*objects).set_show_secondary_text(true);
    (*objects).set_requested_bounds({0.0, 0.0, 420.0, 190.0});
    std::vector<ObjectViewItem> model;
    model.reserve(1000U);
    for (std::size_t index = 0; index < 1000U; ++index) {
        model.push_back({"object." + std::to_string(index),
                         index == 777U ? "Quartz report" :
                             "Object " + std::to_string(index),
                         std::to_string(index) + " KB",
                         "Deterministic fixture object",
                         index % 5U == 0U ? ObjectGlyph::folder
                                          : ObjectGlyph::document});
    }
    (*objects).set_items(std::move(model));
    (*objects).set_selected_id("object.5");
    Window window(objects, {420.0, 190.0});
    require((*objects).children().empty(),
            "ObjectView must not allocate one retained control per logical item");
    require((*objects).semantic_virtual_children().size() <= 12U,
            "ObjectView semantic realization must remain bounded for 1000 items");
    require(window.request_focus(objects), "ObjectView must accept retained focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right}) &&
                (*objects).selected_id() == "object.6",
            "ObjectView Right must use deterministic spatial navigation");
    require(window.dispatch_text({"quartz"}) &&
                (*objects).selected_id() == "object.777",
            "ObjectView type-to-select must reach an unrealized stable item");
    (*objects).set_view_mode(ObjectViewMode::details);
    require((*objects).selected_id() == "object.777" &&
                (*objects).semantic_virtual_children().size() <= 8U,
            "ObjectView mode changes must preserve stable selection and bounded semantics");

    std::string activated;
    SubscriptionToken activation = (*objects).item_activated().subscribe(
        test_support::RecordValue<std::string>(activated));
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                activated == "object.777",
            "ObjectView Enter must activate the focused stable item");
    activated.clear();
    const std::vector<SemanticNode> visible = (*objects).semantic_virtual_children();
    const std::vector<SemanticNode>::const_iterator target = std::find_if(
        visible.begin(), visible.end(),
        SemanticNodeHasStableId("object.777"));
    require(target != visible.end(), "focused ObjectView item must be realized");
    const Point center{(*target).bounds.x + (*target).bounds.width * .5,
                       (*target).bounds.y + (*target).bounds.height * .5};
    require((*objects).item_id_at(center) == "object.777" &&
                (*objects).item_id_at({421.0, 191.0}).empty(),
            "ObjectView must map root-client drag coordinates to stable item identity");
    PointerEvent single_down{PointerAction::down, PointerButton::primary, center};
    PointerEvent single_up{PointerAction::up, PointerButton::primary, center};
    require(window.dispatch_pointer(single_down) && window.dispatch_pointer(single_up) &&
                activated.empty(),
            "ObjectView single click must select without invoking the default action");
    PointerEvent double_down{PointerAction::down, PointerButton::primary, center};
    double_down.click_count = 2U;
    PointerEvent double_up{PointerAction::up, PointerButton::primary, center};
    double_up.click_count = 2U;
    require(window.dispatch_pointer(double_down) && window.dispatch_pointer(double_up) &&
                activated == "object.777",
            "ObjectView native double click must invoke exactly one default action");
    require((*objects).on_semantic_child_action("object.777", SemanticAction::select, {}) &&
                (*objects).selected_id() == "object.777",
            "ObjectView semantic selection must share the ordinary selection path");
}

Point semantic_center(const ObjectView& view, std::string_view stable_id) {
    const std::vector<SemanticNode> nodes = view.semantic_virtual_children();
    const std::vector<SemanticNode>::const_iterator found = std::find_if(
        nodes.begin(), nodes.end(), SemanticNodeHasStableId(stable_id));
    require(found != nodes.end(), "test item must be semantically realized");
    return {(*found).bounds.x + (*found).bounds.width * .5,
            (*found).bounds.y + (*found).bounds.height * .5};
}

void pointer_click(Window& window, Point point, PointerButton button,
                   Modifier modifiers = Modifier::none) {
    PointerEvent down{PointerAction::down, button, point, {}, modifiers};
    PointerEvent up{PointerAction::up, button, point, {}, modifiers};
    require(window.dispatch_pointer(down) && window.dispatch_pointer(up),
            "collection pointer click must be retained and handled");
}

void require_selection(const ObjectView& view,
                       std::initializer_list<std::string_view> expected,
                       const char* message) {
    if (view.selected_ids().size() != expected.size()) {
        throw std::runtime_error(message);
    }
    std::size_t index{};
    for (const std::string_view id : expected) {
        if (view.selected_ids()[index++] != id) {
            throw std::runtime_error(message);
        }
    }
}

void test_object_multiselection_pointer_keyboard_and_semantics() {
    std::shared_ptr<gui_forms::ObjectView> objects = make_control<ObjectView>(StableId("objects.multi"));
    (*objects).set_requested_bounds({0.0, 0.0, 320.0, 270.0});
    (*objects).set_icon_cell_size({100.0, 80.0});
    std::vector<ObjectViewItem> model;
    for (std::size_t index = 0; index < 8U; ++index) {
        model.push_back({"multi." + std::to_string(index),
                         "Item " + std::to_string(index), {}, {},
                         ObjectGlyph::document});
    }
    (*objects).set_items(model);
    Window window(objects, {320.0, 270.0});
    require(window.request_focus(objects),
            "multi-selection collection must accept focus");

    std::vector<ObjectSelectionChange> changes;
    SubscriptionToken changed = (*objects).selection_changed().subscribe(
        test_support::PushBack<std::vector<ObjectSelectionChange>,
                               const ObjectSelectionChange&>(changes));

    pointer_click(window, semantic_center(*objects, "multi.1"),
                  PointerButton::primary);
    require_selection(*objects, {"multi.1"},
                      "plain click must establish one stable selection");
    require((*objects).selected_id() == "multi.1" &&
                (*objects).selection_anchor_id() == "multi.1",
            "plain click must establish primary and range-anchor identities");

    pointer_click(window, semantic_center(*objects, "multi.3"),
                  PointerButton::primary, Modifier::meta);
    require_selection(*objects, {"multi.1", "multi.3"},
                      "Command-click must toggle without discarding selection");
    require((*objects).selected_id() == "multi.3",
            "newly toggled item must become the primary selection");

    pointer_click(window, semantic_center(*objects, "multi.6"),
                  PointerButton::primary, Modifier::shift);
    require_selection(*objects, {"multi.3", "multi.4", "multi.5", "multi.6"},
                      "Shift-click must replace selection with an inclusive visual range");
    require((*objects).selection_anchor_id() == "multi.3" &&
                (*objects).focused_id() == "multi.6",
            "range selection must preserve anchor and move independent focus");

    std::string context_id;
    SubscriptionToken context = (*objects).context_requested().subscribe(
        RecordObjectContextId(context_id));
    pointer_click(window, semantic_center(*objects, "multi.4"),
                  PointerButton::secondary);
    require_selection(*objects, {"multi.3", "multi.4", "multi.5", "multi.6"},
                      "right click inside multiselection must preserve it");
    require(context_id == "multi.4",
            "right click must target the item without rewriting selection authority");
    const std::vector<SemanticNode> semantic_menu_nodes = (*objects).semantic_virtual_children();
    const std::vector<SemanticNode>::const_iterator semantic_menu_node = std::find_if(
        semantic_menu_nodes.begin(), semantic_menu_nodes.end(),
        SemanticNodeHasStableId("multi.4"));
    require(semantic_menu_node != semantic_menu_nodes.end() &&
                std::find((*semantic_menu_node).actions.begin(),
                          (*semantic_menu_node).actions.end(),
                          SemanticAction::show_menu) != (*semantic_menu_node).actions.end(),
            "object rows must publish a distinct semantic Show Menu action");
    context_id.clear();
    require((*objects).on_semantic_child_action(
                "multi.4", SemanticAction::show_menu, {}) &&
                context_id == "multi.4",
            "semantic Show Menu must enter the same stable context-request path");
    pointer_click(window, semantic_center(*objects, "multi.0"),
                  PointerButton::secondary);
    require_selection(*objects, {"multi.0"},
                      "right click outside selection must select its context target");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::a, Modifier::control}),
            "Control+A must be consumed by the focused collection");
    require((*objects).selected_ids().size() == 8U,
            "Select All must select every logical enabled item");
    const std::vector<SemanticNode> all_nodes = (*objects).semantic_virtual_children();
    require(std::all_of(all_nodes.begin(), all_nodes.end(),
                        semantic_node_is_selected),
            "every realized member of a multiselection must publish selected semantics");

    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::control}) &&
                (*objects).focused_id() == "multi.1" &&
                (*objects).selected_ids().size() == 8U,
            "Control+Arrow must move focus without mutating selection");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::space,
                                 Modifier::control}),
            "Control+Space must toggle the focused stable item");
    require_selection(*objects,
                      {"multi.0", "multi.2", "multi.3", "multi.4",
                       "multi.5", "multi.6", "multi.7"},
                      "Control+Space must remove only the focused item");

    const Point background{310.0, 266.0};
    pointer_click(window, background, PointerButton::primary);
    require((*objects).selected_ids().empty() && (*objects).selected_id().empty(),
            "plain background click must clear selection without losing focus");

    (*objects).set_selected_ids({"multi.6", "multi.2"}, "multi.6");
    std::reverse(model.begin(), model.end());
    (*objects).set_items(model);
    require_selection(*objects, {"multi.6", "multi.2"},
                      "model replacement must retain selected stable IDs in new visual order");
    (*objects).set_view_mode(ObjectViewMode::details);
    require_selection(*objects, {"multi.6", "multi.2"},
                      "view projection changes must preserve the complete selection set");
    require(!changes.empty() && changes.back().current_ids.size() == 2U,
            "selection events must carry deterministic previous/current snapshots");
}

void test_shared_command_binding() {
    std::shared_ptr<gui_forms::Command> command = std::make_shared<Command>("view.mode", "Icons  ▼");
    std::shared_ptr<gui_forms::Button> ribbon = make_control<Button>(StableId("ribbon.view"), "stale");
    std::shared_ptr<gui_forms::Button> status = make_control<Button>(StableId("status.view"), "stale");
    CommandBinding ribbon_binding(command, ribbon);
    CommandBinding status_binding(command, status);
    require((*ribbon).text() == "Icons  ▼" && (*status).text() == "Icons  ▼",
            "shared command must initialize every bound presentation");
    std::string trace;
    SubscriptionToken invoked = (*command).invoked().subscribe(
        AppendCommandInvocation(trace));
    require((*ribbon).perform_click() && (*status).perform_click(),
            "enabled command presentations must accept public click execution");
    require(trace == "view.mode@ribbon.view\nview.mode@status.view\n",
            "bound presentations must converge on one ordered command path");
    (*command).set_enabled(false);
    require(!(*ribbon).perform_click(),
            "disabled command presentation must reject public click execution");
    require(!(*ribbon).enabled() && !(*status).enabled() &&
                trace == "view.mode@ribbon.view\nview.mode@status.view\n",
            "disabled command state must synchronize and reject execution");

    std::shared_ptr<gui_forms::Command> local_command = std::make_shared<Command>("local.enabled", "Local");
    std::shared_ptr<gui_forms::Button> local_button = make_control<Button>(StableId("local.enabled.button"),
                                             "Local");
    (*local_button).set_enabled(false);
    CommandBindingOptions local_options;
    local_options.synchronize_enabled = false;
    CommandBinding local_binding(local_command, local_button, local_options);
    (*local_command).set_enabled(false);
    (*local_command).set_enabled(true);
    require(!(*local_button).enabled(),
            "command bindings may preserve locally managed enabled state");

    const CommandBindingOptions legacy_aggregate{false, true, true};
    require(!legacy_aggregate.synchronize_text &&
                legacy_aggregate.synchronize_visibility &&
                legacy_aggregate.synchronize_accessible_description &&
                legacy_aggregate.synchronize_enabled,
            "new command options must preserve the established aggregate field order");
}

std::vector<CorrespondenceItem> correspondence_fixture(std::size_t count) {
    std::vector<CorrespondenceItem> items;
    items.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        items.push_back({
            "correspondence." + std::to_string(index),
            "Evidence record " + std::to_string(index),
            "Projects / deterministic fixture",
            "Exact stored reason " + std::to_string(index),
            "Matched excerpt for record " + std::to_string(index),
            std::to_string(100U - index % 80U) + "%",
            "Provider information",
            {"Local index", "Text extractor"},
            "Generation 86 · factual fixture evidence",
            index % 4U == 0U ? ObjectGlyph::image : ObjectGlyph::document,
            true,
            index == 12U,
            index == 13U,
        });
    }
    return items;
}

void test_correspondence_virtualization_and_anchor_stability() {
    std::shared_ptr<gui_forms::CorrespondenceView> customization = make_control<CorrespondenceView>(
        StableId("correspondence.customization"));
    (*customization).set_status_rail_width(7.0);
    require((*customization).status_rail_width() == 7.0,
            "CorrespondenceView must retain explicit status-rail width");
    bool rejected_rail = false;
    try {
        (*customization).set_status_rail_width(0.0);
    } catch (const std::invalid_argument&) {
        rejected_rail = true;
    }
    require(rejected_rail,
            "CorrespondenceView must reject nonvisual status-rail widths");

    std::shared_ptr<gui_forms::CorrespondenceView> records = make_control<CorrespondenceView>(
        StableId("correspondence.records"));
    (*records).set_requested_bounds({0.0, 0.0, 820.0, 230.0});
    (*records).set_items(correspondence_fixture(10000U));
    (*records).set_accessible_name("Evidence correspondence");
    Window window(records, {820.0, 230.0});
    window.perform_layout();
    require((*records).children().empty(),
            "CorrespondenceView must not allocate one retained control per logical item");
    require((*records).realized_count() <= 8U &&
                (*records).semantic_virtual_children().size() <= 8U,
            "variable-height correspondence realization must remain viewport bounded");

    (*records).set_scroll_offset(45.0 * 10.0);
    const double anchored_y = (*(*records).item_bounds("correspondence.10")).y;
    (*records).set_pinned_id("correspondence.2");
    require((*records).pinned_id() == "correspondence.2" &&
                (*records).expanded("correspondence.2") &&
                std::abs((*(*records).item_bounds("correspondence.10")).y -
                         anchored_y) < 0.01,
            "expanding a row above the viewport must preserve the visible scroll anchor");

    const std::optional<Rect> hover_bounds = (*records).item_bounds("correspondence.12");
    require(hover_bounds && (*hover_bounds).y >= 0.0 &&
                (*hover_bounds).y < 230.0,
            "hover test row must be physically realized");
    const Point hover_center{(*hover_bounds).x + (*hover_bounds).width * .5,
                             (*hover_bounds).y + (*hover_bounds).height * .5};
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, hover_center}));
    require((*records).hovered_id() == "correspondence.12" &&
                !(*records).expanded("correspondence.12") &&
                window.next_wake().has_value(),
            "pointer arrival must schedule rather than immediately fake hover intent");
    const double before_expand = (*(*records).item_bounds("correspondence.12")).y;
    static_cast<void>(window.poll_frame_schedule(*window.next_wake()));
    require((*records).expanded("correspondence.12") &&
                (*records).pinned_id() == "correspondence.2" &&
                std::abs((*(*records).item_bounds("correspondence.12")).y -
                         before_expand) < 0.01,
            "hover intent must preserve its pointer target while a distinct pinned row persists");
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, {900.0, 300.0}}));
    require(!(*records).expanded("correspondence.12") &&
                (*records).expanded("correspondence.2"),
            "pointer leave must collapse only transient inspection");
}

void test_correspondence_keyboard_pin_semantics_and_activation() {
    std::shared_ptr<gui_forms::CorrespondenceView> records = make_control<CorrespondenceView>(
        StableId("correspondence.keyboard"));
    (*records).set_requested_bounds({0.0, 0.0, 820.0, 360.0});
    (*records).set_items(correspondence_fixture(20U));
    (*records).set_selected_id("correspondence.10");
    (*records).set_pinned_id("correspondence.0");
    Window window(records, {820.0, 360.0});
    require(window.request_focus(records) &&
                (*records).focused_id() == "correspondence.10" &&
                (*records).expanded("correspondence.10") &&
                (*records).expanded("correspondence.0"),
            "keyboard focus must expand independently from one persistent pin");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::down}) &&
                (*records).focused_id() == "correspondence.11" &&
                (*records).selected_id() == "correspondence.11" &&
                (*records).expanded("correspondence.11"),
            "Down must move stable focus/selection and expand the keyboard-active row");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::space}) &&
                (*records).pinned_id() == "correspondence.11" &&
                !(*records).expanded("correspondence.0"),
            "Space must replace the sole persistent pin without losing focus");

    std::string activated;
    SubscriptionToken activation = (*records).item_activated().subscribe(
        test_support::RecordValue<std::string>(activated));
    require(window.dispatch_key({KeyAction::down, PhysicalKey::enter}) &&
                activated == "correspondence.11",
            "Enter must activate rather than toggle the active correspondence pin");
    const std::vector<SemanticNode> nodes = (*records).semantic_virtual_children();
    const std::vector<SemanticNode>::const_iterator active = std::find_if(
        nodes.begin(), nodes.end(),
        SemanticNodeHasStableId("correspondence.11"));
    require(active != nodes.end() &&
                has_semantic_state((*active).states, SemanticState::expanded) &&
                (*active).children.size() == 5U &&
                (*active).children[0].stable_id ==
                    "correspondence.11.activate" &&
                (*active).children[1].stable_id ==
                    "correspondence.11.percentage" &&
                (*active).children[4].role == SemanticRole::group,
            "expanded virtual semantics must expose default action, factual metric, metadata, excerpt, and provider lanes");
    activated.clear();
    require((*records).on_semantic_child_action(
                "correspondence.11.activate", SemanticAction::press, {}) &&
                activated == "correspondence.11",
            "the explicit correspondence default-action child must share item activation");
    require((*records).on_semantic_child_action(
                "correspondence.12", SemanticAction::expand, {}) &&
                (*records).pinned_id() == "correspondence.12" &&
                (*records).expanded("correspondence.12"),
            "semantic expansion must enter the same one-pin state path for unavailable evidence");
}

void test_correspondence_background_pointer_contract() {
    std::shared_ptr<gui_forms::CorrespondenceView> records =
        make_control<CorrespondenceView>(StableId("correspondence.background"));
    (*records).set_requested_bounds({0.0, 0.0, 820.0, 360.0});
    (*records).set_items(correspondence_fixture(1U));
    (*records).set_selected_id("correspondence.0");
    Window window(records, {820.0, 360.0});
    window.perform_layout();

    std::vector<ObjectContextRequest> requests;
    SubscriptionToken context = (*records).context_requested().subscribe(
        [&requests](const ObjectContextRequest& request) {
            requests.push_back(request);
        });
    const Rect bounds = (*records).absolute_bounds();
    const Point background{bounds.x + bounds.width * .5,
                           bounds.y + bounds.height - 12.0};
    require(window.dispatch_pointer(
                {PointerAction::down, PointerButton::primary, background}) &&
                window.dispatch_pointer(
                    {PointerAction::up, PointerButton::primary, background}) &&
                (*records).selected_id().empty() && requests.empty() &&
                window.focused_control() == records,
            "primary correspondence background click must focus the collection and clear selection without opening a menu");

    (*records).set_selected_id("correspondence.0");
    require(window.dispatch_pointer(
                {PointerAction::down, PointerButton::secondary, background}) &&
                window.dispatch_pointer(
                    {PointerAction::up, PointerButton::secondary, background}) &&
                (*records).selected_id().empty() && requests.size() == 1U &&
                requests.front().stable_id.empty() &&
                requests.front().screen_position.x == background.x &&
                requests.front().screen_position.y == background.y,
            "secondary correspondence background click must clear selection and emit one exact empty-target context request");
}

} // namespace

int main() {
    try {
        test_breadcrumb_identity_overflow_edit_and_input();
        test_tree_visibility_identity_and_navigation();
        test_tree_model_validation();
        test_tree_and_object_view_consume_keyed_image_list();
        test_object_label_wrapping_focus_and_full_name_inspection();
        test_object_virtualization_view_preservation_and_input();
        test_object_multiselection_pointer_keyboard_and_semantics();
        test_shared_command_binding();
        test_correspondence_virtualization_and_anchor_stability();
        test_correspondence_keyboard_pin_semantics_and_activation();
        test_correspondence_background_pointer_contract();
        std::cout << "gui_forms_collection_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_collection_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
