#include "gui_forms/basic_controls.hpp"
#include "gui_forms/diagnostic_controls.hpp"
#include "gui_forms/window.hpp"
#include "support/named_callbacks.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <span>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace gui_forms;
namespace callbacks = gui_forms::test_support;

constexpr std::array<std::uint8_t, 70> one_pixel_png{
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x56, 0xc7, 0x2f, 0x0d, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

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
    void clip_rounded_rect(Rect, double) override { ++rounded_clips; }
    void fill_rect(Rect, Color) override { ++fills; }
    void fill_rounded_rect(Rect, double, Color) override { ++rounded_fills; }
    void stroke_rect(Rect, Color, double) override { ++strokes; }
    void stroke_rounded_rect(Rect, double, Color, double) override {
        ++rounded_strokes;
    }
    void fill_linear_gradient(Rect, Point, Point,
                              std::span<const GradientStop>) override {
        ++gradients;
    }
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
    std::uint64_t rounded_clips{};
    std::uint64_t fills{};
    std::uint64_t rounded_fills{};
    std::uint64_t strokes{};
    std::uint64_t rounded_strokes{};
    std::uint64_t gradients{};
    std::uint64_t lines{};
    std::vector<std::string> texts;
    std::vector<FontRole> roles;
    std::vector<Point> text_origins;
    std::vector<ImageId> images;
    std::vector<Rect> image_destinations;
    std::vector<double> image_opacities;
};

class FocusSink final : public Control {
public:
    explicit FocusSink(StableId stable_id) : Control(std::move(stable_id)) {
        set_focusable(true);
    }
};

class CancelValidation final {
public:
    void operator()(ControlValidationEvent& event) const {
        event.cancel = true;
    }
};

class TraceDialogResult final {
public:
    explicit TraceDialogResult(std::string& trace) noexcept : trace_(trace) {}

    void operator()(DialogResult result) const {
        trace_ += "result:" +
            std::to_string(static_cast<unsigned>(result)) + "\n";
    }

private:
    std::string& trace_;
};

class ReplaceButtonDialogResult final {
public:
    explicit ReplaceButtonDialogResult(std::shared_ptr<Button> button) noexcept
        : button_(std::move(button)) {}

    void operator()(ButtonBase&) const {
        (*button_).set_dialog_result(DialogResult::no);
    }

private:
    std::shared_ptr<Button> button_;
};

class TraceCheckState final {
public:
    explicit TraceCheckState(std::string& trace) noexcept : trace_(trace) {}

    void operator()(CheckState value) const {
        trace_ += value == CheckState::checked
            ? "state:checked\n" : "state:other\n";
    }

private:
    std::string& trace_;
};

class TraceBoolean final {
public:
    TraceBoolean(std::string& trace, std::string_view true_text,
                 std::string_view false_text) noexcept
        : trace_(trace), true_text_(true_text), false_text_(false_text) {}

    void operator()(bool value) const {
        trace_.append(value ? true_text_ : false_text_);
    }

private:
    std::string& trace_;
    std::string_view true_text_;
    std::string_view false_text_;
};

class RecordDrawingPaint final {
public:
    RecordDrawingPaint(std::uint64_t& count, Rect& bounds, Rect& damage) noexcept
        : count_(count), bounds_(bounds), damage_(damage) {}

    void operator()(Painter& painter, Rect bounds, Rect damage) const {
        ++count_;
        bounds_ = bounds;
        damage_ = damage;
        painter.fill_rect(bounds, Color::rgba(1, 2, 3));
    }

private:
    std::uint64_t& count_;
    Rect& bounds_;
    Rect& damage_;
};

void attempt_label_mutation(const std::shared_ptr<Label>& label,
                            bool& rejected) noexcept {
    try {
        (*label).set_text("After");
    } catch (const std::logic_error&) {
        rejected = true;
    }
}

Point center(const Control::Ptr& control) {
    const Rect bounds = (*control).absolute_bounds();
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
    std::shared_ptr<gui_forms::Panel> panel = make_control<Panel>(StableId("controls.panel"));
    (*panel).set_accessible_name("Public panel group");
    (*panel).set_requested_bounds({0.0, 0.0, 360.0, 180.0});
    (*panel).set_border_style(BorderStyle::line);
    std::shared_ptr<gui_forms::GroupBox> group = make_control<GroupBox>(StableId("controls.group"), "Reusable controls");
    (*group).set_requested_bounds({10.0, 10.0, 340.0, 160.0});
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("controls.label"), "Field content");
    (*label).set_requested_bounds({12.0, 24.0, 140.0, 24.0});
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("controls.button"), "Apply");
    (*button).set_requested_bounds({12.0, 54.0, 90.0, 28.0});
    (*button).set_default_button(true);
    std::shared_ptr<gui_forms::CheckBox> check = make_control<CheckBox>(StableId("controls.check"), "Checked option");
    (*check).set_requested_bounds({12.0, 88.0, 150.0, 24.0});
    (*check).set_checked(true);
    std::shared_ptr<gui_forms::RadioButton> radio = make_control<RadioButton>(StableId("controls.radio"), "Radio option");
    (*radio).set_requested_bounds({170.0, 88.0, 150.0, 24.0});
    (*radio).set_checked(true);
    (*group).add_child(label);
    (*group).add_child(button);
    (*group).add_child(check);
    (*group).add_child(radio);
    (*panel).add_child(group);
    Window window(panel, {360.0, 180.0});
    window.perform_layout();

    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 360.0, 180.0});
    require(painter.saves == painter.restores && painter.clips >= 6 &&
                painter.gradients >= 4 && painter.rounded_clips >= 3 &&
                painter.rounded_strokes >= 3 && painter.lines >= 2 &&
                painter.texts.size() == 5,
            "public controls must render through retained painter commands");
    require(painter.roles[0] == FontRole::control &&
                painter.roles[1] == FontRole::content,
            "group/control titles and field labels must preserve typography roles");
    require(window.hit_test(center(label)) == group,
            "noninteractive Label must not intercept its logical container");
    require((*panel).semantic_descriptor().role == SemanticRole::group &&
                (*panel).semantic_descriptor().name == "Public panel group" &&
                (*panel).semantic_descriptor().exposed,
            "a named public Panel must expose stock group semantics");
}

void test_button_pointer_and_keyboard_activation() {
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("button"), "Run");
    (*button).set_requested_bounds({10.0, 10.0, 90.0, 28.0});
    Window window(button, {120.0, 60.0});
    std::uint64_t clicks = 0;
    SubscriptionToken clicked = (*button).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(clicks));

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
    require(!(*button).pressed_visual(),
            "Button keyboard visual must clear after activation");
}

void test_mnemonics_and_dialog_buttons_are_retained_commands() {
    const MnemonicText escaped = parse_mnemonic_text("Save && E&xit");
    require(escaped.display_text == "Save & Exit" &&
                escaped.mnemonic == U'x' && is_mnemonic(U'X', "E&xit") &&
                is_mnemonic(U'a', "&Save &As") &&
                !is_mnemonic(U'&', "&&&") &&
                !is_mnemonic(U'e', "Save && Exit"),
            "mnemonic parsing must collapse escaped ampersands and ASCII-fold the marked scalar");

    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("dialog.root"));
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("dialog.label"), "&Name");
    std::shared_ptr<FocusSink> field =
        make_control<FocusSink>(StableId("dialog.field"));
    std::shared_ptr<gui_forms::Button> run = make_control<Button>(StableId("dialog.run"), "&Run");
    std::shared_ptr<gui_forms::Button> run_duplicate = make_control<Button>(
        StableId("dialog.run.duplicate"), "&Retry");
    std::shared_ptr<gui_forms::Button> accept = make_control<Button>(StableId("dialog.accept"), "&Apply");
    std::shared_ptr<gui_forms::Button> cancel = make_control<Button>(StableId("dialog.cancel"), "&Cancel");
    (*label).set_requested_bounds({4.0, 4.0, 60.0, 24.0});
    (*field).set_requested_bounds({70.0, 4.0, 90.0, 24.0});
    (*run).set_requested_bounds({4.0, 36.0, 70.0, 28.0});
    (*run_duplicate).set_requested_bounds({4.0, 68.0, 70.0, 28.0});
    (*accept).set_requested_bounds({80.0, 36.0, 70.0, 28.0});
    (*cancel).set_requested_bounds({156.0, 36.0, 70.0, 28.0});
    (*cancel).set_causes_validation(false);
    (*root).add_child(label);
    (*root).add_child(field);
    (*root).add_child(run);
    (*root).add_child(run_duplicate);
    (*root).add_child(accept);
    (*root).add_child(cancel);
    Window window(root, {240.0, 104.0});
    window.perform_layout();

    std::uint64_t run_clicks{};
    std::uint64_t duplicate_clicks{};
    std::uint64_t accept_clicks{};
    std::uint64_t cancel_clicks{};
    SubscriptionToken run_token = (*run).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(run_clicks));
    SubscriptionToken duplicate_token = (*run_duplicate).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(duplicate_clicks));
    SubscriptionToken accept_token = (*accept).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(accept_clicks));
    SubscriptionToken cancel_token = (*cancel).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(cancel_clicks));

    window.set_accept_button(accept);
    window.set_cancel_button(cancel);
    require((*accept).default_button() && window.accept_button() == accept &&
                window.cancel_button() == cancel &&
                (*cancel).dialog_result() == DialogResult::cancel,
            "assigning a Window accept button must retain the target and publish its default cue");
    RecordingPainter mnemonic_painter;
    (*run).on_paint(mnemonic_painter, (*run).client_rectangle());
    require(std::find(mnemonic_painter.texts.begin(), mnemonic_painter.texts.end(),
                      "Run") != mnemonic_painter.texts.end() &&
                (*run).semantic_descriptor().name == "Run" &&
                (*label).semantic_descriptor().name == "Name",
            "mnemonic markers must not leak into Button or Label paint and semantics");

    KeyEvent mnemonic;
    mnemonic.action = KeyAction::down;
    mnemonic.physical_key = PhysicalKey::r;
    mnemonic.modifiers = Modifier::alt;
    (*run).set_causes_validation(false);
    require(window.dispatch_key(mnemonic) && run_clicks == 1U,
            "Alt plus a marked character must invoke the first eligible retained command");
    (*run_duplicate).set_causes_validation(false);
    require(window.dispatch_key(mnemonic) && duplicate_clicks == 1U &&
                window.dispatch_key(mnemonic) && run_clicks == 2U,
            "duplicate mnemonics must advance through one stable retained arbitration ring");

    mnemonic.physical_key = PhysicalKey::n;
    require(window.dispatch_key(mnemonic) && window.focused_control() == field,
            "a Label mnemonic must focus the next selectable retained control");

    SubscriptionToken rejecting =
        (*field).validating().subscribe(CancelValidation());
    (*accept).set_dialog_result(DialogResult::yes);
    std::string dialog_trace;
    SubscriptionToken accept_order = (*accept).clicked().subscribe(
        callbacks::AppendLiteral<ButtonBase&>(dialog_trace, "click\n"));
    SubscriptionToken cancel_order = (*cancel).clicked().subscribe(
        callbacks::AppendLiteral<ButtonBase&>(dialog_trace, "cancel-click\n"));
    SubscriptionToken result_order =
        window.dialog_result_changed().subscribe(TraceDialogResult(dialog_trace));
    KeyEvent enter;
    enter.action = KeyAction::down;
    enter.physical_key = PhysicalKey::enter;
    require(window.dispatch_key(enter) && accept_clicks == 0U &&
                window.focused_control() == field,
            "a rejected default command must be consumed without activation or focus theft");
    (*accept).set_causes_validation(false);
    require(window.dispatch_key(enter) && accept_clicks == 1U &&
                window.focused_control() == field &&
                window.dialog_result() == DialogResult::yes &&
                dialog_trace == "click\nresult:6\n",
            "CausesValidation false must run Click before publishing the retained dialog result");

    window.set_dialog_result(DialogResult::none);
    dialog_trace.clear();
    KeyEvent escape = enter;
    escape.physical_key = PhysicalKey::escape;
    require(window.dispatch_key(escape) && cancel_clicks == 1U &&
                window.dialog_result() == DialogResult::cancel &&
                dialog_trace == "cancel-click\nresult:2\n",
            "Escape must invoke the retained cancel command and publish Cancel after Click");

    bool invalid_result_rejected{};
    try {
        (*accept).set_dialog_result(static_cast<DialogResult>(8));
    } catch (const std::invalid_argument&) {
        invalid_result_rejected = true;
    }
    require(invalid_result_rejected,
            "DialogResult mutation must reject numeric gaps in the WinForms enum");

    std::shared_ptr<gui_forms::Panel> scope = make_control<Panel>(StableId("dialog.scope"));
    std::shared_ptr<FocusSink> scope_field =
        make_control<FocusSink>(StableId("dialog.scope.field"));
    (*scope).set_requested_bounds({228.0, 4.0, 10.0, 60.0});
    (*scope_field).set_requested_bounds({0.0, 0.0, 10.0, 10.0});
    (*scope).add_child(scope_field);
    (*root).add_child(scope);
    const FocusScopeId scope_id = window.begin_focus_scope(scope, scope_field);
    mnemonic.physical_key = PhysicalKey::r;
    require(!window.dispatch_key(mnemonic) && run_clicks == 2U &&
                !window.dispatch_key(enter) && accept_clicks == 1U,
            "an active contained focus scope must suppress outer mnemonics and default commands");
    require(window.end_focus_scope(scope_id),
            "dialog command focus-scope fixture must close deterministically");

    std::shared_ptr<gui_forms::Button> replacement = make_control<Button>(StableId("dialog.replacement"), "Other");
    (*replacement).set_requested_bounds({4.0, 68.0, 70.0, 10.0});
    (*root).add_child(replacement);
    window.set_accept_button(replacement);
    require(!(*accept).default_button() && (*replacement).default_button(),
            "moving the accept role must revoke the old default cue exactly once");
    (*replacement).dispose();
    require(!window.accept_button(),
            "disposing a dialog target must synchronously clear its retained Window role");

    window.set_dialog_result(DialogResult::none);
    (*accept).set_dialog_result(DialogResult::yes);
    SubscriptionToken replace_result_during_click =
        (*accept).clicked().subscribe(ReplaceButtonDialogResult(accept));
    require((*accept).perform_click() &&
                window.dialog_result() == DialogResult::no,
            "Button must publish the post-Click DialogResult chosen by application code");

    const DialogKeySnapshot snapshot = window.dialog_key_snapshot();
    require(snapshot.mnemonic_attempts == 5U && snapshot.mnemonics_handled == 4U &&
                snapshot.accept_attempts == 3U && snapshot.accept_handled == 1U &&
                snapshot.cancel_attempts == 1U && snapshot.cancel_handled == 1U &&
                snapshot.command_rejections == 1U &&
                snapshot.mnemonic_candidates == 7U &&
                snapshot.mnemonic_collisions == 3U &&
                snapshot.mnemonic_cycles == 2U,
            "dialog-key instrumentation must distinguish attempts, activations, and validation rejection");
}

void test_checkbox_state_and_click_order() {
    std::shared_ptr<gui_forms::CheckBox> check = make_control<CheckBox>(StableId("check"), "Precise");
    (*check).set_requested_bounds({0.0, 0.0, 140.0, 24.0});
    Window window(check, {140.0, 24.0});
    std::string order;
    SubscriptionToken state =
        (*check).check_state_changed().subscribe(TraceCheckState(order));
    SubscriptionToken checked = (*check).checked_changed().subscribe(
        TraceBoolean(order, "checked:true\n", "checked:false\n"));
    SubscriptionToken clicked = (*check).clicked().subscribe(
        callbacks::AppendLiteral<ButtonBase&>(order, "click\n"));

    click(window, check);
    require((*check).checked() &&
                order == "state:checked\nchecked:true\nclick\n",
            "CheckBox must mutate, publish state, publish checked, then click");

    (*check).set_three_state(true);
    order.clear();
    (*check).on_activate();
    require((*check).check_state() == CheckState::indeterminate &&
                order == "state:other\nchecked:false\nclick\n",
            "three-state CheckBox must deterministically advance to indeterminate");
    order.clear();
    (*check).on_activate();
    require((*check).check_state() == CheckState::unchecked &&
                order == "state:other\nclick\n",
            "indeterminate CheckBox must return to unchecked without duplicate bool change");
}

void test_radio_group_scope_and_order() {
    std::shared_ptr<gui_forms::GroupBox> group = make_control<GroupBox>(StableId("radio.group"), "Mode");
    (*group).set_requested_bounds({0.0, 0.0, 360.0, 70.0});
    std::shared_ptr<gui_forms::RadioButton> first = make_control<RadioButton>(StableId("radio.first"), "First");
    std::shared_ptr<gui_forms::RadioButton> second = make_control<RadioButton>(StableId("radio.second"), "Second");
    std::shared_ptr<gui_forms::RadioButton> independent = make_control<RadioButton>(StableId("radio.other"), "Other group");
    (*first).set_requested_bounds({10.0, 24.0, 100.0, 24.0});
    (*second).set_requested_bounds({120.0, 24.0, 100.0, 24.0});
    (*independent).set_requested_bounds({230.0, 24.0, 120.0, 24.0});
    (*first).set_group_name("mode");
    (*second).set_group_name("mode");
    (*independent).set_group_name("independent");
    (*group).add_child(first);
    (*group).add_child(second);
    (*group).add_child(independent);
    (*first).set_checked(true);
    (*independent).set_checked(true);
    Window window(group, {360.0, 70.0});

    std::string order;
    SubscriptionToken first_changed = (*first).checked_changed().subscribe(
        TraceBoolean(order, "first:on\n", "first:off\n"));
    SubscriptionToken second_changed = (*second).checked_changed().subscribe(
        TraceBoolean(order, "second:on\n", "second:off\n"));
    SubscriptionToken second_clicked = (*second).clicked().subscribe(
        callbacks::AppendLiteral<ButtonBase&>(order, "second:click\n"));
    click(window, second);
    require(!(*first).checked() && (*second).checked() && (*independent).checked() &&
                order == "first:off\nsecond:on\nsecond:click\n",
            "RadioButton must uncheck its group peer before checking/clicking itself");
}

void test_link_and_callback_disposal() {
    std::shared_ptr<gui_forms::LinkLabel> link = make_control<LinkLabel>(StableId("link"), "Show details");
    (*link).set_requested_bounds({0.0, 0.0, 120.0, 24.0});
    Window link_window(link, {120.0, 24.0});
    std::uint64_t clicks = 0;
    SubscriptionToken link_click = (*link).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(clicks));
    click(link_window, link);
    require((*link).visited() && clicks == 1,
            "LinkLabel must mark visited before publishing activation");

    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("dispose.root"));
    std::shared_ptr<gui_forms::CheckBox> check = make_control<CheckBox>(StableId("dispose.check"), "Dispose safely");
    (*check).set_requested_bounds({0.0, 0.0, 140.0, 24.0});
    (*root).add_child(check);
    Window window(root, {140.0, 24.0});
    std::uint64_t later_callbacks = 0;
    SubscriptionToken dispose_on_state = (*check).check_state_changed().subscribe(
        callbacks::DisposeCapturedControl<CheckBox, CheckState>(check));
    SubscriptionToken checked = (*check).checked_changed().subscribe(
        callbacks::IncrementCounter<std::uint64_t, bool>(later_callbacks));
    SubscriptionToken clicked = (*check).clicked().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ButtonBase&>(later_callbacks));
    (*check).on_activate();
    require(!(*check).is_alive() && later_callbacks == 0 &&
                !window.find("dispose.check"),
            "state callback disposal must suppress later checked/click callbacks safely");
}

void test_wrong_thread_property_mutation_is_rejected() {
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("thread.label"), "Before");
    Window window(label, {100.0, 24.0});
    bool rejected = false;
    std::thread worker(attempt_label_mutation, std::cref(label),
                       std::ref(rejected));
    worker.join();
    require(rejected && (*label).text() == "Before",
            "reusable control properties must retain core UI-thread enforcement");
}

void test_fixed_label_text_is_paint_only() {
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("label.telemetry"), "0 kB/s");
    (*label).set_requested_bounds({0.0, 0.0, 120.0, 24.0});
    Window window(label, {120.0, 24.0});
    window.perform_layout();
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 120.0, 24.0});
    window.reset_activity_metrics();

    (*label).set_text("999 kB/s");
    const DamageRegion damage = window.take_damage();
    require(!damage.empty(), "fixed label text must request repaint");
    window.paint(painter, damage.bounds());
    const MetricsSnapshot fixed = window.metrics_snapshot();
    require(fixed.measure_passes == 0U && fixed.arrange_passes == 0U &&
                fixed.paint_passes == 1U,
            "fixed label text must repaint without remeasuring its ancestors");

    (*label).set_auto_size(true);
    window.perform_layout();
    window.reset_activity_metrics();
    (*label).set_text("content-sized telemetry");
    const DamageRegion auto_damage = window.take_damage();
    require(!auto_damage.empty(), "auto-sized label text must request repaint");
    window.paint(painter, auto_damage.bounds());
    const MetricsSnapshot sized = window.metrics_snapshot();
    require(sized.measure_passes > 0U && sized.arrange_passes > 0U,
            "auto-sized label text must continue to participate in layout");
}

void test_label_multiline_wrapping_and_alignment() {
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("label.multiline"),
                                     "Retained labels wrap words\nand preserve breaks");
    (*label).set_requested_bounds({0.0, 0.0, 120.0, 80.0});
    (*label).set_text_wrapping(TextWrapping::word);
    (*label).set_vertical_alignment(VerticalAlignment::near);
    (*label).set_alignment(HorizontalAlignment::far);
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
        (*label).set_line_spacing(0.1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && (*label).line_spacing() == 1.25,
            "Label must reject invalid line spacing without mutation");
}

void test_basic_control_layout_customization_is_bounded_and_atomic() {
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("label.maximum-lines"),
                                     "one two three four five six seven eight");
    (*label).set_text_wrapping(TextWrapping::word);
    const Size unrestricted = (*label).measure({72.0, 200.0});
    (*label).set_maximum_lines(2U);
    const Size limited = (*label).measure({72.0, 200.0});
    require((*label).maximum_lines() == 2U && limited.height < unrestricted.height,
            "Label MaximumLines must bound content-sized desired height");
    (*label).set_requested_bounds({0.0, 0.0, 72.0, 80.0});
    Window label_window(label, {72.0, 80.0});
    RecordingPainter label_painter;
    label_window.paint(label_painter, {0.0, 0.0, 72.0, 80.0});
    require(label_painter.texts.size() == 2U,
            "Label MaximumLines must bound actual retained painting");

    bool rejected_lines{};
    try {
        (*label).set_maximum_lines(4097U);
    } catch (const std::out_of_range&) {
        rejected_lines = true;
    }
    bool rejected_alignment{};
    try {
        (*label).set_alignment(static_cast<HorizontalAlignment>(255));
    } catch (const std::invalid_argument&) {
        rejected_alignment = true;
    }
    require(rejected_lines && rejected_alignment &&
                (*label).maximum_lines() == 2U &&
                (*label).alignment() == HorizontalAlignment::near,
            "Label bounded and enum properties must reject invalid values atomically");

    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("button.content-padding"), "Run");
    const Size default_size = (*button).measure({300.0, 200.0});
    (*button).set_content_padding({20.0, 14.0, 20.0, 14.0});
    const Size padded_size = (*button).measure({300.0, 200.0});
    require(padded_size.width > default_size.width &&
                padded_size.height > default_size.height,
            "ButtonBase ContentPadding must participate in desired size");
    bool rejected_padding{};
    try {
        (*button).set_content_padding({20.0, 14.0, 129.0, 14.0});
    } catch (const std::invalid_argument&) {
        rejected_padding = true;
    }
    require(rejected_padding &&
                (*button).content_padding() == Insets{20.0, 14.0, 20.0, 14.0},
            "ButtonBase ContentPadding must reject invalid insets atomically");

    std::shared_ptr<gui_forms::Panel> panel = make_control<Panel>(StableId("panel.border-validation"));
    bool rejected_border{};
    try {
        (*panel).set_border_style(static_cast<BorderStyle>(255));
    } catch (const std::invalid_argument&) {
        rejected_border = true;
    }
    require(rejected_border && (*panel).border_style() == BorderStyle::none,
            "Panel must reject invalid border-style states atomically");
}

void test_label_inherits_theme_typography_until_explicitly_overridden() {
    ThemeDefinition definition = windows_professional_theme_definition();
    definition.id = "label-type-roles";
    definition.structure.typography.heading =
        {FontRole::control, 17.0, 650, false, 0.31};
    definition.structure.typography.title =
        {FontRole::control, 23.0, 700, false, 0.19};
    constexpr Color themed_text = Color::rgba(21, 47, 73);
    definition.roles[static_cast<std::size_t>(ControlVisualRole::panel)]
        .ordinary[static_cast<std::size_t>(ControlSurfaceState::normal)].text =
        themed_text;
    std::shared_ptr<gui_forms::Label> label = make_control<Label>(StableId("label.theme"), "Heading");
    (*label).set_theme_override(Theme::create(std::move(definition)));
    (*label).set_text_style_role(TextStyleRole::heading);
    require(!(*label).has_font_override() &&
                (*label).font() == FontSpec{FontRole::control, 17.0, 650, false,
                                          0.31} &&
                !(*label).has_foreground_override() &&
                (*label).foreground() == themed_text,
            "Label must resolve typography and text color from inherited theme roles");

    const FontSpec explicit_font{FontRole::content, 11.0, 500, true};
    (*label).set_font(explicit_font);
    (*label).set_text_style_role(TextStyleRole::title);
    require((*label).has_font_override() && (*label).font() == explicit_font,
            "explicit Label font must remain authoritative across role changes");
    (*label).clear_font();
    require(!(*label).has_font_override() &&
                (*label).font() == FontSpec{FontRole::control, 23.0, 700, false,
                                          0.19},
            "clearing Label font must restore inherited role typography");

    constexpr Color explicit_text = Color::rgba(91, 37, 19);
    (*label).set_foreground(explicit_text);
    require((*label).foreground() == explicit_text,
            "explicit Label foreground must override theme text");
    (*label).clear_foreground();
    require((*label).foreground() == themed_text,
            "clearing Label foreground must restore inherited theme text");
}

void test_picture_box_modes_registry_and_semantics() {
    std::shared_ptr<gui_forms::PictureBox> picture = make_control<PictureBox>(StableId("picture"));
    (*picture).set_requested_bounds({0.0, 0.0, 100.0, 100.0});
    (*picture).set_border_style(BorderStyle::line);
    (*picture).set_accessible_name("Professional test card");
    Window window(picture, {100.0, 100.0});
    const std::vector<std::byte> pixels(4U * 2U * 4U, std::byte{0xff});
    const ImageLoadResult loaded = window.load_bgra32_premultiplied(
        4U, 2U, 16U, pixels);
    require(static_cast<bool>(loaded),
            "PictureBox test image must enter the window-owned registry");
    std::uint64_t changes{};
    SubscriptionToken changed = (*picture).image_changed().subscribe(
        callbacks::IncrementCounter<std::uint64_t, ImageId>(changes));
    (*picture).set_image(loaded.image);
    window.perform_layout();
    require(changes == 1U && (*picture).has_valid_image() &&
                (*picture).image_size() == Size{4.0, 2.0},
            "PictureBox must publish an image change and resolve registry metadata");

    const std::array<std::pair<PictureBoxSizeMode, Rect>, 4> modes{{
        {PictureBoxSizeMode::normal, {1.0, 1.0, 4.0, 2.0}},
        {PictureBoxSizeMode::stretch_image, {1.0, 1.0, 98.0, 98.0}},
        {PictureBoxSizeMode::center_image, {48.0, 49.0, 4.0, 2.0}},
        {PictureBoxSizeMode::zoom, {1.0, 25.5, 98.0, 49.0}},
    }};
    for (const std::pair<PictureBoxSizeMode, Rect>& mode_and_bounds : modes) {
        (*picture).set_size_mode(mode_and_bounds.first);
        require((*picture).image_bounds() == mode_and_bounds.second,
                "PictureBox sizing mode must compute deterministic image geometry");
        RecordingPainter painter;
        window.paint(painter, {0.0, 0.0, 100.0, 100.0});
        require(painter.images.size() == 1U &&
                    painter.images.front() == loaded.image &&
                    painter.image_destinations.front() == mode_and_bounds.second,
                "PictureBox must submit the resolved image and destination to Painter");
    }

    (*picture).set_size_mode(PictureBoxSizeMode::auto_size);
    require((*picture).measure({1000.0, 1000.0}) == Size{6.0, 4.0} &&
                (*picture).image_bounds() == Rect{1.0, 1.0, 4.0, 2.0},
            "PictureBox AutoSize must measure to intrinsic pixels plus its border");
    (*picture).set_image_opacity(0.42);
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
        (*picture).set_image_opacity(1.1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && (*picture).image_opacity() == 0.42,
            "PictureBox must reject invalid opacity without mutation");
    require(window.remove_image(loaded.image) && !(*picture).has_valid_image(),
            "PictureBox must reject a removed generational image ID safely");
    RecordingPainter stale_painter;
    window.paint(stale_painter, {0.0, 0.0, 100.0, 100.0});
    require(stale_painter.images.empty(),
            "PictureBox must never submit a stale ImageId to a renderer");
}

void test_public_drawing_metrics_and_control_tag() {
    std::weak_ptr<int> released_anchor;
    {
        std::shared_ptr<gui_forms::Panel> tagged = make_control<Panel>(StableId("controls.tagged"));
        std::shared_ptr<int> anchor = std::make_shared<int>(42);
        released_anchor = anchor;
        (*tagged).set_tag(anchor);
        anchor.reset();
        const std::shared_ptr<int>* retained =
            std::any_cast<std::shared_ptr<int>>(&(*tagged).tag());
        require(retained != nullptr && **retained == 42,
                "Control Tag must retain exact type-erased application metadata");
        (*tagged).dispose();
    }
    require(released_anchor.expired(),
            "control disposal must release Tag-owned application lifetime");

    std::shared_ptr<gui_forms::Panel> root = make_control<Panel>(StableId("controls.diagnostics.root"));
    std::shared_ptr<gui_forms::DrawingSurface> drawing = make_control<DrawingSurface>(
        StableId("controls.diagnostics.drawing"));
    (*drawing).set_requested_bounds({0.0, 0.0, 120.0, 60.0});
    (*drawing).set_accessible_name("Public owner drawing");
    (*drawing).set_background(Color::rgba(8, 9, 10));
    std::uint64_t callback_count = 0U;
    Rect callback_bounds;
    Rect callback_damage;
    (*drawing).set_paint_callback(
        RecordDrawingPaint(callback_count, callback_bounds, callback_damage));
    std::shared_ptr<gui_forms::MetricsView> metrics = make_control<MetricsView>(
        StableId("controls.diagnostics.metrics"), "Runtime proof");
    (*metrics).set_accent_width(9.0);
    (*metrics).set_requested_bounds({0.0, 60.0, 240.0, 90.0});
    (*root).add_child(drawing);
    (*root).add_child(metrics);
    Window window(root, {240.0, 150.0});
    window.perform_layout();
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 240.0, 150.0});
    require(callback_count == 1U &&
                callback_bounds == Rect{0.0, 0.0, 120.0, 60.0} &&
                !callback_damage.empty() &&
                painter.fills + painter.gradients >= 4U,
            "DrawingSurface must invoke public owner paint with retained bounds and damage");
    require(!(*drawing).hit_test_visible() &&
                !(*drawing).hit_test_local({10.0, 10.0}) &&
                (*drawing).background() == Color::rgba(8, 9, 10) &&
                (*drawing).semantic_descriptor().role == SemanticRole::image &&
                (*drawing).semantic_descriptor().exposed,
            "DrawingSurface must default to input-transparent image semantics");
    const SemanticDescriptor metrics_semantics = (*metrics).semantic_descriptor();
    require(metrics_semantics.role == SemanticRole::group &&
                metrics_semantics.name == "Runtime proof" &&
                metrics_semantics.value.find("controls") != std::string::npos &&
                (*metrics).accent_width() == 9.0,
            "MetricsView must expose the structured runtime snapshot semantically");
}

void test_button_disclosure_semantics() {
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("button.disclosure"), "Options");
    (*button).set_expanded_state(false);
    SemanticDescriptor collapsed = (*button).semantic_descriptor();
    require(!has_semantic_state(collapsed.states, SemanticState::expanded) &&
                std::find(collapsed.actions.begin(), collapsed.actions.end(),
                          SemanticAction::expand) != collapsed.actions.end(),
            "collapsed disclosure button must publish an expand action");
    std::size_t activations{};
    SubscriptionToken clicked = (*button).clicked().subscribe(
        callbacks::IncrementCounter<std::size_t, ButtonBase&>(activations));
    require((*button).on_semantic_action(SemanticAction::expand, {}) &&
                activations == 1U,
            "semantic expand must route through the button's shared activation");
    (*button).set_expanded_state(true);
    const SemanticDescriptor expanded = (*button).semantic_descriptor();
    require(has_semantic_state(expanded.states, SemanticState::expanded) &&
                std::find(expanded.actions.begin(), expanded.actions.end(),
                          SemanticAction::collapse) != expanded.actions.end(),
            "expanded disclosure button must publish state and collapse action");
}

void test_image_list_state_density_ownership_and_button_layout() {
    std::shared_ptr<gui_forms::Button> button = make_control<Button>(StableId("button.images"), "Launch");
    (*button).set_requested_bounds({0.0, 0.0, 150.0, 36.0});
    Window window(button, {150.0, 36.0});
    std::shared_ptr<gui_forms::ImageList> images = std::make_shared<ImageList>(window, Size{16.0, 16.0});
    const std::span<const std::byte, 70U> png =
        std::as_bytes(std::span{one_pixel_png});
    const ImageId normal_1x = (*images).add_png("Action", png, 1.0).image;
    const ImageId normal_2x = (*images).add_png("action", png, 2.0).image;
    const ImageId hot_1x =
        (*images).set_variant_png("ACTION", ImageVisualState::hot, 1.0, png).image;
    const ImageId disabled_1x = (*images).set_variant_png(
        "Action", ImageVisualState::disabled, 1.0, png).image;
    require(normal_1x.value != 0U && normal_2x.value != 0U &&
                hot_1x.value != 0U && disabled_1x.value != 0U &&
                (*images).count() == 1U && (*images).contains_key("aCtIoN") &&
                window.image_resource_snapshot().resource_count == 4U,
            "ImageList must own one case-insensitive key with independent density/state variants");

    std::uint64_t changes{};
    SubscriptionToken changed = (*images).changed().subscribe(
        callbacks::IncrementCounter<std::uint64_t,
                                    const ImageListChange&>(changes));
    (*button).set_image_list(images);
    (*button).set_image_key("ACTION");
    (*button).set_text_image_relation(TextImageRelation::image_before_text);
    (*button).set_text_alignment(ContentAlignment::middle_center);
    window.perform_layout();

    RecordingPainter normal;
    (*button).on_paint(normal, {0.0, 0.0, 150.0, 36.0});
    require(normal.images.size() == 1U && normal.images.front() == normal_1x &&
                normal.image_destinations.front().width == 16.0 &&
                normal.text_origins.front().x >
                    normal.image_destinations.front().x + 16.0,
            "Button must lay out keyed ImageList content before text using logical image size");

    PointerEvent enter;
    enter.action = PointerAction::enter;
    (*button).on_pointer(enter);
    RecordingPainter hot;
    (*button).on_paint(hot, {0.0, 0.0, 150.0, 36.0});
    require(hot.images.size() == 1U && hot.images.front() == hot_1x,
            "Button hover must resolve a real ImageList hot-state raster");

    PointerEvent leave;
    leave.action = PointerAction::leave;
    (*button).on_pointer(leave);
    window.set_scale(2.0);
    RecordingPainter dense;
    (*button).on_paint(dense, {0.0, 0.0, 150.0, 36.0});
    require(dense.images.size() == 1U && dense.images.front() == normal_2x &&
                dense.image_destinations.front().width == 16.0,
            "ImageList must select a 2x source without changing logical Button geometry");

    (*button).set_enabled(false);
    window.set_scale(1.0);
    RecordingPainter disabled;
    (*button).on_paint(disabled, {0.0, 0.0, 150.0, 36.0});
    require(disabled.images.size() == 1U &&
                disabled.images.front() == disabled_1x &&
                disabled.image_opacities.front() == 1.0,
            "Button disabled state must prefer an authored disabled raster over opacity fallback");

    std::array<std::byte, 8> malformed{};
    const std::uint64_t revision_before = (*images).revision();
    const ImageLoadResult rejected = (*images).set_variant_png(
        "Action", ImageVisualState::hot, 1.0, malformed);
    require(!rejected && (*images).revision() == revision_before &&
                (*images).resolve("action", ImageVisualState::hot, 1.0).image == hot_1x,
            "failed ImageList replacement must leave the prior variant and revision intact");

    (*images).set_key_name(0U, "Launch");
    require(changes == 1U && (*images).contains_key("LAUNCH") &&
                !(*images).contains_key("Action"),
            "ImageList key rename must be atomic, case-insensitive, and tokenized");

    std::shared_ptr<gui_forms::Panel> other_root = make_control<Panel>(StableId("button.images.other"));
    Window other_window(other_root, {10.0, 10.0});
    std::shared_ptr<gui_forms::ImageList> foreign = std::make_shared<ImageList>(other_window);
    bool rejected_foreign{};
    try {
        (*button).set_image_list(foreign);
    } catch (const std::invalid_argument&) {
        rejected_foreign = true;
    }
    require(rejected_foreign && (*button).image_list() == images,
            "Button must reject cross-Window ImageList assignment without mutation");

    (*button).set_image_key("Launch");
    (*images).dispose();
    require((*images).is_disposed() && (*images).empty() &&
                window.image_resource_snapshot().resource_count == 0U,
            "ImageList disposal must synchronously release every owned Window image");
}

} // namespace

int main() {
    try {
        test_public_controls_render_with_role_policy();
        test_button_pointer_and_keyboard_activation();
        test_mnemonics_and_dialog_buttons_are_retained_commands();
        test_checkbox_state_and_click_order();
        test_radio_group_scope_and_order();
        test_link_and_callback_disposal();
        test_wrong_thread_property_mutation_is_rejected();
        test_fixed_label_text_is_paint_only();
        test_label_multiline_wrapping_and_alignment();
        test_basic_control_layout_customization_is_bounded_and_atomic();
        test_label_inherits_theme_typography_until_explicitly_overridden();
        test_picture_box_modes_registry_and_semantics();
        test_public_drawing_metrics_and_control_tag();
        test_button_disclosure_semantics();
        test_image_list_state_density_ownership_and_button_layout();
        std::cout << "gui_forms_basic_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_basic_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
