#include "gui_forms/gui_forms.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

class RecordingPainter final : public Painter {
public:
    void save() override { ++commands; }
    void restore() override { ++commands; }
    void translate(Point) override { ++commands; }
    void clip_rect(Rect) override { ++commands; }
    void fill_rect(Rect, Color) override { ++fills; ++commands; }
    void stroke_rect(Rect, Color, double) override { ++strokes; ++commands; }
    void draw_line(Point, Point, Color, double) override { ++commands; }
    void draw_text_utf8(Point, std::string_view text, FontSpec, Color) override {
        texts.emplace_back(text);
        ++commands;
    }
    void draw_image(ImageId, Rect, double) override { ++commands; }

    std::uint64_t commands{};
    std::uint64_t fills{};
    std::uint64_t strokes{};
    std::vector<std::string> texts;
};

struct Fixture final {
    std::shared_ptr<TabControl> tabs;
    std::shared_ptr<TabPage> first;
    std::shared_ptr<TabPage> second;
    std::shared_ptr<TabPage> third;
    std::shared_ptr<Button> first_button;
    std::shared_ptr<Button> second_button;
};

Fixture make_fixture() {
    Fixture value;
    value.tabs = make_control<TabControl>(StableId("tabs"));
    value.tabs->set_accessible_name("Workspace tabs");
    value.tabs->set_requested_bounds({0.0, 0.0, 480.0, 260.0});
    value.first = make_control<TabPage>(StableId("tabs.first"), "Overview");
    value.second = make_control<TabPage>(StableId("tabs.second"), "Details");
    value.third = make_control<TabPage>(StableId("tabs.third"), "Diagnostics");
    value.first_button = make_control<Button>(StableId("tabs.first.button"),
                                               "First page action");
    value.second_button = make_control<Button>(StableId("tabs.second.button"),
                                                "Second page action");
    value.first_button->set_requested_bounds({20.0, 20.0, 160.0, 32.0});
    value.second_button->set_requested_bounds({20.0, 20.0, 170.0, 32.0});
    value.first->add_child(value.first_button);
    value.second->add_child(value.second_button);
    auto third_label = make_control<Label>(StableId("tabs.third.label"),
                                            "Third page content");
    third_label->set_requested_bounds({20.0, 20.0, 180.0, 30.0});
    value.third->add_child(third_label);
    value.tabs->add_page(value.first);
    value.tabs->add_page(value.second);
    value.tabs->add_page(value.third);
    return value;
}

void test_page_ownership_layout_and_visibility() {
    Fixture fixture = make_fixture();
    Window window(fixture.tabs, {480.0, 260.0});
    window.perform_layout();
    require(fixture.tabs->page_count() == 3U &&
                fixture.tabs->selected_index() == 0U &&
                fixture.first->visible() && !fixture.second->visible() &&
                !fixture.third->visible(),
            "TabControl must own pages and expose exactly one selected page");
    require(fixture.tabs->display_bounds() == Rect{0.0, 29.0, 480.0, 231.0} &&
                fixture.first->committed_arranged_bounds() ==
                    fixture.tabs->display_bounds(),
            "top-aligned TabControl must reserve a deterministic header strip");

    TabSelectionChange observed;
    std::uint64_t changes{};
    auto token = fixture.tabs->selected_index_changed().subscribe(
        [&observed, &changes](const TabSelectionChange& change) {
            observed = change;
            ++changes;
        });
    fixture.tabs->set_selected_index(1U);
    window.perform_layout();
    require(changes == 1U && observed.old_index == 0U &&
                observed.new_index == 1U && !fixture.first->visible() &&
                fixture.second->visible(),
            "TabControl selection must mutate visibility before one ordered event");
    require(window.hit_test({30.0, 70.0}) != fixture.first_button,
            "a hidden TabPage subtree must not remain hit-testable");

    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 480.0, 260.0});
    require(std::find(painter.texts.begin(), painter.texts.end(), "Overview") !=
                painter.texts.end() &&
                std::find(painter.texts.begin(), painter.texts.end(), "Details") !=
                painter.texts.end() &&
                std::find(painter.texts.begin(), painter.texts.end(),
                          "First page action") == painter.texts.end() &&
                std::find(painter.texts.begin(), painter.texts.end(),
                          "Second page action") != painter.texts.end(),
            "TabControl must paint every header and only the selected page content");
}

void test_keyboard_pointer_and_focus_restoration() {
    Fixture fixture = make_fixture();
    Window window(fixture.tabs, {480.0, 260.0});
    window.perform_layout();
    require(window.request_focus(fixture.first_button),
            "first page child must receive initial focus");
    KeyEvent next{KeyAction::down, PhysicalKey::tab, Modifier::control};
    require(window.dispatch_key(next) && fixture.tabs->selected_index() == 1U &&
                window.focused_control() == fixture.tabs,
            "Ctrl+Tab from page content must switch pages and retain safe focus");
    require(window.request_focus(fixture.second_button),
            "second page child must receive focus");
    KeyEvent previous{KeyAction::down, PhysicalKey::tab,
                      Modifier::control | Modifier::shift};
    require(window.dispatch_key(previous) && fixture.tabs->selected_index() == 0U &&
                window.focused_control() == fixture.first_button,
            "Ctrl+Shift+Tab must restore the destination page's remembered focus");

    const Rect third = fixture.tabs->tab_bounds(2U);
    const Point click{third.x + third.width * 0.5, third.y + third.height * 0.5};
    require(window.dispatch_pointer(
                {PointerAction::down, PointerButton::primary, click}) &&
                fixture.tabs->selected_index() == 2U &&
                window.focused_control() == fixture.tabs,
            "pointer header activation must select and focus the tab strip");
    KeyEvent left{KeyAction::down, PhysicalKey::left};
    require(window.dispatch_key(left) && fixture.tabs->selected_index() == 1U,
            "focused horizontal tabs must traverse with arrow keys");
    KeyEvent home{KeyAction::down, PhysicalKey::home};
    require(window.dispatch_key(home) && fixture.tabs->selected_index() == 0U,
            "focused tabs must support Home/End boundary traversal");
}

void test_alignment_appearance_and_semantics() {
    Fixture fixture = make_fixture();
    Window window(fixture.tabs, {480.0, 260.0});
    window.perform_layout();
    const std::array<std::pair<TabAlignment, Rect>, 4> alignments{{
        {TabAlignment::top, {0.0, 29.0, 480.0, 231.0}},
        {TabAlignment::bottom, {0.0, 0.0, 480.0, 231.0}},
        {TabAlignment::left, {119.0, 0.0, 361.0, 260.0}},
        {TabAlignment::right, {0.0, 0.0, 361.0, 260.0}},
    }};
    for (const auto& [alignment, expected] : alignments) {
        fixture.tabs->set_alignment(alignment);
        window.perform_layout();
        require(fixture.tabs->display_bounds() == expected,
                "TabControl alignment must reserve the correct content edge");
    }
    for (const TabAppearance appearance :
         {TabAppearance::normal, TabAppearance::buttons,
          TabAppearance::flat_buttons}) {
        fixture.tabs->set_appearance(appearance);
        RecordingPainter painter;
        window.paint(painter, {0.0, 0.0, 480.0, 260.0});
        require(painter.fills >= 5U && painter.strokes >= 5U,
                "every TabControl appearance must produce a complete retained frame");
    }

    const SemanticSnapshot before = window.semantic_snapshot();
    const std::string json = before.to_json();
    require(json.find("\"role\":\"tab_group\"") != std::string::npos &&
                json.find("\"role\":\"tab\"") != std::string::npos &&
                json.find("tabs.third.tab") != std::string::npos,
            "TabControl must publish a tab group and stable virtual tab identities");
    require(window.perform_semantic_action("tabs.third.tab",
                                           SemanticAction::select) &&
                fixture.tabs->selected_index() == 2U,
            "semantic tab selection must execute the same public selection path");
}

void test_page_removal_and_disposal_reconciliation() {
    Fixture fixture = make_fixture();
    Window window(fixture.tabs, {480.0, 260.0});
    fixture.tabs->set_selected_index(1U);
    const auto removed = fixture.tabs->remove_page(*fixture.second);
    window.perform_layout();
    require(removed == fixture.second && !removed->attached() &&
                fixture.tabs->page_count() == 2U &&
                fixture.tabs->selected_tab() == fixture.third &&
                fixture.tabs->selected_index() == 1U,
            "removing a selected page must choose its surviving successor");

    fixture.third->dispose();
    window.perform_layout();
    require(fixture.tabs->page_count() == 1U &&
                fixture.tabs->selected_tab() == fixture.first &&
                fixture.tabs->selected_index() == 0U && fixture.first->visible(),
            "direct selected-page disposal must reconcile without stale ownership");
}

} // namespace

int main() {
    try {
        test_page_ownership_layout_and_visibility();
        test_keyboard_pointer_and_focus_restoration();
        test_alignment_appearance_and_semantics();
        test_page_removal_and_disposal_reconciliation();
        std::cout << "gui_forms_tab_control_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_tab_control_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
