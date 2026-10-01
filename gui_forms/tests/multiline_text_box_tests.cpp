#include "gui_forms/input_controls.hpp"
#include "gui_forms/window.hpp"
#include "headless_host.hpp"
#include "support/typography_painter.hpp"
#include "support/named_callbacks.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <optional>
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
        result.logical_size = {static_cast<double>(store.grapheme_count().value()) * advance, line_height};
        result.ascent = ascent;
        result.descent = line_height - ascent;
        result.line_gap = 0.0;
        return result;
    }
    std::size_t calls{};
    std::size_t bytes{};
    double advance{10.0};
    double line_height{20.0};
    double ascent{15.0};
};

struct Fixture {
    std::shared_ptr<TextBox> field = make_control<TextBox>(StableId("multiline"));
    TextBox& editor{*field};
    Metrics metrics{};
    test_support::TypographyPainter painter{};
    // Window borrows metrics; declaration order destroys it before the provider.
    Window window{field, {110.0, 68.0}};
    Fixture() {
        editor.set_requested_bounds({0, 0, 110, 68});
        editor.set_multiline(true);
        window.set_text_metrics_provider(&metrics);
        window.perform_layout();
        const bool focused = window.request_focus(field);
        require(focused, "focus");
    }
    void key(std::uint32_t key, Modifier modifier = Modifier::none) {
        const KeyEvent event{.action = KeyAction::down, .physical_key = key, .modifiers = modifier};
        const bool handled = window.dispatch_key(event);
        require(handled, "key must be handled");
    }
    void caret(std::size_t position) { editor.select(Utf8Offset(position), Utf8Offset(position)); }
    std::size_t caret() const {
        const TextSelection selected = editor.selection();
        const std::size_t position = selected.caret.value();
        return position;
    }
    // Dispatch one fully specified input before asserting its effect.
    void pointer(const PointerEvent& event) {
        const bool handled = window.dispatch_pointer(event);
        require(handled, "pointer event must be handled");
    }
    void paint() { editor.on_paint(painter, {0, 0, 110, 68}); }
};

void exact_editing() {
    Fixture f{};
    const std::string source = "alpha\r\nbeta\rgamma\n";
    f.editor.set_text(source);
    const std::size_t visual_count_1 = f.editor.visual_line_count();
    require(f.editor.text() == source && visual_count_1 == 4, "preserve mixed line endings");
    f.editor.set_text("a\xC2\x85" "b\xE2\x80\xA8" "c\xE2\x80\xA9");
    const std::size_t visual_count_2 = f.editor.visual_line_count();
    require(visual_count_2 == 4, "TextStore Unicode line separators");
    f.editor.set_text(source);
    f.editor.set_newline_sequence("\r\n");
    f.caret(5);
    f.key(PhysicalKey::enter);
    require(f.editor.text() == "alpha\r\n\r\nbeta\rgamma\n" && f.caret() == 7, "configured Enter");
    f.key(PhysicalKey::z, Modifier::control);
    require(f.editor.text() == source && f.caret() == 5, "undo newline and selection");
    f.key(PhysicalKey::y, Modifier::control);
    require(f.caret() == 7, "redo newline");
    f.editor.set_accepts_tab(true);
    f.key(PhysicalKey::tab);
    require(f.editor.text().substr(7, 1) == "\t", "literal Tab");
    f.editor.set_read_only(true);
    const std::string before(f.editor.text());
    f.key(PhysicalKey::enter);
    f.key(PhysicalKey::tab);
    require(f.editor.text() == before, "read-only Enter/Tab");

    f.editor.set_read_only(false);
    f.editor.set_text("\rX");
    f.caret(1);
    const bool inserted_newline = f.editor.replace_selection("\n");
    require(inserted_newline && f.caret() == 2, "CRLF merge caret remains valid");
    f.key(PhysicalKey::backspace);
    require(f.editor.text() == "X", "delete complete CRLF grapheme");
    f.editor.set_text("a\n\xCC\x81");
    f.editor.select(Utf8Offset(1), Utf8Offset(2));
    const bool deleted_newline = f.editor.delete_selection();
    require(deleted_newline && f.caret() == 3, "deletion joining combining mark snaps caret");
}

void navigation() {
    Fixture f{};
    f.editor.set_text("abcdef\nx\nabcdef\nlast\n");
    f.caret(4);
    f.key(PhysicalKey::down);
    require(f.caret() == 8, "down clamps short line");
    f.key(PhysicalKey::down);
    require(f.caret() == 13, "down retains preferred x across short line");
    f.key(PhysicalKey::home);
    require(f.caret() == 9, "Home visual line start");
    f.key(PhysicalKey::end, Modifier::shift);
    require(f.editor.selected_text() == "abcdef", "Shift End selection");
    f.key(PhysicalKey::home, Modifier::control);
    require(f.caret() == 0, "Ctrl Home document start");
    f.key(PhysicalKey::page_down);
    require(f.caret() == 16, "PageDown uses visible rows");
    f.key(PhysicalKey::page_up);
    require(f.caret() == 0, "PageUp uses visible rows");
    f.key(PhysicalKey::end, Modifier::control | Modifier::shift);
    require(f.editor.selected_text() == f.editor.text(), "Ctrl Shift End document selection");
    f.paint();
    require(f.editor.scroll_offset().y > 0, "caret selection scrolls into view");
    const PointerEvent wheel{.action = PointerAction::wheel, .position = {20.0, 20.0}, .wheel_delta = {0.0, 1000.0}};
    f.pointer(wheel);
    f.paint();
    require(f.editor.scroll_offset().y == 0, "wheel scroll persists despite offscreen caret");

    f.editor.set_text("a\xCC\x81\xF0\x9F\x9A\x80\r\nabc");
    f.caret(7);
    f.key(PhysicalKey::left);
    require(f.caret() == 3, "emoji left boundary");
    f.key(PhysicalKey::left);
    require(f.caret() == 0, "combining grapheme left boundary");
    f.key(PhysicalKey::down);
    require(f.caret() == 9, "vertical Unicode line navigation");
}

void wrapping_metrics_and_hit_testing() {
    Fixture f{};
    f.editor.set_word_wrap(true);
    f.editor.set_text("one two three four");
    const std::size_t visual_count_3 = f.editor.visual_line_count();
    require(visual_count_3 == 2, "word wrap splits at whitespace");
    f.editor.arrange({0, 0, 60, 68});
    const std::size_t visual_count_4 = f.editor.visual_line_count();
    require(visual_count_4 > 2, "resize invalidates retained wrap geometry");
    f.editor.arrange({0, 0, 110, 68});
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
    const PointerEvent press{.action = PointerAction::down, .button = PointerButton::primary, .position = {26.0, 29.0}};
    f.pointer(press);
    require(f.caret() == 10, "hit test uses y and provider width");
    const PointerEvent drag{.action = PointerAction::move, .position = {46.0, 9.0}};
    f.pointer(drag);
    require(f.editor.selected_text() == "two th", "cross-row pointer selection uses exact bytes");
    const PointerEvent release{.action = PointerAction::up, .button = PointerButton::primary, .position = {46.0, 9.0}};
    f.pointer(release);
    f.editor.set_word_wrap(false);
    const std::size_t visual_count_5 = f.editor.visual_line_count();
    require(visual_count_5 == 1 && f.editor.text() == "one two three four", "wrap toggle preserves exact source");
    f.key(PhysicalKey::end, Modifier::control);
    f.paint();
    require(f.editor.scroll_offset().x > 0, "unwrapped caret horizontal reveal");
    f.editor.set_word_wrap(true);
    f.paint();
    require(f.editor.scroll_offset().x == 0, "wrap resets horizontal offset");
    FontSpec font = f.editor.font();
    font.size += 2;
    f.editor.set_font(font);
    f.paint();
    require(f.metrics.calls > calls, "font change remeasures");
    const std::string long_run(300U, 'a');
    f.editor.set_text(long_run);
    f.editor.set_word_wrap(false);
    f.caret(0);
    f.painter.texts.clear();
    f.paint();
    require(f.painter.texts.size() == 1 && f.painter.texts.front().text.size() == 300,
        "long shaping run must not be arbitrarily split");
    const std::string joining = "العربية العربية العربية العربية العربية العربية العربية العربية";
    f.editor.set_text(joining);
    f.caret(0);
    f.painter.texts.clear();
    f.paint();
    require(f.painter.texts.size() == 1 && f.painter.texts.front().text == joining,
        "joining-script painter input remains one complete run");
}

void tab_wrap_and_reused_rows() {
    Fixture f{};
    f.editor.set_word_wrap(true);
    f.editor.set_text("a\tb");
    f.editor.arrange({0.0, 0.0, 40.0, 68.0});
    f.caret(0U);
    const std::size_t narrow_rows = f.editor.visual_line_count();
    require(narrow_rows == 3U, "oversize tab must advance on its own wrapped row");
    f.paint();
    require(f.painter.texts.size() == 2U, "tab spacing must not create a drawn text run");
    require(f.painter.texts[0].text == "a" && f.painter.texts[1].text == "b",
        "tab boundaries preserve complete adjacent text runs");
    require(f.painter.texts[1].origin.y - f.painter.texts[0].origin.y == 40.0,
        "tab-only wrapped row contributes its full height");
    f.editor.arrange({0.0, 0.0, 110.0, 68.0});
    const std::size_t wide_rows = f.editor.visual_line_count();
    require(wide_rows == 1U, "widening removes stale retained rows");
    f.editor.arrange({0.0, 0.0, 40.0, 68.0});
    const std::size_t rebuilt_rows = f.editor.visual_line_count();
    require(rebuilt_rows == narrow_rows && f.editor.text() == "a\tb",
        "narrowing again rebuilds rows without changing exact source");
}

void clipboard_and_limits() {
    Fixture f{};
    host::HeadlessHost host(f.window);
    f.editor.set_text("first\r\nsecond");
    f.editor.select(Utf8Offset(3), Utf8Offset(10));
    f.key(PhysicalKey::x, Modifier::control);
    HostServices& services = host.services();
    const HostClipboardTextResult clipboard = services.read_clipboard_text();
    require(f.editor.text() == "firond" && clipboard.text_utf8 == "st\r\nsec", "cross-line cut");
    f.key(PhysicalKey::v, Modifier::control);
    require(f.editor.text() == "first\r\nsecond", "exact clipboard paste");
    const std::string oversized_line(4097U, 'x');
    const std::string oversized_document(1048577U, '\n');
    const TextBox::MultilineValidation line_status = TextBox::validate_multiline_text(oversized_line);
    const TextBox::MultilineValidation document_status = TextBox::validate_multiline_text(oversized_document);
    const TextBox::MultilineValidation encoding_status = TextBox::validate_multiline_text("\xFF");
    require(line_status == TextBox::MultilineValidation::line_too_long,
        "queryable long-line refusal");
    require(document_status == TextBox::MultilineValidation::document_too_large,
        "queryable document refusal");
    require(encoding_status == TextBox::MultilineValidation::invalid_utf8,
        "queryable invalid UTF8 refusal");
    const std::string before(f.editor.text());
    const TextSelection selected = f.editor.selection();
    bool rejected{};
    try {
        f.editor.set_text(oversized_line);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected && f.editor.text() == before && f.editor.selection() == selected, "set_text refusal atomicity");
    const bool oversized_edit = f.editor.replace_selection(oversized_line);
    require(!oversized_edit && f.editor.text() == before &&
        f.editor.selection() == selected, "edit refusal atomicity");
    const bool semantic_edit = f.editor.on_semantic_action(SemanticAction::set_value, oversized_line);
    require(!semantic_edit &&
        f.editor.selection() == selected, "semantic refusal preserves selection");
    const bool undone = f.editor.undo();
    require(undone && f.editor.text() == "firond", "refused edit leaves undo unchanged");
}

void dpi_transition_remeasures_same_provider() {
    Fixture f{};
    f.editor.set_word_wrap(true);
    f.editor.set_text("abcdefghijklmnopqrst");
    f.caret(0);
    f.paint();
    const std::size_t visual_count_6 = f.editor.visual_line_count();
    require(visual_count_6 == 2, "initial DPI wrap geometry");
    const std::size_t calls = f.metrics.calls;
    const FontSpec font = f.editor.font();
    const Rect bounds = f.editor.arranged_bounds();

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
    require(f.editor.font() == font && f.editor.arranged_bounds() == bounds &&
        f.window.text_metrics_provider() == &f.metrics, "DPI regression keeps other cache keys unchanged");
    const std::size_t visual_count_7 = f.editor.visual_line_count();
    require(visual_count_7 == 4, "DPI transition updates wrapping");
    require(f.painter.texts.size() >= 2 && f.painter.texts[0].origin.y == 27.0 &&
        f.painter.texts[1].origin.y == 57.0, "DPI transition updates ascent and row height");
    const PointerEvent press{.action = PointerAction::down, .button = PointerButton::primary, .position = {46.0, 39.0}};
    f.pointer(press);
    require(f.caret() == 7, "DPI hit-test uses updated row height and advances");
    const PointerEvent release{.action = PointerAction::up, .button = PointerButton::primary, .position = {46.0, 39.0}};
    f.pointer(release);
    f.key(PhysicalKey::end, Modifier::control);
    f.paint();
    require(f.editor.scroll_offset().y == 60.0, "DPI caret reveal uses updated document height");
    const std::size_t warm_calls = f.metrics.calls;
    f.paint();
    require(f.metrics.calls == warm_calls, "unchanged DPI repaint reuses refreshed metrics");

    f.metrics.advance = 10.0;
    f.metrics.line_height = 20.0;
    f.metrics.ascent = 15.0;
    f.window.set_scale(1.0);
    f.paint();
    const std::size_t visual_count_8 = f.editor.visual_line_count();
    require(visual_count_8 == 2 && f.editor.scroll_offset().y == 0.0,
        "return DPI transition remeasures and clamps viewport");
}

void clear_history_establishes_save_boundary() {
    Fixture f{};
    f.editor.set_text("alpha\r\nbeta\r\ngamma\r\ndelta\r\nepsilon");
    const bool first_edit = f.editor.replace_selection("1");
    require(first_edit, "first save-boundary edit");
    const bool second_edit = f.editor.replace_selection("2");
    require(second_edit, "second save-boundary edit");
    const bool initial_undo = f.editor.undo();
    require(initial_undo && f.editor.can_undo() && f.editor.can_redo(),
        "save-boundary fixture has both undo and redo history");
    f.editor.select(Utf8Offset(2), Utf8Offset(f.editor.text().size()));
    f.editor.set_read_only(true);
    f.paint();
    const std::string text(f.editor.text());
    const TextSelection selection = f.editor.selection();
    const Point viewport = f.editor.scroll_offset();
    const Dirty dirty = f.editor.dirty();
    const std::size_t metric_calls = f.metrics.calls;
    std::size_t text_events{};
    std::size_t selection_events{};
    const SubscriptionToken text_subscription = f.editor.text_changed().subscribe(
        test_support::IncrementCounter<std::size_t, const std::string&>(text_events));
    const SubscriptionToken selection_subscription = f.editor.selection_changed().subscribe(
        test_support::IncrementCounter<std::size_t, const TextSelection&>(selection_events));

    f.editor.clear_undo_history();
    f.editor.clear_undo_history();
    require(!f.editor.can_undo() && !f.editor.can_redo(), "clear history removes both directions and is idempotent");
    require(f.editor.text() == text && f.editor.selection() == selection &&
        f.editor.scroll_offset() == viewport && f.editor.dirty() == dirty &&
        f.metrics.calls == metric_calls && text_events == 0 && selection_events == 0,
        "clearing history preserves document, selection, viewport and notifications");
    f.editor.set_read_only(false);
    const bool old_undo = f.editor.undo();
    const bool old_redo = f.editor.redo();
    require(!old_undo && !old_redo, "old states cannot cross save boundary");
    const bool replacement = f.editor.replace_selection("replacement");
    require(replacement, "post-save replacement");
    const bool new_undo = f.editor.undo();
    require(new_undo &&
        f.editor.text() == text && f.editor.selection() == selection && !f.editor.can_undo(),
        "next edit can undo exactly to successful-save boundary");
    const bool new_redo = f.editor.redo();
    require(new_redo && f.editor.text() != text, "redo works after new post-save edit");

    const std::shared_ptr<TextBox> single = make_control<TextBox>(StableId("single.history"), "saved");
    const bool single_edit = (*single).replace_selection(" edit");
    require(single_edit, "single-line edit");
    (*single).clear_undo_history();
    const bool single_undo = (*single).undo();
    require((*single).text() == "saved edit" && !single_undo, "single-line history boundary preserves value");
}

void bounded_workload() {
    Fixture f{};
    std::string many{};
    many.reserve(140000U);
    for (std::size_t i = 0; i < 10000; ++i) many += "line fixture\r\n";
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    f.editor.set_text(many);
    const std::size_t visual_count_9 = f.editor.visual_line_count();
    require(visual_count_9 == 10001, "10k logical lines");
    f.paint();
    const std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> duration = finish - start;
    const double elapsed = duration.count();
    require(f.painter.texts.size() <= 4, "paint only visible lines");
    const std::size_t count = f.metrics.calls;
    f.paint();
    require(f.metrics.calls == count, "large document warm repaint performs no measurements");
    std::cout << "10k lines: " << elapsed << " ms; cold metric calls=" << count << "; warm=0\n";
    const std::string maximum_line(4096U, 'x');
    f.editor.set_text(maximum_line);
    start = std::chrono::steady_clock::now();
    const std::size_t visual_count_10 = f.editor.visual_line_count();
    require(visual_count_10 == 1, "maximum admitted line");
    const std::chrono::steady_clock::time_point line_finish = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> line_duration = line_finish - start;
    const double line_elapsed = line_duration.count();
    std::cout << "4096-byte line cold layout: " << line_elapsed << " ms\n";
}

class CaretPainter final : public Painter {
public:
    void save() override {}
    void restore() override {}
    void translate(Point) override {}
    void clip_rect(Rect) override {}
    void fill_rect(Rect, Color) override {}
    void stroke_rect(Rect, Color, double) override {}
    void draw_line(const Point first, const Point last, Color, double) override {
        ++lines;
        start = first;
        end = last;
    }
    void draw_text_utf8(Point, std::string_view, FontSpec, Color) override {}
    void draw_image(ImageId, Rect, double) override {}
    std::size_t lines{0};
    Point start{};
    Point end{};
};

void paint_retained(Fixture& fixture, CaretPainter& painter) {
    fixture.window.perform_layout();
    const DamageRegion damage = fixture.window.take_damage();
    const std::optional<PaintReceipt> receipt = fixture.window.paint(painter, damage.bounds());
    require(receipt.has_value(), "retained caret fixture must paint");
}

Rect blink_damage(Fixture& fixture) {
    const std::optional<FrameTime> deadline = fixture.window.next_wake();
    require(deadline.has_value(), "focused caret must retain a deadline");
    const FramePollResult poll = fixture.window.poll_frame_schedule(*deadline);
    require(poll.deadlines_fired == 1U && poll.coalesced_requests == 1U,
            "actual caret callback must supply damage before scheduler fallback");
    const DamageRegion damage = fixture.window.take_damage();
    const Rect bounds = damage.bounds();
    return bounds;
}

void localized_caret_blinks() {
    const std::array<bool, 2> modes{false, true};
    for (const bool multiline : modes) {
        Fixture fixture{};
        fixture.editor.set_multiline(multiline);
        CaretPainter painter{};
        paint_retained(fixture, painter);
        require(painter.lines == 1U, "empty editor initially draws its visible caret");
        const Point old_start = painter.start;
        const Point old_end = painter.end;
        fixture.window.reset_activity_metrics();
        const Rect hidden_damage = blink_damage(fixture);
        require(hidden_damage.width > 0.0 && hidden_damage.width <= 3.0 &&
                    hidden_damage.height <= 63.0,
                "empty caret damage must remain a narrow padded strip");
        require(hidden_damage.x <= old_start.x &&
                    hidden_damage.x + hidden_damage.width > old_start.x &&
                    hidden_damage.y <= old_start.y &&
                    hidden_damage.y + hidden_damage.height >= old_end.y,
                "erase damage must contain the previously recorded caret endpoints");
        painter.lines = 0U;
        const std::optional<PaintReceipt> hidden = fixture.window.paint(painter, hidden_damage);
        require(hidden.has_value() && painter.lines == 0U,
                "hidden blink must remove the recorded caret line");
        const Rect visible_damage = blink_damage(fixture);
        require(visible_damage == hidden_damage,
                "show and erase must damage the same retained caret pixels");
        const std::optional<PaintReceipt> visible = fixture.window.paint(painter, visible_damage);
        require(visible.has_value() && painter.lines == 1U,
                "visible blink must restore the recorded caret line");
        const MetricsSnapshot metrics = fixture.window.metrics_snapshot();
        require(metrics.display_chunks_rebuilt == 2U && metrics.measure_passes == 0U &&
                    metrics.arrange_passes == 0U && metrics.painted_damage_area < 400.0,
                "blink keeps chunk rebuilding but bounds raster damage without layout");
        fixture.window.set_occluded(true, FrameClock::now());
        require(!fixture.window.next_wake().has_value(), "occluded caret remains suspended");
    }

    Fixture fixture{};
    fixture.editor.set_text("alpha\nbeta\ngamma");
    fixture.caret(7U);
    CaretPainter painter{};
    paint_retained(fixture, painter);
    const Rect damage = blink_damage(fixture);
    require(damage.width <= 3.0 && damage.height <= 23.0 && damage.y > 4.0,
            "multiline caret damage follows the selected row");

    const std::array<double, 3> scales{0.5, 1.5, 2.0};
    for (const double scale : scales) {
        Fixture scaled{};
        scaled.editor.set_text("abc");
        scaled.caret(1U);
        scaled.window.set_scale(scale);
        CaretPainter scaled_painter{};
        paint_retained(scaled, scaled_painter);
        const Rect scaled_damage = blink_damage(scaled);
        const double expected_width = 1.0 + 2.0 / scale;
        require(std::abs(scaled_damage.width - expected_width) < 1.0e-9,
                "unclipped caret damage includes one device pixel beyond each stroke edge");
    }
}

void require_full_blink_fallback(Fixture& fixture, const char* message) {
    // Drain the prior mutation's region while leaving its dirty state pending.
    // The blink itself must not trust previously recorded geometry.
    const DamageRegion pending = fixture.window.take_damage();
    static_cast<void>(pending);
    const Rect damage = blink_damage(fixture);
    require(damage.width >= fixture.editor.client_rectangle().width, message);
}

void caret_geometry_fallbacks() {
    Metrics replacement_metrics{};
    Fixture fixture{};
    CaretPainter painter{};
    require_full_blink_fallback(fixture, "unpainted caret requires full fallback");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    fixture.editor.set_text("alpha\nbeta\ngamma\ndelta\nepsilon");
    require_full_blink_fallback(fixture, "text replacement invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    fixture.editor.arrange({0.0, 0.0, 90.0, 60.0});
    require_full_blink_fallback(fixture, "resize invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    const PointerEvent wheel{.action = PointerAction::wheel,
                             .position = {20.0, 20.0}, .wheel_delta = {0.0, 1.0}};
    fixture.pointer(wheel);
    require_full_blink_fallback(fixture, "scroll invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    fixture.window.set_scale(1.5);
    require_full_blink_fallback(fixture, "device scale invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    FontSpec font = fixture.editor.font();
    font.size += 2.0;
    fixture.editor.set_font(font);
    require_full_blink_fallback(fixture, "font invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    fixture.window.set_text_metrics_provider(&replacement_metrics);
    require_full_blink_fallback(fixture, "metrics provider invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    fixture.caret(0U);
    require_full_blink_fallback(fixture, "selection invalidates caret geometry");
    fixture.editor.invalidate(Dirty::paint);
    paint_retained(fixture, painter);
    const bool blurred = fixture.window.request_focus({});
    require(blurred, "blur must succeed");
    require(!fixture.window.next_wake().has_value(), "blur revokes caret scheduling");
    const bool focused = fixture.window.request_focus(fixture.field);
    require(focused, "refocus must succeed");
    require_full_blink_fallback(fixture, "refocus cannot reuse previous caret geometry");
}
}

int main() {
    try {
        exact_editing();
        navigation();
        wrapping_metrics_and_hit_testing();
        tab_wrap_and_reused_rows();
        clipboard_and_limits();
        dpi_transition_remeasures_same_provider();
        clear_history_establishes_save_boundary();
        localized_caret_blinks();
        caret_geometry_fallbacks();
        bounded_workload();
        std::cout << "gui_forms_multiline_text_box_tests: passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
