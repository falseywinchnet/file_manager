#include "gui_forms/controls/panel/text_box/text_box.hpp"

#include "../input_control_utilities.hpp"
#include "gui_forms/detail/algorithm/binary_search.hpp"
#include "gui_forms/detail/bound_member_function.hpp"
#include "gui_forms/detail/property_binding_adapters.hpp"
#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
using namespace input_control_detail;

TextBox::MultilineValidation TextBox::validate_multiline_text(std::string_view text) {
    if (text.size() > maximum_multiline_bytes) return MultilineValidation::document_too_large;
    if (!validate_utf8(text).valid()) return MultilineValidation::invalid_utf8;
    const TextStore candidate(text);
    for (std::size_t i = 0; i < candidate.line_count(); ++i) {
        const auto range = candidate.line_content_range(LineIndex(i));
        if (range.end.value() - range.start.value() > maximum_multiline_line_bytes) return MultilineValidation::line_too_long;
    }
    return MultilineValidation::valid;
}

TextBox::TextBox(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), store_(text) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
    set_cursor(CursorKind::text);
    selection_ = {store_.utf8_size(), store_.utf8_size()};
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Editable UTF-8 text content.", BindingValue{std::string{}},
         invalidation::text_content},
        detail::BoundMemberFunction<BindingValue (TextBox::*)() const>(
            *this, &TextBox::text_property_value),
        detail::ConvertedPropertySetter<TextBox, std::string>(
            *this, &TextBox::set_text, BindingValueKind::text,
            "TextBox.Text binding requires text"),
        detail::EventChangeConnector<const std::string&>(text_changed_), {}, {}});
}

BindingValue TextBox::text_property_value() const {
    return BindingValue{std::string(store_.utf8())};
}

void TextBox::set_text(std::string text) {
    require_mutable();
    if (multiline_ && validate_multiline_text(text) != MultilineValidation::valid) {
        throw std::invalid_argument("Multiline TextBox requires valid UTF-8, at most 1 MiB total and 4096 bytes per line");
    }
    if (text == store_.utf8()) {
        return;
    }
    store_.set_text(text);
    selection_ = {store_.utf8_size(), store_.utf8_size()};
    layout_positions_.clear();
    layout_offsets_.clear();
    undo_.clear();
    redo_.clear();
    history_bytes_ = 0U;
    horizontal_offset_ = 0.0;
    vertical_offset_ = 0.0;
    reset_caret_blink();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    const std::string value(store_.utf8());
    publish_change(text_changed_, value);
    publish_change(selection_changed_, selection_);
}

void TextBox::set_placeholder_text(std::string text) {
    require_mutable();
    if (!validate_utf8(text).valid()) {
        throw std::invalid_argument("TextBox placeholder must be valid UTF-8");
    }
    if (placeholder_ == text) {
        return;
    }
    placeholder_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics);
}

void TextBox::set_read_only(bool read_only) {
    require_mutable();
    if (read_only_ == read_only) {
        return;
    }
    read_only_ = read_only;
    invalidate(Dirty::paint | Dirty::semantics);
}

void TextBox::set_multiline(bool enabled_value) {
    require_mutable();
    if (multiline_ == enabled_value) return;
    if (enabled_value && (password_protected() || validate_multiline_text(text()) != MultilineValidation::valid)) {
        throw std::invalid_argument("Multiline TextBox requires unmasked valid UTF-8, at most 1 MiB total and 4096 bytes per line");
    }
    multiline_ = enabled_value;
    visual_lines_.clear();
    horizontal_offset_ = vertical_offset_ = 0.0;
    reset_caret_blink();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TextBox::set_word_wrap(bool enabled_value) {
    require_mutable();
    if (word_wrap_ == enabled_value) return;
    word_wrap_ = enabled_value;
    visual_lines_.clear();
    horizontal_offset_ = 0.0;
    reset_caret_blink();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TextBox::set_newline_sequence(std::string sequence) {
    require_mutable();
    if (sequence != "\n" && sequence != "\r" && sequence != "\r\n") {
        throw std::invalid_argument("TextBox newline must be LF, CR, or CRLF");
    }
    newline_ = std::move(sequence);
}

void TextBox::set_accepts_tab(bool enabled_value) {
    require_mutable();
    accepts_tab_ = enabled_value;
}

void TextBox::set_maximum_length(std::size_t length) {
    require_mutable();
    if (length > maximum_configured_length_) {
        throw std::out_of_range(
            "TextBox maximum length must be zero or no greater than 16 Mi scalars");
    }
    if (maximum_length_ == length) return;
    maximum_length_ = length;
    invalidate(Dirty::semantics);
}

void TextBox::set_password_character(char32_t character) {
    require_mutable();
    if (multiline_ && character != U'\0') {
        throw std::invalid_argument("Multiline TextBox does not support password masking");
    }
    if (!valid_password_character(character)) {
        throw std::invalid_argument(
            "TextBox password character must be one printable Unicode scalar");
    }
    if (password_character_ == character) return;
    password_character_ = character;
    layout_positions_.clear();
    layout_offsets_.clear();
    horizontal_offset_ = 0.0;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TextBox::set_use_system_password_character(bool enabled_value) {
    require_mutable();
    if (multiline_ && enabled_value) {
        throw std::invalid_argument("Multiline TextBox does not support password masking");
    }
    if (use_system_password_character_ == enabled_value) return;
    use_system_password_character_ = enabled_value;
    layout_positions_.clear();
    layout_offsets_.clear();
    horizontal_offset_ = 0.0;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TextBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("TextBox font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    layout_positions_.clear();
    layout_offsets_.clear();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TextBox::select(Utf8Offset anchor, Utf8Offset caret) {
    require_mutable();
    if (!store_.is_grapheme_boundary(anchor) ||
        !store_.is_grapheme_boundary(caret)) {
        throw std::out_of_range("TextBox selection must use grapheme boundaries");
    }
    set_selection({anchor, caret});
}

void TextBox::select_all() {
    set_selection({Utf8Offset(0U), store_.utf8_size()});
}

std::string TextBox::selected_text() const {
    return std::string(store_.utf8().substr(selection_.start().value(),
                                            selection_.length()));
}

TextBox::Snapshot TextBox::snapshot() const {
    return {std::string(store_.utf8()), selection_};
}

void TextBox::apply_snapshot(Snapshot snapshot) {
    store_.set_text(snapshot.text);
    selection_ = snapshot.selection;
    layout_positions_.clear();
    layout_offsets_.clear();
    reset_caret_blink();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    const std::string value(store_.utf8());
    publish_change(text_changed_, value);
    publish_change(selection_changed_, selection_);
}

void TextBox::push_history(std::deque<Snapshot>& history, Snapshot snapshot) {
    history_bytes_ += snapshot.text.size();
    history.push_back(std::move(snapshot));
    while (undo_.size() + redo_.size() > maximum_history_entries_ ||
           history_bytes_ > maximum_history_bytes_) {
        std::deque<Snapshot>* source = !undo_.empty() ? &undo_ : &redo_;
        history_bytes_ -= (*source).front().text.size();
        (*source).pop_front();
    }
}

void TextBox::clear_redo() noexcept {
    for (const Snapshot& snapshot : redo_) {
        history_bytes_ -= snapshot.text.size();
    }
    redo_.clear();
}

bool TextBox::undo() {
    require_mutable();
    if (read_only_ || undo_.empty()) {
        return false;
    }
    if (multiline_ && validate_multiline_text(undo_.back().text) != MultilineValidation::valid) return false;
    Snapshot target = std::move(undo_.back());
    history_bytes_ -= target.text.size();
    undo_.pop_back();
    push_history(redo_, snapshot());
    apply_snapshot(std::move(target));
    return true;
}

bool TextBox::redo() {
    require_mutable();
    if (read_only_ || redo_.empty()) {
        return false;
    }
    if (multiline_ && validate_multiline_text(redo_.back().text) != MultilineValidation::valid) return false;
    Snapshot target = std::move(redo_.back());
    history_bytes_ -= target.text.size();
    redo_.pop_back();
    push_history(undo_, snapshot());
    apply_snapshot(std::move(target));
    return true;
}

bool TextBox::replace(Utf8Offset start, Utf8Offset end,
                      std::string_view replacement, bool record_history) {
    require_mutable();
    const Utf8ValidationResult replacement_validation =
        validate_utf8(replacement);
    if (read_only_ || start.value() > end.value() ||
        !store_.is_grapheme_boundary(start) ||
        !store_.is_grapheme_boundary(end) ||
        !replacement_validation.valid()) {
        return false;
    }
    if (multiline_ && replacement.size() > maximum_multiline_bytes -
            (text().size() - (end.value() - start.value()))) return false;
    if (multiline_) {
        std::string candidate(text());
        candidate.replace(start.value(), end.value() - start.value(), replacement);
        if (validate_multiline_text(candidate) != MultilineValidation::valid) return false;
    }
    const std::size_t removed_scalars =
        store_.scalar_index(end).value() - store_.scalar_index(start).value();
    if (maximum_length_ != 0U &&
        store_.scalar_count().value() - removed_scalars +
                replacement_validation.scalar_count >
            maximum_length_) {
        return false;
    }
    const std::string_view existing = store_.utf8().substr(
        start.value(), end.value() - start.value());
    if (existing == replacement) {
        set_selection({Utf8Offset(start.value() + replacement.size()),
                       Utf8Offset(start.value() + replacement.size())});
        return true;
    }
    Snapshot previous = snapshot();
    static_cast<void>(store_.replace({start, end}, replacement));
    if (record_history) {
        clear_redo();
        push_history(undo_, std::move(previous));
    }
    Utf8Offset next(start.value() + replacement.size());
    // An insertion may join a following combining sequence or CRLF. Keep the
    // resulting caret on a boundary of the new text, not merely the old text.
    while (!store_.is_grapheme_boundary(next)) next = store_.next_scalar_boundary(next);
    selection_ = {next, next};
    layout_positions_.clear();
    layout_offsets_.clear();
    reset_caret_blink();
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    const std::string value(store_.utf8());
    publish_change(text_changed_, value);
    publish_change(selection_changed_, selection_);
    return true;
}

bool TextBox::replace_selection(std::string_view replacement) {
    return replace(selection_.start(), selection_.end(), replacement);
}

bool TextBox::delete_selection() {
    return !selection_.empty() && replace_selection({});
}

bool TextBox::copy() {
    require_mutable();
    if (selection_.empty() || password_protected() || window() == nullptr ||
        (*window()).host_services() == nullptr) {
        return false;
    }
    return (*(*window()).host_services()).write_clipboard_text(selected_text()).accepted();
}

bool TextBox::cut() {
    require_mutable();
    if (read_only_ || password_protected() || selection_.empty() || !copy()) {
        return false;
    }
    return delete_selection();
}

bool TextBox::paste() {
    require_mutable();
    if (read_only_ || window() == nullptr || (*window()).host_services() == nullptr) {
        return false;
    }
    const HostClipboardTextResult result =
        (*(*window()).host_services()).read_clipboard_text();
    return result.status.accepted() && result.has_text &&
        replace_selection(result.text_utf8);
}

void TextBox::set_selection(TextSelection selection, bool reveal_caret) {
    const bool upstream = std::exchange(next_upstream_, false);
    if (selection_ == selection) {
        if (reveal_caret) {
            reset_caret_blink();
        }
        caret_upstream_ = upstream;
        invalidate(Dirty::paint);
        return;
    }
    selection_ = selection;
    if (reveal_caret) {
        reset_caret_blink();
    }
    caret_upstream_ = upstream;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selection_changed_, selection_);
}

double TextBox::boundary_x(Utf8Offset offset) const noexcept {
    if (layout_positions_.empty() ||
        layout_positions_.size() != layout_offsets_.size() ||
        layout_text_scale_ != effective_text_scale()) {
        return static_cast<double>(store_.grapheme_index(offset).value()) *
            effective_font(font_).size * 0.55;
    }
    const std::size_t found = detail::lower_bound_index(
        std::span<const std::uint64_t>(layout_offsets_), offset.value());
    const std::size_t index = found == layout_offsets_.size()
        ? layout_offsets_.size() - 1U
        : found;
    return layout_positions_[index];
}

Utf8Offset TextBox::previous_word_boundary(Utf8Offset offset) const {
    if (offset.value() == 0U) return offset;
    Utf8Offset cursor = store_.previous_grapheme_boundary(offset);
    WordClass category = word_class(store_.scalar_at(cursor));
    while (cursor.value() > 0U) {
        const Utf8Offset previous = store_.previous_grapheme_boundary(cursor);
        if (word_class(store_.scalar_at(previous)) != category) break;
        cursor = previous;
    }
    if (category == WordClass::spacing && cursor.value() > 0U) {
        cursor = store_.previous_grapheme_boundary(cursor);
        category = word_class(store_.scalar_at(cursor));
        while (cursor.value() > 0U) {
            const Utf8Offset previous = store_.previous_grapheme_boundary(cursor);
            if (word_class(store_.scalar_at(previous)) != category) break;
            cursor = previous;
        }
    }
    return cursor;
}

Utf8Offset TextBox::next_word_boundary(Utf8Offset offset) const {
    const Utf8Offset end = store_.utf8_size();
    if (offset == end) return end;
    WordClass category = word_class(store_.scalar_at(offset));
    Utf8Offset cursor = offset;
    while (cursor != end && word_class(store_.scalar_at(cursor)) == category) {
        cursor = store_.next_grapheme_boundary(cursor);
    }
    if (category != WordClass::spacing) {
        while (cursor != end &&
               word_class(store_.scalar_at(cursor)) == WordClass::spacing) {
            cursor = store_.next_grapheme_boundary(cursor);
        }
    }
    return cursor;
}

std::string TextBox::display_text() const {
    if (!password_protected()) return std::string(store_.utf8());
    const char32_t mask = use_system_password_character_
        ? U'\u2022' : password_character_;
    const std::string encoded = utf8_scalar(mask);
    std::string result;
    result.reserve(encoded.size() * store_.grapheme_count().value());
    for (std::size_t index = 0U; index < store_.grapheme_count().value(); ++index) {
        result += encoded;
    }
    return result;
}

void TextBox::ensure_multiline_layout() {
    const FontSpec font = effective_font(font_);
    const double width = std::max(1.0, local_bounds().width - 10.0);
    // Device-pixel rounding can change logical advances/ascent while the
    // provider address, authored font and logical viewport remain unchanged.
    const double device_scale = window() ? window()->scale() : 1.0;
    const TextMetricsProvider* provider = window() ? window()->text_metrics_provider() : nullptr;
    if (!visual_lines_.empty() && multiline_revision_ == store_.revision() &&
        multiline_font_ == font && multiline_width_ == width &&
        multiline_provider_ == provider && multiline_device_scale_ == device_scale) return;
    visual_lines_.clear();
    document_width_ = 0.0;
    const ResolvedTextLayout metrics = resolve_text_layout_utf8("Mg", font);
    line_height_ = std::max(1.0, std::max(metrics.logical_size.height,
        metrics.ascent + metrics.descent + metrics.line_gap));
    line_ascent_ = metrics.ascent > 0.0 ? metrics.ascent : font.size;
    const double tab_width = std::max(1.0,
        resolve_text_layout_utf8("    ", font).logical_size.width);
    // Keep complete visual-row shaping runs. Prefix measurement is quadratic
    // within the admitted 4096-byte logical-line bound, cached across paints.
    // Tabs are explicit shaping boundaries; arbitrary chunk cuts are forbidden.
    for (std::size_t logical = 0; logical < store_.line_count(); ++logical) {
        const Utf8Range range = store_.line_content_range(LineIndex(logical));
        std::size_t cursor = range.start.value();
        do {
            VisualLine line;
            line.offsets.push_back(cursor);
            line.positions.push_back(0.0);
            std::size_t last_space{};
            bool wrapped{};
            while (cursor < range.end.value() && !wrapped) {
                const std::size_t run_start = cursor;
                const double run_x = line.positions.back();
                const bool tab = text()[cursor] == '\t';
                std::size_t run_end = cursor;
                for (std::size_t count = 0; cursor < range.end.value(); ++count) {
                    if (count != 0 && text()[cursor] == '\t') break;
                    const std::size_t next = store_.next_grapheme_boundary(Utf8Offset(cursor)).value();
                    const double x = tab
                        ? (std::floor(run_x / tab_width) + 1.0) * tab_width
                        : run_x + resolve_text_layout_utf8(
                            text().substr(run_start, next - run_start), font).logical_size.width;
                    if (word_wrap_ && x > width && line.offsets.size() > 1) {
                        wrapped = true;
                        break;
                    }
                    cursor = next;
                    run_end = next;
                    line.offsets.push_back(next);
                    line.positions.push_back(x);
                    const char32_t scalar = store_.scalar_at(Utf8Offset(line.offsets[line.offsets.size() - 2]));
                    if (scalar == U' ' || scalar == U'\t') last_space = line.offsets.size() - 1;
                    if (tab) break;
                }
                if (!tab && run_end != run_start) line.runs.push_back({run_start, run_end, run_x});
            }
            if (wrapped && last_space > 0 && last_space + 1 < line.offsets.size()) {
                cursor = line.offsets[last_space];
                line.offsets.resize(last_space + 1);
                line.positions.resize(last_space + 1);
                while (!line.runs.empty() && line.runs.back().start >= cursor) line.runs.pop_back();
                if (!line.runs.empty()) line.runs.back().end = std::min(line.runs.back().end, cursor);
            }
            document_width_ = std::max(document_width_, line.positions.back());
            visual_lines_.push_back(std::move(line));
        } while (cursor < range.end.value());
    }
    multiline_revision_ = store_.revision();
    multiline_font_ = font;
    multiline_width_ = width;
    multiline_provider_ = provider;
    multiline_device_scale_ = device_scale;
    reveal_pending_ = true;
}

std::size_t TextBox::visual_line_count() {
    require_mutable();
    if (!multiline_) return 1;
    ensure_multiline_layout();
    return visual_lines_.size();
}

std::size_t TextBox::caret_line() const {
    const auto found = std::upper_bound(visual_lines_.begin(), visual_lines_.end(),
        selection_.caret.value(), [](std::size_t value, const VisualLine& row) {
            return value < row.offsets.front();
        });
    std::size_t row = found == visual_lines_.begin() ? 0U
        : static_cast<std::size_t>(found - visual_lines_.begin() - 1);
    if (caret_upstream_ && row > 0 && visual_lines_[row - 1].offsets.back() == selection_.caret.value()) --row;
    return row;
}

double TextBox::multiline_boundary_x(std::size_t line, Utf8Offset offset) const {
    const VisualLine& row = visual_lines_[line];
    const auto found = std::lower_bound(row.offsets.begin(), row.offsets.end(), offset.value());
    return row.positions[std::min(static_cast<std::size_t>(found - row.offsets.begin()), row.positions.size() - 1)];
}

Utf8Offset TextBox::position_in_line(std::size_t line, double x) const {
    const VisualLine& row = visual_lines_[line];
    // Prefix advances need not be monotonic for every script. Nearest logical
    // boundary is deterministic without assuming sorted visual positions.
    std::size_t best{};
    double distance = std::abs(x - row.positions.front());
    for (std::size_t i = 1; i < row.positions.size(); ++i) {
        const double candidate = std::abs(x - row.positions[i]);
        if (candidate <= distance) { best = i; distance = candidate; }
    }
    return Utf8Offset(row.offsets[best]);
}

Utf8Offset TextBox::multiline_position_at(double x, double y) {
    ensure_multiline_layout();
    const double row = std::max(0.0, std::floor((y - 4.0 + vertical_offset_) / line_height_));
    const std::size_t index = std::min(static_cast<std::size_t>(row), visual_lines_.size() - 1);
    const Utf8Offset position = position_in_line(index, x - text_left_ + horizontal_offset_);
    next_upstream_ = position.value() == visual_lines_[index].offsets.back();
    return position;
}

void TextBox::reveal_multiline_caret() {
    const double width = std::max(1.0, local_bounds().width - 10.0);
    const double height = std::max(1.0, local_bounds().height - 8.0);
    if (reveal_pending_) {
        const std::size_t row = caret_line();
        const double x = multiline_boundary_x(row, selection_.caret);
        const double y = static_cast<double>(row) * line_height_;
        if (y < vertical_offset_) vertical_offset_ = y;
        if (y + line_height_ > vertical_offset_ + height) vertical_offset_ = y + line_height_ - height;
        if (x < horizontal_offset_) horizontal_offset_ = x;
        if (x > horizontal_offset_ + width - 1.0) horizontal_offset_ = x - width + 1.0;
        reveal_pending_ = false;
    }
    vertical_offset_ = std::clamp(vertical_offset_, 0.0,
        std::max(0.0, visual_lines_.size() * line_height_ - height));
    horizontal_offset_ = word_wrap_ ? 0.0 : std::clamp(horizontal_offset_, 0.0,
        std::max(0.0, document_width_ - width + 1.0));
}

void TextBox::paint_multiline(Painter& painter) {
    ensure_multiline_layout();
    reveal_multiline_caret();
    const Rect bounds = local_bounds();
    const double width = std::max(0.0, bounds.width - 10.0);
    const double height = std::max(0.0, bounds.height - 8.0);
    const FontSpec font = effective_font(font_);
    const bool themed = !has_background_override() && !has_style_override();
    const auto& editor = effective_theme().resolve(ControlVisualRole::editor,
        visual_context(false, false, false, focused_));
    const auto& highlight = effective_theme().resolve(ControlVisualRole::selection,
        visual_context(false, false, true, true));
    const Color foreground = themed ? editor.text : enabled() ? style().text : style().disabled_text;
    painter.save();
    painter.clip_rect({text_left_, 4.0, width, height});
    const std::size_t first = static_cast<std::size_t>(vertical_offset_ / line_height_);
    const std::size_t last = std::min(visual_lines_.size(), first +
        static_cast<std::size_t>(std::ceil(height / line_height_)) + 1);
    for (std::size_t i = first; i < last; ++i) {
        const VisualLine& row = visual_lines_[i];
        const double y = 4.0 + i * line_height_ - vertical_offset_;
        const double origin = text_left_ - horizontal_offset_;
        const bool selected = focused_ && !selection_.empty() &&
            selection_.start().value() <= row.offsets.back() &&
            selection_.end().value() > row.offsets.front();
        Rect selection_bounds{};
        if (selected) {
            const double left = multiline_boundary_x(i, selection_.start());
            double right = multiline_boundary_x(i, selection_.end());
            if (selection_.end().value() > row.offsets.back()) right += std::max(3.0, font.size * 0.35);
            selection_bounds = {origin + left, y, std::max(0.0, right - left), line_height_};
            if (themed) paint_surface_material(painter, selection_bounds, highlight.material);
            else painter.fill_rect(selection_bounds, style().accent);
        }
        for (const VisualRun& run : row.runs) {
            const Point baseline{origin + run.x, y + line_ascent_};
            const auto value = text().substr(run.start, run.end - run.start);
            painter.draw_text_utf8(baseline, value, font, foreground);
            if (selected) {
                painter.save();
                painter.clip_rect(selection_bounds);
                painter.draw_text_utf8(baseline, value, font, themed ? highlight.text : style().highlight);
                painter.restore();
            }
        }
    }
    if (text().empty() && !placeholder_.empty()) painter.draw_text_utf8(
        {text_left_, 4.0 + line_ascent_}, placeholder_, font, themed ? editor.muted_text : style().disabled_text);
    if (focused_ && caret_visible_) {
        const std::size_t row = caret_line();
        const double x = text_left_ - horizontal_offset_ + multiline_boundary_x(row, selection_.caret);
        const double y = 4.0 + row * line_height_ - vertical_offset_;
        painter.draw_line({x, y}, {x, y + line_height_}, foreground, 1.0);
    }
    painter.restore();
}

Utf8Offset TextBox::position_at(double local_x) const noexcept {
    const double content_x = std::max(0.0,
        local_x - text_left_ + horizontal_offset_);
    if (layout_positions_.empty() ||
        layout_positions_.size() != layout_offsets_.size() ||
        layout_text_scale_ != effective_text_scale()) {
        const FontSpec font = effective_font(font_);
        const std::size_t grapheme = std::min(
            static_cast<std::size_t>(std::lround(content_x /
                                                  (font.size * 0.55))),
            store_.grapheme_count().value());
        return store_.utf8_offset(GraphemeIndex(grapheme));
    }
    const std::size_t index = detail::lower_bound_index(
        std::span<const double>(layout_positions_), content_x);
    if (index == 0U) {
        return Utf8Offset(layout_offsets_.front());
    }
    if (index == layout_positions_.size()) {
        return Utf8Offset(layout_offsets_.back());
    }
    const double left_distance = content_x - layout_positions_[index - 1U];
    const double right_distance = layout_positions_[index] - content_x;
    return Utf8Offset(layout_offsets_[left_distance < right_distance
        ? index - 1U : index]);
}

void TextBox::on_paint(Painter& painter, Rect damage) {
    const Rect bounds = local_bounds();
    const bool themed = !has_background_override() && !has_style_override();
    const ControlVisualRecipe& editor_recipe = effective_theme().resolve(
        ControlVisualRole::editor,
        visual_context(false, false, false, focused_));
    if (themed) {
        paint_surface_material(painter, bounds, editor_recipe.material);
    } else {
        Panel::on_paint(painter, damage);
    }
    if (multiline_) {
        paint_multiline(painter);
        return;
    }
    const FontSpec font = effective_font(font_);
    const double right = std::max(text_left_, bounds.width - 4.0);
    const double viewport = std::max(0.0, right - text_left_);
    const std::string presented = display_text();
    const std::string mask = password_protected()
        ? utf8_scalar(use_system_password_character_ ? U'\u2022'
                                                     : password_character_)
        : std::string{};
    layout_positions_.clear();
    layout_offsets_.clear();
    layout_text_scale_ = effective_text_scale();
    const std::size_t count = store_.grapheme_count().value();
    layout_positions_.reserve(count + 1U);
    layout_offsets_.reserve(count + 1U);
    for (std::size_t index = 0; index <= count; ++index) {
        const Utf8Offset offset = store_.utf8_offset(GraphemeIndex(index));
        layout_offsets_.push_back(offset.value());
        const std::size_t presentation_offset = password_protected()
            ? index * mask.size() : offset.value();
        layout_positions_.push_back(painter.measure_text_utf8(
            std::string_view(presented).substr(0U, presentation_offset), font).width);
    }
    const double caret_content_x = boundary_x(selection_.caret);
    if (caret_content_x < horizontal_offset_) {
        horizontal_offset_ = caret_content_x;
    } else if (caret_content_x > horizontal_offset_ + viewport) {
        horizontal_offset_ = caret_content_x - viewport;
    }
    horizontal_offset_ = std::clamp(horizontal_offset_, 0.0,
        std::max(0.0, layout_positions_.back() - viewport));

    const double origin_x = text_left_ - horizontal_offset_;
    const double selection_x = origin_x + boundary_x(selection_.start());
    const double selection_end_x = origin_x + boundary_x(selection_.end());
    const double baseline = std::max(font.size,
        (bounds.height + font.size) * 0.5 - 1.0);
    painter.save();
    painter.clip_rect({text_left_, 2.0, viewport,
                       std::max(0.0, bounds.height - 4.0)});
    if (focused_ && !selection_.empty()) {
        const Rect selection_bounds{
            selection_x, 3.0,
            std::max(0.0, selection_end_x - selection_x),
            std::max(0.0, bounds.height - 6.0)};
        if (themed) {
            paint_surface_material(
                painter, selection_bounds,
                effective_theme().resolve(
                    ControlVisualRole::selection,
                    visual_context(false, false, true, true)).material);
        } else {
            painter.fill_rect(selection_bounds, style().accent);
        }
    }
    if (!store_.utf8().empty()) {
        painter.draw_text_utf8(
            {origin_x, baseline}, presented, font,
            themed ? editor_recipe.text
                   : enabled() ? style().text : style().disabled_text);
        if (focused_ && !selection_.empty()) {
            painter.save();
            painter.clip_rect({selection_x, 3.0,
                               std::max(0.0, selection_end_x - selection_x),
                               std::max(0.0, bounds.height - 6.0)});
            const Color selected_text = themed
                ? effective_theme().resolve(
                      ControlVisualRole::selection,
                      visual_context(false, false, true, true)).text
                : style().highlight;
            painter.draw_text_utf8({origin_x, baseline}, presented, font,
                                   selected_text);
            painter.restore();
        }
    } else if (!placeholder_.empty()) {
        painter.draw_text_utf8(
            {text_left_, baseline}, placeholder_, font,
            themed ? editor_recipe.muted_text : style().disabled_text);
    }
    if (focused_ && caret_visible_ && selection_.empty()) {
        const double caret_x = origin_x + caret_content_x;
        painter.draw_line({caret_x, 4.0},
                          {caret_x, std::max(4.0, bounds.height - 4.0)},
                          themed ? editor_recipe.text : style().text, 1.0);
    }
    painter.restore();
    if (focused_) {
        const Rect ring{1.5, 1.5, std::max(0.0, bounds.width - 3.0),
                        std::max(0.0, bounds.height - 3.0)};
        if (themed) {
            painter.stroke_rounded_rect(
                ring, std::max(0.0, editor_recipe.material.corner_radius - 1.0),
                editor_recipe.focus_ring, editor_recipe.focus_width);
        } else {
            painter.stroke_rect(ring, style().accent, 1.0);
        }
    }
}

void TextBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) {
        return;
    }
    const Rect absolute = absolute_bounds();
    const double local_x = event.position.x - absolute.x;
    const double local_y = event.position.y - absolute.y;
    if (multiline_ && event.action == PointerAction::wheel) {
        ensure_multiline_layout();
        vertical_offset_ = std::clamp(vertical_offset_ - event.wheel_delta.y * line_height_ * 3.0,
            0.0, std::max(0.0, visual_lines_.size() * line_height_ -
                std::max(0.0, local_bounds().height - 8.0)));
        reveal_pending_ = false;
        invalidate(Dirty::paint);
        event.handled = true;
        return;
    }
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        if (window() != nullptr) {
            static_cast<void>((*window()).request_focus(shared_from_this()));
        }
        const Utf8Offset position = multiline_ ? multiline_position_at(local_x, local_y) : position_at(local_x);
        const bool extend = includes(event.modifiers, Modifier::shift);
        set_selection(extend ? TextSelection{selection_.anchor, position}
                             : TextSelection{position, position});
        selecting_ = true;
        set_pointer_capture(true);
        event.handled = true;
    } else if (event.action == PointerAction::move && selecting_ &&
               has_pointer_capture()) {
        set_selection({selection_.anchor, multiline_ ? multiline_position_at(local_x, local_y) : position_at(local_x)});
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary && selecting_) {
        set_selection({selection_.anchor, multiline_ ? multiline_position_at(local_x, local_y) : position_at(local_x)});
        selecting_ = false;
        set_pointer_capture(false);
        event.handled = true;
    }
}

void TextBox::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down) {
        return;
    }
    const bool extend = includes(event.modifiers, Modifier::shift);
    const bool command = command_modifier(event.modifiers);
    if (multiline_ && accepts_tab_ && event.physical_key == PhysicalKey::tab && !command) {
        if (!read_only_) static_cast<void>(replace_selection("\t"));
        event.handled = true;
        return;
    }
    if (multiline_ && (event.physical_key == PhysicalKey::up ||
        event.physical_key == PhysicalKey::down ||
        event.physical_key == PhysicalKey::page_up ||
        event.physical_key == PhysicalKey::page_down ||
        event.physical_key == PhysicalKey::home ||
        event.physical_key == PhysicalKey::end)) {
        ensure_multiline_layout();
        const std::size_t row = caret_line();
        const bool home_end = event.physical_key == PhysicalKey::home ||
                              event.physical_key == PhysicalKey::end;
        Utf8Offset next;
        double goal = preferred_x_;
        if (home_end) {
            const bool first = event.physical_key == PhysicalKey::home;
            next = command ? (first ? Utf8Offset(0) : store_.utf8_size())
                : Utf8Offset(first ? visual_lines_[row].offsets.front()
                                   : visual_lines_[row].offsets.back());
            next_upstream_ = !command && !first;
        } else {
            if (goal < 0.0) goal = multiline_boundary_x(row, selection_.caret);
            const bool up = event.physical_key == PhysicalKey::up ||
                            event.physical_key == PhysicalKey::page_up;
            const bool page = event.physical_key == PhysicalKey::page_up ||
                              event.physical_key == PhysicalKey::page_down;
            const std::size_t step = page ? static_cast<std::size_t>(std::max(1.0,
                std::floor((local_bounds().height - 8.0) / line_height_))) : 1U;
            const std::size_t target = up ? row - std::min(row, step)
                : std::min(visual_lines_.size() - 1, row + step);
            next = position_in_line(target, goal);
            next_upstream_ = next.value() == visual_lines_[target].offsets.back();
        }
        set_selection(extend ? TextSelection{selection_.anchor, next} : TextSelection{next, next});
        if (!home_end) preferred_x_ = goal;
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::enter) {
        if (multiline_) {
            if (!read_only_) static_cast<void>(replace_selection(newline_));
            event.handled = true;
            return;
        }
        const std::string value(text());
        committed_.emit(value);
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::escape) {
        cancelled_.emit();
        event.handled = true;
        return;
    }
    if (command && event.physical_key == PhysicalKey::a) {
        select_all();
        event.handled = true;
        return;
    }
    if (command && event.physical_key == PhysicalKey::c) {
        static_cast<void>(copy());
        event.handled = true;
        return;
    }
    if (command && event.physical_key == PhysicalKey::x) {
        static_cast<void>(cut());
        event.handled = true;
        return;
    }
    if (command && event.physical_key == PhysicalKey::v) {
        static_cast<void>(paste());
        event.handled = true;
        return;
    }
    if (command && event.physical_key == PhysicalKey::z) {
        event.handled = includes(event.modifiers, Modifier::shift) ? redo() : undo();
        return;
    }
    if (command && event.physical_key == PhysicalKey::y) {
        event.handled = redo();
        return;
    }
    if (event.physical_key == PhysicalKey::left ||
        event.physical_key == PhysicalKey::right) {
        const bool line_navigation = includes(event.modifiers, Modifier::meta);
        const bool word_navigation = !line_navigation &&
            (includes(event.modifiers, Modifier::control) ||
             includes(event.modifiers, Modifier::alt));
        Utf8Offset next = selection_.caret;
        if (!extend && !selection_.empty()) {
            next = event.physical_key == PhysicalKey::left
                ? selection_.start() : selection_.end();
        } else if (line_navigation) {
            next = event.physical_key == PhysicalKey::left
                ? Utf8Offset(0U) : store_.utf8_size();
        } else if (word_navigation) {
            next = event.physical_key == PhysicalKey::left
                ? previous_word_boundary(selection_.caret)
                : next_word_boundary(selection_.caret);
        } else {
            next = event.physical_key == PhysicalKey::left
                ? store_.previous_grapheme_boundary(selection_.caret)
                : store_.next_grapheme_boundary(selection_.caret);
        }
        set_selection(extend ? TextSelection{selection_.anchor, next}
                             : TextSelection{next, next});
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::home ||
        event.physical_key == PhysicalKey::end) {
        const Utf8Offset next = event.physical_key == PhysicalKey::home
            ? Utf8Offset(0U) : store_.utf8_size();
        set_selection(extend ? TextSelection{selection_.anchor, next}
                             : TextSelection{next, next});
        event.handled = true;
        return;
    }
    if (!read_only_ && (event.physical_key == PhysicalKey::backspace ||
                        event.physical_key == PhysicalKey::delete_forward)) {
        if (!selection_.empty()) {
            event.handled = delete_selection();
        } else {
            const bool line_deletion = includes(event.modifiers, Modifier::meta);
            const bool word_deletion = !line_deletion &&
                (includes(event.modifiers, Modifier::control) ||
                 includes(event.modifiers, Modifier::alt));
            Utf8Offset other;
            if (line_deletion) {
                other = event.physical_key == PhysicalKey::backspace
                    ? Utf8Offset(0U) : store_.utf8_size();
            } else if (word_deletion) {
                other = event.physical_key == PhysicalKey::backspace
                    ? previous_word_boundary(selection_.caret)
                    : next_word_boundary(selection_.caret);
            } else {
                other = event.physical_key == PhysicalKey::backspace
                    ? store_.previous_grapheme_boundary(selection_.caret)
                    : store_.next_grapheme_boundary(selection_.caret);
            }
            event.handled = replace(std::min(other, selection_.caret),
                                    std::max(other, selection_.caret), {});
        }
    }
}

void TextBox::on_text_input(TextInputEvent& event) {
    if (!focused_ || !enabled() || read_only_ ||
        !validate_utf8(event.text_utf8).valid()) {
        return;
    }
    Utf8Offset start = selection_.start();
    Utf8Offset end = selection_.end();
    if (event.replacement_start >= 0 && event.replacement_length >= 0) {
        try {
            start = store_.utf8_offset(Utf16Offset(
                static_cast<std::size_t>(event.replacement_start)));
            end = store_.utf8_offset(Utf16Offset(
                static_cast<std::size_t>(event.replacement_start +
                                         event.replacement_length)));
        } catch (const std::out_of_range&) {
            return;
        }
    }
    event.handled = replace(start, end, event.text_utf8);
}

void TextBox::reset_caret_blink() {
    caret_upstream_ = false;
    reveal_pending_ = true;
    preferred_x_ = -1.0;
    caret_visible_ = true;
    caret_frame_.disconnect();
    schedule_caret_blink();
}

void TextBox::schedule_caret_blink() {
    if (focused_ && attached() && window() != nullptr) {
        caret_frame_ = (*window()).schedule_paint(
            shared_from_this(), FrameClock::now() + std::chrono::milliseconds(530));
    }
}

void TextBox::on_focus_changed(bool focused) {
    focused_ = focused;
    selecting_ = false;
    if (focused_) {
        reset_caret_blink();
    } else {
        caret_frame_.disconnect();
        caret_visible_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void TextBox::on_frame(FrameTime) {
    if (!focused_) {
        return;
    }
    caret_visible_ = !caret_visible_;
    invalidate(Dirty::paint);
    schedule_caret_blink();
}

SemanticDescriptor TextBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::text_box;
    descriptor.name = accessible_name();
    descriptor.value = password_protected() ? std::string{} : std::string(text());
    descriptor.description = accessible_description().empty()
        ? placeholder_ : accessible_description();
    if (read_only_) descriptor.states |= SemanticState::read_only;
    if (password_protected()) descriptor.states |= SemanticState::protected_content;
    descriptor.actions = {SemanticAction::focus};
    if (!read_only_) descriptor.actions.push_back(SemanticAction::set_value);
    descriptor.exposed = true;
    return descriptor;
}

bool TextBox::on_semantic_action(SemanticAction action, std::string_view value) {
    if (action == SemanticAction::set_value && !read_only_) {
        return replace(Utf8Offset(0), store_.utf8_size(), value);
    }
    return Panel::on_semantic_action(action, value);
}

void TextBox::on_detached_from_window() noexcept {
    caret_frame_.disconnect();
    focused_ = false;
    selecting_ = false;
}

} // namespace gui_forms
