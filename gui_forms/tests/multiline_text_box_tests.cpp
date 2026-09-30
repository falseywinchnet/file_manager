#include "gui_forms/input_controls.hpp"
#include "gui_forms/window.hpp"
#include "headless_host.hpp"
#include "support/typography_painter.hpp"
#include "support/named_callbacks.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

namespace {
using namespace gui_forms;
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

class Metrics final : public TextMetricsProvider {
public:
    ResolvedTextLayout resolve_text_layout_utf8(std::string_view text, FontSpec font) override {
        ++calls;
        bytes += text.size();
        ResolvedTextLayout result = estimate_text_layout_utf8(text, font);
        const TextStore store(text);
        result.logical_size = {store.grapheme_count().value() * advance, line_height};
        result.ascent = ascent;
        result.descent = line_height - ascent;
        result.line_gap = 0.0;
        return result;
    }
    std::size_t calls{}, bytes{};
    double advance{10.0}, line_height{20.0}, ascent{15.0};
};

struct Fixture {
    std::shared_ptr<TextBox> field = make_control<TextBox>(StableId("multiline"));
    Window window{field, {110, 68}};
    Metrics metrics;
    test_support::TypographyPainter painter;
    Fixture() {
        field->set_requested_bounds({0, 0, 110, 68});
        field->set_multiline(true);
        window.set_text_metrics_provider(&metrics);
        window.perform_layout();
        require(window.request_focus(field), "focus");
    }
    void key(std::uint32_t key, Modifier modifier = Modifier::none) {
        require(window.dispatch_key({KeyAction::down, key, modifier}), "key must be handled");
    }
    void caret(std::size_t position) { field->select(Utf8Offset(position), Utf8Offset(position)); }
    std::size_t caret() const { return field->selection().caret.value(); }
    void paint() { field->on_paint(painter, {0, 0, 110, 68}); }
};

void exact_editing() {
    Fixture f;
    const std::string source = "alpha\r\nbeta\rgamma\n";
    f.field->set_text(source);
    require(f.field->text() == source && f.field->visual_line_count() == 4, "preserve mixed line endings");
    f.field->set_text("a\xC2\x85" "b\xE2\x80\xA8" "c\xE2\x80\xA9");
    require(f.field->visual_line_count() == 4, "TextStore Unicode line separators");
    f.field->set_text(source);
    f.field->set_newline_sequence("\r\n");
    f.caret(5);
    f.key(PhysicalKey::enter);
    require(f.field->text() == "alpha\r\n\r\nbeta\rgamma\n" && f.caret() == 7, "configured Enter");
    f.key(PhysicalKey::z, Modifier::control);
    require(f.field->text() == source && f.caret() == 5, "undo newline and selection");
    f.key(PhysicalKey::y, Modifier::control);
    require(f.caret() == 7, "redo newline");
    f.field->set_accepts_tab(true);
    f.key(PhysicalKey::tab);
    require(f.field->text().substr(7, 1) == "\t", "literal Tab");
    f.field->set_read_only(true);
    const std::string before(f.field->text());
    f.key(PhysicalKey::enter);
    f.key(PhysicalKey::tab);
    require(f.field->text() == before, "read-only Enter/Tab");

    f.field->set_read_only(false);
    f.field->set_text("\rX");
    f.caret(1);
    require(f.field->replace_selection("\n") && f.caret() == 2, "CRLF merge caret remains valid");
    f.key(PhysicalKey::backspace);
    require(f.field->text() == "X", "delete complete CRLF grapheme");
    f.field->set_text("a\n\xCC\x81");
    f.field->select(Utf8Offset(1), Utf8Offset(2));
    require(f.field->delete_selection() && f.caret() == 3, "deletion joining combining mark snaps caret");
}

void navigation() {
    Fixture f;
    f.field->set_text("abcdef\nx\nabcdef\nlast\n");
    f.caret(4);
    f.key(PhysicalKey::down);
    require(f.caret() == 8, "down clamps short line");
    f.key(PhysicalKey::down);
    require(f.caret() == 13, "down retains preferred x across short line");
    f.key(PhysicalKey::home);
    require(f.caret() == 9, "Home visual line start");
    f.key(PhysicalKey::end, Modifier::shift);
    require(f.field->selected_text() == "abcdef", "Shift End selection");
    f.key(PhysicalKey::home, Modifier::control);
    require(f.caret() == 0, "Ctrl Home document start");
    f.key(PhysicalKey::page_down);
    require(f.caret() == 16, "PageDown uses visible rows");
    f.key(PhysicalKey::page_up);
    require(f.caret() == 0, "PageUp uses visible rows");
    f.key(PhysicalKey::end, Modifier::control | Modifier::shift);
    require(f.field->selected_text() == f.field->text(), "Ctrl Shift End document selection");
    f.paint();
    require(f.field->scroll_offset().y > 0, "caret selection scrolls into view");
    f.window.dispatch_pointer({PointerAction::wheel, PointerButton::none, {20,20}, {0,1000}});
    f.paint();
    require(f.field->scroll_offset().y == 0, "wheel scroll persists despite offscreen caret");

    f.field->set_text("a\xCC\x81\xF0\x9F\x9A\x80\r\nabc");
    f.caret(7);
    f.key(PhysicalKey::left);
    require(f.caret() == 3, "emoji left boundary");
    f.key(PhysicalKey::left);
    require(f.caret() == 0, "combining grapheme left boundary");
    f.key(PhysicalKey::down);
    require(f.caret() == 9, "vertical Unicode line navigation");
}

void wrapping_metrics_and_hit_testing() {
    Fixture f;
    f.field->set_word_wrap(true);
    f.field->set_text("one two three four");
    require(f.field->visual_line_count() == 2, "word wrap splits at whitespace");
    f.field->arrange({0, 0, 60, 68});
    require(f.field->visual_line_count() > 2, "resize invalidates retained wrap geometry");
    f.field->arrange({0, 0, 110, 68});
    f.caret(0);
    f.key(PhysicalKey::end);
    require(f.caret() == 8, "End reaches soft wrap boundary");
    f.key(PhysicalKey::home);
    require(f.caret() == 0, "soft wrap upstream affinity survives End then Home");
    f.key(PhysicalKey::down);
    require(f.caret() == 8, "down enters wrapped row");
    f.paint();
    const std::size_t calls = f.metrics.calls;
    f.paint();
    require(f.metrics.calls == calls, "repaint reuses cached metrics");
    require(f.window.dispatch_pointer({PointerAction::down, PointerButton::primary, {26,29}}), "row pointer down");
    require(f.caret() == 10, "hit test uses y and provider width");
    require(f.window.dispatch_pointer({PointerAction::move, PointerButton::none, {46,9}}), "cross-row drag");
    require(f.field->selected_text() == "two th", "cross-row pointer selection uses exact bytes");
    require(f.window.dispatch_pointer({PointerAction::up, PointerButton::primary, {46,9}}), "pointer release");
    f.field->set_word_wrap(false);
    require(f.field->visual_line_count() == 1 && f.field->text() == "one two three four", "wrap toggle preserves exact source");
    f.key(PhysicalKey::end, Modifier::control);
    f.paint();
    require(f.field->scroll_offset().x > 0, "unwrapped caret horizontal reveal");
    f.field->set_word_wrap(true);
    f.paint();
    require(f.field->scroll_offset().x == 0, "wrap resets horizontal offset");
    FontSpec font = f.field->font();
    font.size += 2;
    f.field->set_font(font);
    f.paint();
    require(f.metrics.calls > calls, "font change remeasures");
    f.field->set_text(std::string(300, 'a'));
    f.field->set_word_wrap(false);
    f.caret(0);
    f.painter.texts.clear();
    f.paint();
    require(f.painter.texts.size() == 1 && f.painter.texts.front().text.size() == 300,
        "long shaping run must not be arbitrarily split");
    const std::string joining = "العربية العربية العربية العربية العربية العربية العربية العربية";
    f.field->set_text(joining);
    f.caret(0);
    f.painter.texts.clear();
    f.paint();
    require(f.painter.texts.size() == 1 && f.painter.texts.front().text == joining,
        "joining-script painter input remains one complete run");
}

void clipboard_and_limits() {
    Fixture f;
    host::HeadlessHost host(f.window);
    f.field->set_text("first\r\nsecond");
    f.field->select(Utf8Offset(3), Utf8Offset(10));
    f.key(PhysicalKey::x, Modifier::control);
    require(f.field->text() == "firond" && host.services().read_clipboard_text().text_utf8 == "st\r\nsec", "cross-line cut");
    f.key(PhysicalKey::v, Modifier::control);
    require(f.field->text() == "first\r\nsecond", "exact clipboard paste");
    require(TextBox::validate_multiline_text(std::string(4097, 'x')) == TextBox::MultilineValidation::line_too_long,
        "queryable long-line refusal");
    require(TextBox::validate_multiline_text(std::string(1048577, '\n')) == TextBox::MultilineValidation::document_too_large,
        "queryable document refusal");
    require(TextBox::validate_multiline_text("\xFF") == TextBox::MultilineValidation::invalid_utf8,
        "queryable invalid UTF8 refusal");
    const std::string before(f.field->text());
    const TextSelection selected = f.field->selection();
    bool rejected{};
    try { f.field->set_text(std::string(4097, 'x')); } catch (const std::invalid_argument&) { rejected = true; }
    require(rejected && f.field->text() == before && f.field->selection() == selected, "set_text refusal atomicity");
    require(!f.field->replace_selection(std::string(4097, 'x')) && f.field->text() == before &&
        f.field->selection() == selected, "edit refusal atomicity");
    require(!f.field->on_semantic_action(SemanticAction::set_value, std::string(4097, 'x')) &&
        f.field->selection() == selected, "semantic refusal preserves selection");
    require(f.field->undo() && f.field->text() == "firond", "refused edit leaves undo unchanged");
}

void dpi_transition_remeasures_same_provider() {
    Fixture f;
    f.field->set_word_wrap(true);
    f.field->set_text("abcdefghijklmnopqrst");
    f.caret(0);
    f.paint();
    require(f.field->visual_line_count() == 2, "initial DPI wrap geometry");
    const auto calls = f.metrics.calls;
    const FontSpec font = f.field->font();
    const Rect bounds = f.field->arranged_bounds();

    // Native providers stay at the same address across a DPI change; rounded
    // device font metrics can produce different advances in logical units.
    f.metrics.advance = 20.0;
    f.metrics.line_height = 30.0;
    f.metrics.ascent = 23.0;
    f.window.set_scale(1.5);
    f.window.perform_layout();
    f.painter.texts.clear();
    f.paint();
    require(f.metrics.calls > calls, "DPI change must invalidate same-provider multiline metrics");
    require(f.field->font() == font && f.field->arranged_bounds() == bounds &&
        f.window.text_metrics_provider() == &f.metrics, "DPI regression keeps other cache keys unchanged");
    require(f.field->visual_line_count() == 4, "DPI transition updates wrapping");
    require(f.painter.texts.size() >= 2 && f.painter.texts[0].origin.y == 27.0 &&
        f.painter.texts[1].origin.y == 57.0, "DPI transition updates ascent and row height");
    require(f.window.dispatch_pointer({PointerAction::down, PointerButton::primary, {46, 39}}), "DPI hit-test down");
    require(f.caret() == 7, "DPI hit-test uses updated row height and advances");
    require(f.window.dispatch_pointer({PointerAction::up, PointerButton::primary, {46, 39}}), "DPI hit-test up");
    f.key(PhysicalKey::end, Modifier::control);
    f.paint();
    require(f.field->scroll_offset().y == 60.0, "DPI caret reveal uses updated document height");
    const auto warm_calls = f.metrics.calls;
    f.paint();
    require(f.metrics.calls == warm_calls, "unchanged DPI repaint reuses refreshed metrics");

    f.metrics.advance = 10.0;
    f.metrics.line_height = 20.0;
    f.metrics.ascent = 15.0;
    f.window.set_scale(1.0);
    f.paint();
    require(f.field->visual_line_count() == 2 && f.field->scroll_offset().y == 0.0,
        "return DPI transition remeasures and clamps viewport");
}

void clear_history_establishes_save_boundary() {
    Fixture f;
    f.field->set_text("alpha\r\nbeta\r\ngamma\r\ndelta\r\nepsilon");
    require(f.field->replace_selection("1") && f.field->replace_selection("2") &&
        f.field->undo() && f.field->can_undo() && f.field->can_redo(),
        "save-boundary fixture has both undo and redo history");
    f.field->select(Utf8Offset(2), Utf8Offset(f.field->text().size()));
    f.field->set_read_only(true);
    f.paint();
    const std::string text(f.field->text());
    const TextSelection selection = f.field->selection();
    const Point viewport = f.field->scroll_offset();
    const Dirty dirty = f.field->dirty();
    const auto metric_calls = f.metrics.calls;
    std::size_t text_events{}, selection_events{};
    const auto text_subscription = f.field->text_changed().subscribe(
        test_support::IncrementCounter<std::size_t, const std::string&>(text_events));
    const auto selection_subscription = f.field->selection_changed().subscribe(
        test_support::IncrementCounter<std::size_t, const TextSelection&>(selection_events));

    f.field->clear_undo_history();
    f.field->clear_undo_history();
    require(!f.field->can_undo() && !f.field->can_redo(), "clear history removes both directions and is idempotent");
    require(f.field->text() == text && f.field->selection() == selection &&
        f.field->scroll_offset() == viewport && f.field->dirty() == dirty &&
        f.metrics.calls == metric_calls && text_events == 0 && selection_events == 0,
        "clearing history preserves document, selection, viewport and notifications");
    f.field->set_read_only(false);
    require(!f.field->undo() && !f.field->redo(), "old states cannot cross save boundary");
    require(f.field->replace_selection("replacement") && f.field->undo() &&
        f.field->text() == text && f.field->selection() == selection && !f.field->can_undo(),
        "next edit can undo exactly to successful-save boundary");
    require(f.field->redo() && f.field->text() != text, "redo works after new post-save edit");

    auto single = make_control<TextBox>(StableId("single.history"), "saved");
    require(single->replace_selection(" edit"), "single-line edit");
    single->clear_undo_history();
    require(single->text() == "saved edit" && !single->undo(), "single-line history boundary preserves value");
}

void bounded_workload() {
    Fixture f;
    std::string many;
    for (std::size_t i = 0; i < 10000; ++i) many += "line fixture\r\n";
    auto start = std::chrono::steady_clock::now();
    f.field->set_text(many);
    require(f.field->visual_line_count() == 10001, "10k logical lines");
    f.paint();
    const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    require(f.painter.texts.size() <= 4, "paint only visible lines");
    const auto count = f.metrics.calls;
    f.paint();
    require(f.metrics.calls == count, "large document warm repaint performs no measurements");
    std::cout << "10k lines: " << elapsed << " ms; cold metric calls=" << count << "; warm=0\n";
    f.field->set_text(std::string(4096, 'x'));
    start = std::chrono::steady_clock::now();
    require(f.field->visual_line_count() == 1, "maximum admitted line");
    std::cout << "4096-byte line cold layout: " << std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() << " ms\n";
}
}

int main() {
    try {
        exact_editing();
        navigation();
        wrapping_metrics_and_hit_testing();
        clipboard_and_limits();
        dpi_transition_remeasures_same_provider();
        clear_history_establishes_save_boundary();
        bounded_workload();
        std::cout << "gui_forms_multiline_text_box_tests: passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
