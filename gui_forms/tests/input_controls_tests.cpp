#include "gui_forms/input_controls.hpp"
#include "gui_forms/window.hpp"
#include "headless_host.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using namespace gui_forms;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

class RecordingPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect rect, Color color) override {
        if (color == Color::rgba(38, 114, 185) && rect.width > 0.0) {
            saw_selection = true;
        }
    }
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(Point, Point, Color, double) override { ++lines; }
    void draw_text_utf8(Point, std::string_view text, FontSpec, Color) override {
        painted_text += std::string(text);
    }
    void draw_image(ImageId, Rect, double) override {}

    bool saw_selection{};
    std::uint64_t lines{};
    std::string painted_text;
};

void test_unicode_editing_selection_and_history() {
    auto field = make_control<TextBox>(StableId("input.unicode"),
                                       "a\xCC\x81" "bc");
    field->set_requested_bounds({0.0, 0.0, 180.0, 30.0});
    Window window(field, {180.0, 30.0});
    require(window.request_focus(field), "TextBox must accept retained focus");

    std::string order;
    auto text = field->text_changed().subscribe(
        [&order](const std::string&) { order += "text\n"; });
    auto selection = field->selection_changed().subscribe(
        [&order](const TextSelection&) { order += "selection\n"; });

    field->select(Utf8Offset(0U), Utf8Offset(3U));
    require(field->selected_text() == "a\xCC\x81",
            "TextBox selection must preserve a combining grapheme");
    require(field->replace_selection("Ω") && field->text() == "Ωbc" &&
                order == "selection\ntext\nselection\n" && field->can_undo(),
            "TextBox replacement must publish text then collapsed selection");
    require(field->undo() && field->text() == "a\xCC\x81" "bc" &&
                field->can_redo(),
            "TextBox undo must restore Unicode text and directional selection");
    require(field->redo() && field->text() == "Ωbc",
            "TextBox redo must restore the replacement");
}

void test_keyboard_text_input_and_read_only() {
    auto field = make_control<TextBox>(StableId("input.keyboard"), "alpha");
    field->set_requested_bounds({0.0, 0.0, 180.0, 30.0});
    Window window(field, {180.0, 30.0});
    require(window.request_focus(field), "TextBox keyboard fixture must focus");

    KeyEvent home{KeyAction::down, PhysicalKey::home};
    require(window.dispatch_key(home) && field->selection().caret == Utf8Offset(0U),
            "Home must move the TextBox caret to the first boundary");
    KeyEvent right{KeyAction::down, PhysicalKey::right, Modifier::shift};
    require(window.dispatch_key(right) && field->selected_text() == "a",
            "Shift+Right must extend TextBox selection by one grapheme");
    require(window.dispatch_text({"Ω"}) && field->text() == "Ωlpha",
            "normalized text input must replace the selected range");

    KeyEvent select_all{KeyAction::down, PhysicalKey::a, Modifier::control};
    require(window.dispatch_key(select_all) &&
                field->selected_text() == field->text(),
            "command+A must select all TextBox text");
    KeyEvent erase{KeyAction::down, PhysicalKey::backspace};
    require(window.dispatch_key(erase) && field->text().empty(),
            "Backspace must delete the active TextBox selection");
    KeyEvent undo{KeyAction::down, PhysicalKey::z, Modifier::control};
    require(window.dispatch_key(undo) && field->text() == "Ωlpha",
            "command+Z must use public TextBox history");

    field->set_read_only(true);
    require(!window.dispatch_text({"blocked"}) && field->text() == "Ωlpha",
            "read-only TextBox must reject normalized text mutation");
}

void test_pointer_drag_paint_and_caret_deadline() {
    auto field = make_control<TextBox>(StableId("input.pointer"), "abcdef");
    field->set_requested_bounds({0.0, 0.0, 180.0, 30.0});
    Window window(field, {180.0, 30.0});
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 180.0, 30.0});
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary,
                                     {7.0, 15.0}}) &&
                field->has_pointer_capture(),
            "TextBox primary down must focus and capture for selection");
    require(window.dispatch_pointer({PointerAction::move, PointerButton::none,
                                     {35.0, 15.0}}) &&
                !field->selection().empty(),
            "TextBox captured drag must continuously extend selection");
    require(window.dispatch_pointer({PointerAction::up, PointerButton::primary,
                                     {35.0, 15.0}}) &&
                !field->has_pointer_capture(),
            "TextBox pointer release must commit selection and release capture");

    RecordingPainter selected;
    window.paint(selected, {0.0, 0.0, 180.0, 30.0});
    require(selected.saw_selection && selected.painted_text.find("abcdef") !=
                std::string::npos,
            "focused TextBox must paint selected and unselected glyph passes");
    require(window.next_wake().has_value(),
            "focused TextBox caret must use a bounded scheduled deadline");
    const FrameTime deadline = *window.next_wake();
    const FramePollResult poll = window.poll_frame_schedule(deadline);
    require(poll.deadlines_fired == 1U && window.next_wake().has_value(),
            "TextBox caret deadline must repaint and schedule the next blink");
}

void test_placeholder_and_validation() {
    auto field = make_control<TextBox>(StableId("input.placeholder"));
    field->set_placeholder_text("Type a local path");
    field->set_requested_bounds({0.0, 0.0, 180.0, 30.0});
    Window window(field, {180.0, 30.0});
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 180.0, 30.0});
    require(painter.painted_text == "Type a local path",
            "empty TextBox must paint its placeholder without mutating text");

    bool invalid = false;
    try {
        field->select(Utf8Offset(1U), Utf8Offset(1U));
    } catch (const std::out_of_range&) {
        invalid = true;
    }
    require(invalid, "TextBox must reject selections outside its text store");
}

void test_word_navigation_and_deletion() {
    auto field = make_control<TextBox>(StableId("input.words"),
                                       "alpha  日本語, bravo");
    field->set_requested_bounds({0.0, 0.0, 260.0, 30.0});
    Window window(field, {260.0, 30.0});
    require(window.request_focus(field), "word-navigation fixture must focus");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::home}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                     Modifier::control}) &&
                field->selection().caret == Utf8Offset(7U),
            "Ctrl+Right must cross one word and its following spaces");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::control | Modifier::shift}) &&
                field->selected_text() == "日本語",
            "Ctrl+Shift+Right must extend by one Unicode word without splitting graphemes");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::control}) &&
                field->selection().empty() &&
                field->selection().caret == Utf8Offset(16U),
            "unshifted word navigation must first collapse a directional selection");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::right,
                                 Modifier::alt}) &&
                field->selection().caret == Utf8Offset(18U),
            "Alt+Right must share the deterministic cross-platform word policy");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::end}) &&
                window.dispatch_key({KeyAction::down, PhysicalKey::backspace,
                                     Modifier::control}) &&
                field->text() == "alpha  日本語, ",
            "Ctrl+Backspace must remove a complete word as one undoable edit");
    require(field->undo() && field->text() == "alpha  日本語, bravo",
            "word deletion must participate in ordinary TextBox history");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::left,
                                 Modifier::meta}) &&
                field->selection().caret == Utf8Offset(0U),
            "Meta+Left must provide deterministic single-line start navigation");
}

void test_clipboard_commands_and_protected_text() {
    auto root = make_control<Panel>(StableId("input.clipboard.root"));
    auto field = make_control<TextBox>(StableId("input.clipboard"), "alpha bravo");
    field->set_requested_bounds({0.0, 0.0, 240.0, 30.0});
    root->add_child(field);
    auto password = make_control<TextBox>(StableId("input.password"),
                                          "sëcret🚀");
    password->set_requested_bounds({0.0, 40.0, 240.0, 30.0});
    password->set_accessible_name("Protected credential");
    password->set_use_system_password_character(true);
    root->add_child(password);
    Window window(root, {260.0, 90.0});
    host::HeadlessHost host(window);
    HostServices& services = host.services();

    require(window.request_focus(field), "clipboard fixture must focus TextBox");
    field->select(Utf8Offset(0U), Utf8Offset(5U));
    require(window.dispatch_key({KeyAction::down, PhysicalKey::c,
                                 Modifier::control}),
            "Ctrl+C must be consumed by the focused TextBox");
    HostClipboardTextResult clipboard = services.read_clipboard_text();
    require(clipboard.status.accepted() && clipboard.has_text &&
                clipboard.text_utf8 == "alpha",
            "copy must use the portable HostServices clipboard seam");

    field->select(Utf8Offset(6U), Utf8Offset(11U));
    require(window.dispatch_key({KeyAction::down, PhysicalKey::x,
                                 Modifier::meta}) && field->text() == "alpha ",
            "Cmd/Ctrl+X must copy then remove the selected range");
    require(services.write_clipboard_text("Ω paste").accepted(),
            "clipboard fixture must install Unicode paste content");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::v,
                                 Modifier::control}) &&
                field->text() == "alpha Ω paste",
            "Ctrl+V must replace selection from portable UTF-8 clipboard data");
    field->set_read_only(true);
    require(window.dispatch_key({KeyAction::down, PhysicalKey::v,
                                 Modifier::control}) &&
                field->text() == "alpha Ω paste",
            "read-only paste must be consumed without mutating text");

    require(services.write_clipboard_text("sentinel").accepted() &&
                window.request_focus(password),
            "protected-field fixture must retain a sentinel clipboard");
    password->select_all();
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 260.0, 90.0});
    require(painter.painted_text.find("sëcret🚀") == std::string::npos &&
                painter.painted_text.find("•") != std::string::npos,
            "protected TextBox paint must emit mask glyphs and never secret text");
    require(window.dispatch_key({KeyAction::down, PhysicalKey::c,
                                 Modifier::meta}) && !password->copy(),
            "protected copy must be consumed and reject programmatic export");
    clipboard = services.read_clipboard_text();
    require(clipboard.text_utf8 == "sentinel",
            "protected copy must leave existing clipboard content untouched");
    const SemanticDescriptor descriptor = password->semantic_descriptor();
    const std::string semantic_json = window.semantic_snapshot().to_json();
    require(descriptor.value.empty() &&
                has_semantic_state(descriptor.states,
                                   SemanticState::protected_content) &&
                semantic_json.find("sëcret🚀") == std::string::npos,
            "protected semantics must identify protection without leaking the value");
    password->set_use_system_password_character(false);
    password->set_password_character(U'*');
    RecordingPainter custom_mask;
    window.paint(custom_mask, {0.0, 0.0, 260.0, 90.0});
    require(custom_mask.painted_text.find("*******") != std::string::npos,
            "custom password character must preserve one mask per grapheme");
    bool invalid_mask = false;
    try {
        password->set_password_character(static_cast<char32_t>(0xd800U));
    } catch (const std::invalid_argument&) {
        invalid_mask = true;
    }
    require(invalid_mask,
            "password mask must reject invalid Unicode scalar values");
}

void test_list_box_selection_navigation_and_mutation() {
    auto list = make_control<ListBox>(StableId("input.list"));
    list->set_items({"Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot"});
    list->set_requested_bounds({0.0, 0.0, 180.0, 86.0});
    Window window(list, {180.0, 86.0});
    require(window.request_focus(list), "ListBox must accept retained focus");
    std::size_t changes{};
    std::size_t activations{};
    auto changed = list->selection_changed().subscribe(
        [&changes](const ListSelectionChange&) { ++changes; });
    auto activated = list->item_activated().subscribe(
        [&activations](std::size_t index) { activations += index + 1U; });

    list->select_index(1U);
    KeyEvent down{KeyAction::down, PhysicalKey::down};
    require(window.dispatch_key(down) && list->selected_index() == 2U &&
                changes == 2U,
            "ListBox Down must select the next item exactly once");
    KeyEvent enter{KeyAction::down, PhysicalKey::enter};
    require(window.dispatch_key(enter) && activations == 3U,
            "ListBox Enter must activate its active item");

    list->set_selection_mode(ListSelectionMode::multiple_extended);
    list->select_index(4U, true, false);
    require(list->selected_indices().size() == 3U &&
                list->selected_indices().front() == 2U &&
                list->selected_indices().back() == 4U,
            "extended ListBox selection must include its deterministic anchor range");
    list->select_index(3U, false, true);
    require(list->selected_indices().size() == 2U &&
                std::find(list->selected_indices().begin(),
                          list->selected_indices().end(), 3U) ==
                    list->selected_indices().end(),
            "command-toggle must remove one item from a multi-selection");

    list->remove_item(0U);
    require(list->selected_indices().front() == 1U &&
                list->selected_indices().back() == 3U,
            "ListBox collection mutation must remap retained selection indices");
    list->set_top_index(2U);
    RecordingPainter painter;
    window.paint(painter, {0.0, 0.0, 180.0, 86.0});
    require(painter.painted_text.find("Delta") != std::string::npos &&
                painter.painted_text.find("Bravo") == std::string::npos,
            "ListBox top index must bound virtualized row painting");
}

void test_combo_box_popup_commit_dismiss_and_owner_revocation() {
    auto root = make_control<Panel>(StableId("combo.root"));
    auto combo = make_control<ComboBox>(StableId("combo.field"));
    combo->set_items({"Low latency", "Balanced", "High fidelity", "Archive"});
    combo->set_placeholder_text("Choose a profile");
    combo->set_requested_bounds({20.0, 20.0, 220.0, 32.0});
    root->add_child(combo);
    Window window(root, {300.0, 220.0});
    require(window.request_focus(combo), "ComboBox must accept retained focus");
    std::string order;
    auto selection = combo->selected_index_changed().subscribe(
        [&order](std::optional<std::size_t>) { order += "selection\n"; });
    auto dropdown = combo->drop_down_changed().subscribe(
        [&order](bool open) { order += open ? "open\n" : "close\n"; });

    combo->set_dropped_down(true);
    require(combo->dropped_down() && window.focus_scope_depth() == 1U &&
                window.find("combo.field.popup.list"),
            "ComboBox must publish a retained overlay and contained focus scope");
    const auto popup = window.find("combo.field.popup.list");
    window.perform_layout();
    const Rect bounds = popup->absolute_bounds();
    const Point third{bounds.x + 20.0, bounds.y + 2.0 + 2.0 * 26.0 + 13.0};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary, third}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary, third}) &&
                combo->selected_index() == 2U && !combo->dropped_down() &&
                window.focus_scope_depth() == 0U &&
                !window.find("combo.field.popup.layer") &&
                order == "open\nselection\nclose\n",
            "ComboBox popup activation must commit, close, restore focus, and detach overlay");

    combo->set_dropped_down(true);
    const Point outside{5.0, 5.0};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary, outside}) &&
                !combo->dropped_down(),
            "ComboBox popup layer must dismiss an outside pointer press");

    combo->set_dropped_down(true);
    combo->set_visible(false);
    require(!combo->dropped_down() && !window.find("combo.field.popup.layer") &&
                window.focus_scope_depth() == 0U,
            "popup controller must revoke a ComboBox overlay when its owner becomes ineligible");
}

void test_numeric_up_down_composite_edit_spinner_and_keys() {
    auto numeric = make_control<NumericUpDown>(StableId("input.numeric"));
    numeric->set_range(-10.0, 10.0);
    numeric->set_increment(0.5);
    numeric->set_decimal_places(1U);
    numeric->set_value(1.5);
    numeric->set_requested_bounds({0.0, 0.0, 180.0, 32.0});
    Window window(numeric, {180.0, 32.0});
    window.perform_layout();
    require(numeric->editor() && numeric->editor()->text() == "1.5",
            "NumericUpDown must expose its public TextBox editor and formatted value");
    std::string values;
    auto changed = numeric->value_changed().subscribe(
        [&values](double value) { values += std::to_string(value) + "\n"; });
    require(window.request_focus(numeric->editor()),
            "NumericUpDown editor must accept retained focus");
    KeyEvent up{KeyAction::down, PhysicalKey::up};
    require(window.dispatch_key(up) && numeric->value() == 2.0 &&
                numeric->editor()->text() == "2.0",
            "NumericUpDown parent preview must step before TextBox caret routing");

    numeric->editor()->select_all();
    require(window.dispatch_text({"7.5"}) && numeric->value() == 7.5,
            "NumericUpDown must commit a complete in-range editor value");
    numeric->editor()->select_all();
    require(window.dispatch_text({"99"}) && numeric->value() == 7.5,
            "NumericUpDown must preserve value during an out-of-range intermediate edit");
    KeyEvent enter{KeyAction::down, PhysicalKey::enter};
    require(window.dispatch_key(enter) && numeric->editor()->text() == "7.5",
            "NumericUpDown Enter must reject and restore an invalid editor value");

    const auto spinner = window.find("input.numeric.spinner");
    const Rect spin = spinner->absolute_bounds();
    const Point lower{spin.x + spin.width * 0.5, spin.y + spin.height * 0.75};
    require(window.dispatch_pointer({PointerAction::down, PointerButton::primary, lower}) &&
                window.dispatch_pointer({PointerAction::up, PointerButton::primary, lower}) &&
                numeric->value() == 7.0,
            "NumericUpDown spinner lower button must decrement by the public increment");
    numeric->set_hexadecimal(true);
    numeric->set_value(10.0);
    require(numeric->editor()->text() == "A" && values.find("7.000000") != std::string::npos,
            "NumericUpDown hexadecimal formatting must retain ordered value events");
}

} // namespace

int main() {
    try {
        test_unicode_editing_selection_and_history();
        test_keyboard_text_input_and_read_only();
        test_pointer_drag_paint_and_caret_deadline();
        test_placeholder_and_validation();
        test_word_navigation_and_deletion();
        test_clipboard_commands_and_protected_text();
        test_list_box_selection_navigation_and_mutation();
        test_combo_box_popup_commit_dismiss_and_owner_revocation();
        test_numeric_up_down_composite_edit_spinner_and_keys();
        std::cout << "gui_forms_input_controls_tests: all tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "gui_forms_input_controls_tests: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
