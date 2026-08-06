#include "gui_forms/basic_controls.hpp"
#include "gui_forms/diagnostic_controls.hpp"
#include "gui_forms/window.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class RecordingPainter final : public Painter {
public:
    void save() override { ++saves; }
    void restore() override { ++restores; }
    void translate(Point) override {}
    void clip_rect(Rect) override { ++clips; }
    void fill_rect(Rect, Color) override { ++fills; }
    void stroke_rect(Rect, Color, double) override { ++strokes; }
    void draw_line(Point, Point, Color, double) override { ++lines; }
    void draw_text_utf8(Point origin, std::string_view text, FontSpec font, Color) override {
        texts.emplace_back(text);
        roles.push_back(font.role);
        text_origins.push_back(origin);
    }
    void draw_image(ImageId image, Rect destination, double opacity) override {
        images.push_back(image);
        image_destinations.push_back(destination);
        image_opacities.push_back(opacity);
    }

    std::uint64_t saves{};
    std::uint64_t restores{};
    std::uint64_t clips{};
    std::uint64_t fills{};
    std::uint64_t strokes{};
    std::uint64_t lines{};
    std::vector<std::string> texts;
    std::vector<FontRole> roles;
    std::vector<Point> text_origins;
    std::vector<ImageId> images;
    std::vector<Rect> image_destinations;
    std::vector<double> image_opacities;
};

Point center(const Control::Ptr& control) {
    const Rect bounds = control->absolute_bounds();
    return {bounds.x + bounds.width * 0.5, bounds.y + bounds.height * 0.5};
}

void click(Window& window, const Control::Ptr& control) {
    const Point point = center(control);
    require(window.dispatch_pointer(
                {PointerAction::down, PointerButton::primary, point}),
            "reusable control must handle primary down");
    require(window.dispatch_pointer(
                {PointerAction::up, PointerButton::primary, point}),
            "reusable control must handle primary up");
}

void test_public_controls_render_with_role_policy() {
    auto panel = make_control<Panel>(StableId("controls.panel"));
    panel->set_accessible_name("Public panel group");
    panel->set_requested_bounds({0.0, 0.0, 360.0, 180.0});
    panel->set_border_style(BorderStyle::line);
    auto group = make_control<GroupBox>(StableId("controls.group"), "Reusable controls");
    group->set_requested_bounds({10.0, 10.0, 340.0, 160.0});
    auto label = make_control<Label>(StableId("controls.label"), "Field content");
    label->set_requested_bounds({12.0, 24.0, 140.0, 24.0});
    auto button = make_control<Button>(StableId("controls.button"), "Apply");
    button->set_requested_bounds({12.0, 54.0, 90.0, 28.0});
    button->set_default_button(true);
    auto check = make_control<CheckBox>(StableId("controls.check"), "Checked option");
    check->set_requested_bounds({12.0, 88.0, 150.0, 24.0});
    check->set_checked(true);
    auto radio = make_control<RadioButton>(StableId("controls.radio"), "Radio option");
    radio->set_requested_bounds({170.0, 88.0, 150.0, 24.0});
    radio->set_checked(true);
    group->add_child(label);
    group->add_child(button);
    group->add_child(check);
    group->add_child(radio);
    panel->add_child(group);
    Window window(panel, {360.0, 180.0});
    window.perform_layout();

    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 360.0, 180.0});
    require(painter.saves == painter.restores && painter.clips >= 6 &&
                painter.fills >= 4 && painter.lines >= 12 &&
                painter.texts.size() == 5,
            "public controls must render through retained painter commands");
    require(painter.roles[0] == FontRole::control &&
                painter.roles[1] == FontRole::content,
            "group/control titles and field labels must preserve typography roles");
    require(window.hit_test(center(label)) == group,
            "noninteractive Label must not intercept its logical container");
    require(panel->semantic_descriptor().role == SemanticRole::group &&
                panel->semantic_descriptor().name == "Public panel group" &&
                panel->semantic_descriptor().exposed,
            "a named public Panel must expose stock group semantics");
}

void test_button_pointer_and_keyboard_activation() {
    auto button = make_control<Button>(StableId("button"), "Run");
    button->set_requested_bounds({10.0, 10.0, 90.0, 28.0});
    Window window(button, {120.0, 60.0});
    std::uint64_t clicks = 0;
    auto clicked = button->clicked().subscribe(
        [&clicks](ButtonBase&) { ++clicks; });

    click(window, button);
    require(clicks == 1 && window.focused_control() == button,
            "matching pointer press/release must click and focus Button once");
    KeyEvent down;
    down.action = KeyAction::down;
    down.physical_key = PhysicalKey::space;
    KeyEvent up = down;
    up.action = KeyAction::up;
    require(window.dispatch_key(down) && window.dispatch_key(up) && clicks == 2,
            "focused Button must activate once on normalized Space release");
    require(!button->pressed_visual(),
            "Button keyboard visual must clear after activation");
}

void test_checkbox_state_and_click_order() {
    auto check = make_control<CheckBox>(StableId("check"), "Precise");
    check->set_requested_bounds({0.0, 0.0, 140.0, 24.0});
    Window window(check, {140.0, 24.0});
    std::string order;
    auto state = check->check_state_changed().subscribe(
        [&order](CheckState value) {
            order += value == CheckState::checked ? "state:checked\n" : "state:other\n";
        });
    auto checked = check->checked_changed().subscribe(
        [&order](bool value) { order += value ? "checked:true\n" : "checked:false\n"; });
    auto clicked = check->clicked().subscribe(
        [&order](ButtonBase&) { order += "click\n"; });

    click(window, check);
    require(check->checked() &&
                order == "state:checked\nchecked:true\nclick\n",
            "CheckBox must mutate, publish state, publish checked, then click");

    check->set_three_state(true);
    order.clear();
    check->on_activate();
    require(check->check_state() == CheckState::indeterminate &&
                order == "state:other\nchecked:false\nclick\n",
            "three-state CheckBox must deterministically advance to indeterminate");
    order.clear();
    check->on_activate();
    require(check->check_state() == CheckState::unchecked &&
                order == "state:other\nclick\n",
            "indeterminate CheckBox must return to unchecked without duplicate bool change");
}

void test_radio_group_scope_and_order() {
    auto group = make_control<GroupBox>(StableId("radio.group"), "Mode");
    group->set_requested_bounds({0.0, 0.0, 360.0, 70.0});
    auto first = make_control<RadioButton>(StableId("radio.first"), "First");
    auto second = make_control<RadioButton>(StableId("radio.second"), "Second");
    auto independent = make_control<RadioButton>(StableId("radio.other"), "Other group");
    first->set_requested_bounds({10.0, 24.0, 100.0, 24.0});
    second->set_requested_bounds({120.0, 24.0, 100.0, 24.0});
    independent->set_requested_bounds({230.0, 24.0, 120.0, 24.0});
    first->set_group_name("mode");
    second->set_group_name("mode");
    independent->set_group_name("independent");
    group->add_child(first);
    group->add_child(second);
    group->add_child(independent);
    first->set_checked(true);
    independent->set_checked(true);
    Window window(group, {360.0, 70.0});

    std::string order;
    auto first_changed = first->checked_changed().subscribe(
        [&order](bool value) { order += value ? "first:on\n" : "first:off\n"; });
    auto second_changed = second->checked_changed().subscribe(
        [&order](bool value) { order += value ? "second:on\n" : "second:off\n"; });
    auto second_clicked = second->clicked().subscribe(
        [&order](ButtonBase&) { order += "second:click\n"; });
    click(window, second);
    require(!first->checked() && second->checked() && independent->checked() &&
                order == "first:off\nsecond:on\nsecond:click\n",
            "RadioButton must uncheck its group peer before checking/clicking itself");
}

void test_link_and_callback_disposal() {
    auto link = make_control<LinkLabel>(StableId("link"), "Show details");
    link->set_requested_bounds({0.0, 0.0, 120.0, 24.0});
    Window link_window(link, {120.0, 24.0});
    std::uint64_t clicks = 0;
    auto link_click = link->clicked().subscribe(
        [&clicks](ButtonBase&) { ++clicks; });
    click(link_window, link);
    require(link->visited() && clicks == 1,
            "LinkLabel must mark visited before publishing activation");

    auto root = make_control<Panel>(StableId("dispose.root"));
    auto check = make_control<CheckBox>(StableId("dispose.check"), "Dispose safely");
    check->set_requested_bounds({0.0, 0.0, 140.0, 24.0});
    root->add_child(check);
    Window window(root, {140.0, 24.0});
    std::uint64_t later_callbacks = 0;
    auto dispose_on_state = check->check_state_changed().subscribe(
        [check](CheckState) { check->dispose(); });
    auto checked = check->checked_changed().subscribe(
        [&later_callbacks](bool) { ++later_callbacks; });
    auto clicked = check->clicked().subscribe(
        [&later_callbacks](ButtonBase&) { ++later_callbacks; });
    check->on_activate();
    require(!check->is_alive() && later_callbacks == 0 &&
                !window.find("dispose.check"),
            "state callback disposal must suppress later checked/click callbacks safely");
}

void test_wrong_thread_property_mutation_is_rejected() {
    auto label = make_control<Label>(StableId("thread.label"), "Before");
    Window window(label, {100.0, 24.0});
    bool rejected = false;
    std::thread worker([&] {
        try {
            label->set_text("After");
        } catch (const std::logic_error&) {
            rejected = true;
        }
    });
    worker.join();
    require(rejected && label->text() == "Before",
            "reusable control properties must retain core UI-thread enforcement");
}

void test_label_multiline_wrapping_and_alignment() {
    auto label = make_control<Label>(StableId("label.multiline"),
                                     "Retained labels wrap words\nand preserve breaks");
    label->set_requested_bounds({0.0, 0.0, 120.0, 80.0});
    label->set_text_wrapping(TextWrapping::word);
    label->set_vertical_alignment(VerticalAlignment::near);
    label->set_alignment(HorizontalAlignment::far);
    Window window(label, {120.0, 80.0});
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 120.0, 80.0});
    require(painter.texts.size() == 4U && painter.texts.front() == "Retained labels" &&
                painter.texts[2] == "and preserve" && painter.texts.back() == "breaks",
            "Label must word-wrap and preserve explicit line breaks");
    require(painter.text_origins.front().y < painter.text_origins.back().y &&
                painter.text_origins.front().x > 2.0,
            "Label multiline alignment must position each line independently");

    bool rejected = false;
    try {
        label->set_line_spacing(0.1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && label->line_spacing() == 1.25,
            "Label must reject invalid line spacing without mutation");
}

void test_picture_box_modes_registry_and_semantics() {
    auto picture = make_control<PictureBox>(StableId("picture"));
    picture->set_requested_bounds({0.0, 0.0, 100.0, 100.0});
    picture->set_border_style(BorderStyle::line);
    picture->set_accessible_name("Professional test card");
    Window window(picture, {100.0, 100.0});
    const std::vector<std::byte> pixels(4U * 2U * 4U, std::byte{0xff});
    const ImageLoadResult loaded = window.load_bgra32_premultiplied(
        4U, 2U, 16U, pixels);
    require(static_cast<bool>(loaded),
            "PictureBox test image must enter the window-owned registry");
    std::uint64_t changes{};
    auto changed = picture->image_changed().subscribe(
        [&changes](ImageId) { ++changes; });
    picture->set_image(loaded.image);
    window.perform_layout();
    require(changes == 1U && picture->has_valid_image() &&
                picture->image_size() == Size{4.0, 2.0},
            "PictureBox must publish an image change and resolve registry metadata");

    const std::array<std::pair<PictureBoxSizeMode, Rect>, 4> modes{{
        {PictureBoxSizeMode::normal, {1.0, 1.0, 4.0, 2.0}},
        {PictureBoxSizeMode::stretch_image, {1.0, 1.0, 98.0, 98.0}},
        {PictureBoxSizeMode::center_image, {48.0, 49.0, 4.0, 2.0}},
        {PictureBoxSizeMode::zoom, {1.0, 25.5, 98.0, 49.0}},
    }};
    for (const auto& [mode, expected] : modes) {
        picture->set_size_mode(mode);
        require(picture->image_bounds() == expected,
                "PictureBox sizing mode must compute deterministic image geometry");
        RecordingPainter painter;
        window.paint(painter, {0.0, 0.0, 100.0, 100.0});
        require(painter.images.size() == 1U &&
                    painter.images.front() == loaded.image &&
                    painter.image_destinations.front() == expected,
                "PictureBox must submit the resolved image and destination to Painter");
    }

    picture->set_size_mode(PictureBoxSizeMode::auto_size);
    require(picture->measure({1000.0, 1000.0}) == Size{6.0, 4.0} &&
                picture->image_bounds() == Rect{1.0, 1.0, 4.0, 2.0},
            "PictureBox AutoSize must measure to intrinsic pixels plus its border");
    picture->set_image_opacity(0.42);
    RecordingPainter opacity_painter;
    window.paint(opacity_painter, {0.0, 0.0, 100.0, 100.0});
    require(opacity_painter.image_opacities.size() == 1U &&
                opacity_painter.image_opacities.front() == 0.42,
            "PictureBox must preserve renderer-neutral image opacity");

    const SemanticSnapshot semantics = window.semantic_snapshot();
    const std::string json = semantics.to_json();
    require(json.find("\"role\":\"image\"") != std::string::npos &&
                json.find("Professional test card") != std::string::npos &&
                json.find("4 x 2") != std::string::npos,
            "PictureBox must expose stable image semantics and intrinsic dimensions");

    bool rejected{};
    try {
        picture->set_image_opacity(1.1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && picture->image_opacity() == 0.42,
            "PictureBox must reject invalid opacity without mutation");
    require(window.remove_image(loaded.image) && !picture->has_valid_image(),
            "PictureBox must reject a removed generational image ID safely");
    RecordingPainter stale_painter;
    window.paint(stale_painter, {0.0, 0.0, 100.0, 100.0});
    require(stale_painter.images.empty(),
            "PictureBox must never submit a stale ImageId to a renderer");
}

void test_public_drawing_metrics_and_control_tag() {
    std::weak_ptr<int> released_anchor;
    {
        auto tagged = make_control<Panel>(StableId("controls.tagged"));
        auto anchor = std::make_shared<int>(42);
        released_anchor = anchor;
        tagged->set_tag(anchor);
        anchor.reset();
        const auto* retained =
            std::any_cast<std::shared_ptr<int>>(&tagged->tag());
        require(retained != nullptr && **retained == 42,
                "Control Tag must retain exact type-erased application metadata");
        tagged->dispose();
    }
    require(released_anchor.expired(),
            "control disposal must release Tag-owned application lifetime");

    auto root = make_control<Panel>(StableId("controls.diagnostics.root"));
    auto drawing = make_control<DrawingSurface>(
        StableId("controls.diagnostics.drawing"));
    drawing->set_requested_bounds({0.0, 0.0, 120.0, 60.0});
    drawing->set_accessible_name("Public owner drawing");
    std::uint64_t callback_count = 0U;
    Rect callback_bounds;
    Rect callback_damage;
    drawing->set_paint_callback(
        [&](Painter& painter, Rect bounds, Rect damage) {
            ++callback_count;
            callback_bounds = bounds;
            callback_damage = damage;
            painter.fill_rect(bounds, Color::rgba(1, 2, 3));
        });
    auto metrics = make_control<MetricsView>(
        StableId("controls.diagnostics.metrics"), "Runtime proof");
    metrics->set_requested_bounds({0.0, 60.0, 240.0, 90.0});
    root->add_child(drawing);
    root->add_child(metrics);
    Window window(root, {240.0, 150.0});
    window.perform_layout();
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 240.0, 150.0});
    require(callback_count == 1U &&
                callback_bounds == Rect{0.0, 0.0, 120.0, 60.0} &&
                !callback_damage.empty() && painter.fills >= 4U,
            "DrawingSurface must invoke public owner paint with retained bounds and damage");
    require(!drawing->hit_test_visible() &&
                !drawing->hit_test_local({10.0, 10.0}) &&
                drawing->semantic_descriptor().role == SemanticRole::image &&
                drawing->semantic_descriptor().exposed,
            "DrawingSurface must default to input-transparent image semantics");
    const SemanticDescriptor metrics_semantics = metrics->semantic_descriptor();
    require(metrics_semantics.role == SemanticRole::group &&
                metrics_semantics.name == "Runtime proof" &&
                metrics_semantics.value.find("controls") != std::string::npos,
            "MetricsView must expose the structured runtime snapshot semantically");
}

} // namespace

int main() {
    try {
        test_public_controls_render_with_role_policy();
        test_button_pointer_and_keyboard_activation();
        test_checkbox_state_and_click_order();
        test_radio_group_scope_and_order();
        test_link_and_callback_disposal();
        test_wrong_thread_property_mutation_is_rejected();
        test_label_multiline_wrapping_and_alignment();
        test_picture_box_modes_registry_and_semantics();
        test_public_drawing_metrics_and_control_tag();
        std::cout << "gui_forms_basic_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_basic_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
