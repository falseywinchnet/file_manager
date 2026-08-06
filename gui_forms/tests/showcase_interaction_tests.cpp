#include "demo/showcase.hpp"
#include "gui_forms/gui_forms.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class CountingPainter final : public Painter {
public:
    void save() override { ++commands; }
    void restore() override { ++commands; }
    void translate(Point) override { ++commands; }
    void clip_rect(Rect) override { ++commands; }
    void fill_rect(Rect, Color) override { ++commands; }
    void stroke_rect(Rect, Color, double) override { ++commands; }
    void draw_line(Point, Point, Color, double) override { ++commands; }
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override { ++commands; }
    void draw_image(ImageId, Rect, double) override {
        ++commands;
        ++images;
    }
    std::uint64_t commands{};
    std::uint64_t images{};
};

Point center(const Control::Ptr& control) {
    const Rect bounds = control->absolute_bounds();
    return {bounds.x + bounds.width * 0.5, bounds.y + bounds.height * 0.5};
}

void click(Window& window, const Control::Ptr& control) {
    if (!control) throw std::runtime_error("showcase click target is missing");
    const Point point = center(control);
    const bool down = window.dispatch_pointer(
        {PointerAction::down, PointerButton::primary, point});
    const bool up = window.dispatch_pointer(
        {PointerAction::up, PointerButton::primary, point});
    if (!down || !up) {
        const Rect bounds = control->absolute_bounds();
        const Control::Ptr hit = window.hit_test(point);
        const Control::Ptr captured = window.captured_control();
        throw std::runtime_error("showcase click failed for " +
                                 std::string(control->stable_id().value()) +
                                 " (down=" + (down ? "true" : "false") +
                                 ", up=" + (up ? "true" : "false") +
                                 ", bounds=" + std::to_string(bounds.x) + "," +
                                 std::to_string(bounds.y) + "," +
                                 std::to_string(bounds.width) + "," +
                                 std::to_string(bounds.height) +
                                 ", hit=" + (hit ? std::string(hit->stable_id().value())
                                                : std::string("none")) +
                                 ", eligible=" +
                                 (control->eligible_for_input() ? "true" : "false") +
                                 ", visible=" +
                                 (control->effectively_visible() ? "true" : "false") +
                                 ", enabled=" +
                                 (control->effectively_enabled() ? "true" : "false") +
                                 ", captured=" +
                                 (captured ? std::string(captured->stable_id().value())
                                           : std::string("none")) +
                                 ")");
    }
}

void select_page(Window& window, std::size_t index) {
    const Control::Ptr navigation = window.find(
        "showcase.navigation." + std::to_string(index));
    require(navigation != nullptr, "showcase navigation stable ID must resolve");
    click(window, navigation);
    for (std::size_t page = 0; page < 12U; ++page) {
        const Control::Ptr surface = window.find("showcase.page." + std::to_string(page));
        require(surface && surface->visible() == (page == index),
                "showcase navigation must expose exactly one retained page");
    }
}

void test_page_contract_and_rendering() {
    std::unique_ptr<Window> owned = showcase::make_showcase();
    Window& window = *owned;
    window.perform_layout();
    require(window.metrics_snapshot().control_count >= 140U,
            "complete showcase must retain a broad public control population");
    require(window.find("showcase.text.primary") &&
                window.find("showcase.collections.single.list") &&
                window.find("showcase.animation.easing") &&
                window.find("showcase.containers.split") &&
                window.find("showcase.images.picture.4"),
            "showcase must publish stable IDs for every proving family");

    CountingPainter painter;
    window.paint(painter, {0.0, 0.0, 1280.0, 820.0});
    require(painter.commands > 150U,
            "showcase must generate a substantial retained painter command surface");
    for (std::size_t page = 0; page < 12U; ++page) select_page(window, page);
}

void test_timing_and_tooltip_runtime() {
    std::unique_ptr<Window> owned = showcase::make_showcase();
    Window& window = *owned;
    select_page(window, 11U);
    auto status = std::dynamic_pointer_cast<Label>(
        window.find("showcase.timing.tick.status"));
    auto progress = std::dynamic_pointer_cast<ProgressBar>(
        window.find("showcase.timing.progress"));
    require(status && progress && window.next_wake().has_value(),
            "timing page must activate its public UI Timer without a hidden control");
    const std::string before = status->text();
    const FrameTime timer_deadline = *window.next_wake();
    const FramePollResult timer_tick = window.poll_frame_schedule(timer_deadline);
    require(timer_tick.ui_timer_ticks == 1U && status->text() != before &&
                progress->value() > 0.0,
            "showcase Timer tick must update retained public controls on the UI queue");
    click(window, window.find("showcase.timing.stop"));
    require(!window.next_wake().has_value() &&
                status->text().find("zero idle wakeups") != std::string::npos,
            "showcase Timer stop must make the window scheduler quiescent");

    const Control::Ptr hover_target = window.find("showcase.timing.tooltip.hover");
    const Rect hover_bounds = hover_target->absolute_bounds();
    const Point hover_point{hover_bounds.x + 12.0, hover_bounds.y + 12.0};
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, hover_point}));
    require(window.next_wake().has_value(),
            "showcase hover target must arm the public ToolTip delay");
    static_cast<void>(window.poll_frame_schedule(*window.next_wake()));
    window.perform_layout();
    require(window.semantic_snapshot().to_json().find("\"role\":\"tool_tip\"") !=
                std::string::npos,
            "showcase ToolTip must materialize as a retained semantic overlay");
    static_cast<void>(window.dispatch_pointer(
        {PointerAction::move, PointerButton::none, {1000.0, 700.0}}));
    require(window.semantic_snapshot().to_json().find("\"role\":\"tool_tip\"") ==
                std::string::npos,
            "showcase ToolTip must cancel immediately when hover leaves its owner");
    click(window, window.find("showcase.timing.tooltip.show-disabled"));
    require(window.semantic_snapshot().to_json().find(
                "This action is disabled because no compatible device is selected") !=
                std::string::npos,
            "showcase must explain a disabled target through passive popup ownership");
}

void test_showcase_semantic_surface() {
    std::unique_ptr<Window> owned = showcase::make_showcase();
    Window& window = *owned;
    const SemanticSnapshot snapshot = window.semantic_snapshot();
    require(snapshot.node_count >= 30U && !snapshot.roots.empty(),
            "visible showcase page must publish a substantial semantic surface");
    require(snapshot.to_json().find("showcase.controls.button.0") != std::string::npos &&
                snapshot.to_json().find("\"role\":\"check_box\"") != std::string::npos &&
                snapshot.to_json().find("\"role\":\"radio_button\"") != std::string::npos,
            "showcase semantics must include distinct stock control roles and stable IDs");
}

void test_ranges_containers_and_animation() {
    std::unique_ptr<Window> owned = showcase::make_showcase();
    Window& window = *owned;
    select_page(window, 1U);
    require(window.semantic_snapshot().to_json().find("\"role\":\"scroll_bar\"") !=
                std::string::npos,
            "showcase range page must publish native-neutral scrollbar semantics");
    auto slider = std::dynamic_pointer_cast<TrackBar>(
        window.find("showcase.ranges.slider.1"));
    auto progress = std::dynamic_pointer_cast<ProgressBar>(
        window.find("showcase.ranges.progress.1"));
    auto horizontal_scroll = std::dynamic_pointer_cast<HScrollBar>(
        window.find("showcase.ranges.scroll.horizontal"));
    auto vertical_scroll = std::dynamic_pointer_cast<VScrollBar>(
        window.find("showcase.ranges.scroll.vertical"));
    require(slider && progress && horizontal_scroll && vertical_scroll,
            "range proving controls must retain public slider, progress, and scrollbar types");
    const Rect track = slider->absolute_bounds();
    const Point from{track.x + track.width * 0.2, track.y + track.height * 0.5};
    const Point to{track.x + track.width * 0.82, track.y + track.height * 0.5};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary, from}) &&
                window.dispatch_pointer({PointerAction::move, PointerButton::none, to}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary, to}) &&
                slider->value() > 75.0 && progress->value() == slider->value(),
            "showcase TrackBar drag must update linked ProgressBar continuously");
    const Rect scroll_bounds = horizontal_scroll->absolute_bounds();
    const Rect scroll_thumb = horizontal_scroll->thumb_bounds();
    const Point scroll_from{scroll_bounds.x + scroll_thumb.x + scroll_thumb.width * 0.5,
                            scroll_bounds.y + scroll_thumb.height * 0.5};
    const Point scroll_to{scroll_bounds.x + horizontal_scroll->track_bounds().x +
                              horizontal_scroll->track_bounds().width - 2.0,
                          scroll_from.y};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary,
                                     scroll_from}) &&
                window.dispatch_pointer({PointerAction::move, PointerButton::none,
                                         scroll_to}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary,
                                         scroll_to}) &&
                horizontal_scroll->value() > 95.0 &&
                vertical_scroll->value() == horizontal_scroll->value(),
            "showcase ScrollBar drag must track continuously and synchronize orientations");

    select_page(window, 2U);
    auto split = std::dynamic_pointer_cast<SplitContainer>(
        window.find("showcase.containers.split"));
    click(window, window.find("showcase.containers.collapse"));
    require(split && split->first_collapsed(),
            "showcase collapse command must mutate the public SplitContainer");
    click(window, window.find("showcase.containers.collapse"));
    require(!split->first_collapsed(),
            "showcase SplitContainer must restore its retained allocation");

    select_page(window, 3U);
    require(window.next_wake().has_value(),
            "visible animation page must publish a retained frame deadline");
    click(window, window.find("showcase.animation.pause"));
    require(!window.next_wake().has_value(),
            "paused animation page must become fully quiescent");
}

void test_text_collections_and_popup_lifecycle() {
    std::unique_ptr<Window> owned = showcase::make_showcase();
    Window& window = *owned;
    select_page(window, 5U);
    auto text = std::dynamic_pointer_cast<TextBox>(window.find("showcase.text.primary"));
    require(text && window.request_focus(text) && window.dispatch_text({"dogfood Ω"}) &&
                text->text() == "dogfood Ω" && text->can_undo(),
            "showcase public TextBox must accept normalized Unicode input");
    click(window, window.find("showcase.text.command.0"));
    click(window, window.find("showcase.text.command.1"));
    require(text->text() == "GUI.Forms",
            "showcase TextBox commands must operate on the public selection surface");
    click(window, window.find("showcase.text.command.2"));
    require(text->text() == "dogfood Ω",
            "showcase TextBox Undo command must restore its prior snapshot");

    select_page(window, 6U);
    auto multi = std::dynamic_pointer_cast<ListBox>(
        window.find("showcase.collections.multi.list"));
    require(multi && multi->selected_indices().size() == 4U,
            "showcase extended ListBox must retain its initial selection range");
    auto combo = std::dynamic_pointer_cast<ComboBox>(
        window.find("showcase.collections.combo.density"));
    click(window, combo);
    require(combo->dropped_down() && window.focus_scope_depth() == 1U &&
                window.find("showcase.collections.combo.density.popup.list"),
            "showcase ComboBox must open through the public popup controller");
    const auto popup = window.find("showcase.collections.combo.density.popup.list");
    window.perform_layout();
    const Rect bounds = popup->absolute_bounds();
    const Point choice{bounds.x + 20.0, bounds.y + 2.0 + 2.0 * 26.0 + 13.0};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary, choice}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary, choice}) &&
                combo->selected_index() == 2U && !combo->dropped_down() &&
                window.focus_scope_depth() == 0U,
            "showcase ComboBox popup must commit and restore focus without residue");

    select_page(window, 7U);
    auto numeric = std::dynamic_pointer_cast<NumericUpDown>(
        window.find("showcase.values.numeric.1"));
    require(numeric && window.request_focus(numeric->editor()),
            "showcase NumericUpDown must expose a focusable public editor");
    KeyEvent up{KeyAction::down, PhysicalKey::up};
    require(window.dispatch_key(up) && numeric->value() == 4.0 &&
                numeric->editor()->text() == "4.00",
            "showcase NumericUpDown key step must synchronize value and editor");

    select_page(window, 8U);
    auto picture = std::dynamic_pointer_cast<PictureBox>(
        window.find("showcase.images.picture.4"));
    require(picture && picture->has_valid_image() &&
                picture->size_mode() == PictureBoxSizeMode::zoom &&
                picture->image_size() == Size{128.0, 80.0},
            "showcase image page must use the public PictureBox and image registry");
    const std::string image_semantics = window.semantic_snapshot().to_json();
    require(image_semantics.find("\"role\":\"image\"") != std::string::npos &&
                image_semantics.find("128 x 80") != std::string::npos,
            "showcase image page must publish image semantics and intrinsic size");
    CountingPainter image_painter;
    window.paint(image_painter, {0.0, 0.0, 1280.0, 820.0});
    require(image_painter.images == 8U,
            "showcase image page must render all eight public PictureBox consumers");

    select_page(window, 9U);
    auto tabs = std::dynamic_pointer_cast<TabControl>(
        window.find("showcase.tabs.primary"));
    require(tabs && tabs->page_count() == 4U && tabs->selected_index() == 1U,
            "showcase tab page must retain a real public TabControl page model");
    const Rect security = tabs->tab_bounds(2U);
    const Rect tabs_absolute = tabs->absolute_bounds();
    const Point security_click{tabs_absolute.x + security.x + security.width * 0.5,
                               tabs_absolute.y + security.y + security.height * 0.5};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary,
                                     security_click}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary,
                                         security_click}) &&
                tabs->selected_index() == 2U &&
                window.find("showcase.tabs.primary.security.radio")->effectively_visible(),
            "showcase tabs must select pages through public pointer behavior");
    require(window.perform_semantic_action(
                "showcase.tabs.primary.diagnostics.tab", SemanticAction::select) &&
                tabs->selected_index() == 3U,
            "showcase tabs must share semantic and pointer selection behavior");

    select_page(window, 10U);
    auto checked = std::dynamic_pointer_cast<CheckedListBox>(
        window.find("showcase.checked.immediate.list"));
    require(checked && checked->check_on_click() &&
                checked->item_check_state(3U) == CheckState::indeterminate,
            "showcase checked page must retain a public check-state collection");
    const Rect checked_bounds = checked->absolute_bounds();
    const Point last_row{checked_bounds.x + 14.0,
                         checked_bounds.y + 2.0 + 5.5 * checked->item_height()};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary,
                                     last_row}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary,
                                         last_row}) &&
                checked->item_checked(5U),
            "showcase CheckOnClick row must select and toggle immediately");
    const std::string checked_semantics = window.semantic_snapshot().to_json();
    require(checked_semantics.find("\"role\":\"check_list_item\"") !=
                std::string::npos,
            "showcase checked rows must publish checkable virtual semantics");
}

} // namespace

int main() {
    try {
        test_page_contract_and_rendering();
        test_showcase_semantic_surface();
        test_ranges_containers_and_animation();
        test_text_collections_and_popup_lifecycle();
        test_timing_and_tooltip_runtime();
        std::cout << "gui_forms_showcase_interaction_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_showcase_interaction_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
