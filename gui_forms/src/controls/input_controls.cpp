#include "gui_forms/input_controls.hpp"

#include "gui_forms/host.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <chrono>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

[[nodiscard]] std::uint64_t virtual_semantic_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

[[nodiscard]] bool includes(Modifier value, Modifier requested) noexcept {
    return (static_cast<std::uint8_t>(value) &
            static_cast<std::uint8_t>(requested)) != 0U;
}

[[nodiscard]] bool command_modifier(Modifier value) noexcept {
    return includes(value, Modifier::control) || includes(value, Modifier::meta);
}

enum class WordClass : std::uint8_t {
    spacing,
    word,
    punctuation,
};

[[nodiscard]] bool unicode_spacing(char32_t value) noexcept {
    return value == U' ' || (value >= U'\t' && value <= U'\r') ||
           value == U'\u0085' || value == U'\u00a0' || value == U'\u1680' ||
           (value >= U'\u2000' && value <= U'\u200a') ||
           value == U'\u2028' || value == U'\u2029' || value == U'\u202f' ||
           value == U'\u205f' || value == U'\u3000';
}

[[nodiscard]] bool unicode_punctuation_or_symbol(char32_t value) noexcept {
    if (value < U'\u0080') {
        return !((value >= U'a' && value <= U'z') ||
                 (value >= U'A' && value <= U'Z') ||
                 (value >= U'0' && value <= U'9') || value == U'_');
    }
    return (value >= U'\u2000' && value <= U'\u206f') ||
           (value >= U'\u2190' && value <= U'\u2bff') ||
           (value >= U'\u3001' && value <= U'\u303f') ||
           (value >= U'\ufe10' && value <= U'\ufe1f') ||
           (value >= U'\ufe30' && value <= U'\ufe4f') ||
           (value >= U'\uff01' && value <= U'\uff0f') ||
           (value >= U'\uff1a' && value <= U'\uff20') ||
           (value >= U'\uff3b' && value <= U'\uff40') ||
           (value >= U'\uff5b' && value <= U'\uff65') ||
           (value >= U'\U0001f000' && value <= U'\U0001faff');
}

[[nodiscard]] WordClass word_class(char32_t value) noexcept {
    if (unicode_spacing(value)) return WordClass::spacing;
    return unicode_punctuation_or_symbol(value)
        ? WordClass::punctuation : WordClass::word;
}

[[nodiscard]] bool valid_password_character(char32_t value) noexcept {
    return value == U'\0' ||
        (value >= U' ' && value <= U'\U0010ffff' &&
         !(value >= static_cast<char32_t>(0xd800U) &&
           value <= static_cast<char32_t>(0xdfffU)) &&
         value != U'\u2028' && value != U'\u2029');
}

[[nodiscard]] std::string utf8_scalar(char32_t value) {
    std::string result;
    if (value <= U'\u007f') {
        result.push_back(static_cast<char>(value));
    } else if (value <= U'\u07ff') {
        result.push_back(static_cast<char>(0xc0U | (value >> 6U)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    } else if (value <= U'\uffff') {
        result.push_back(static_cast<char>(0xe0U | (value >> 12U)));
        result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    } else {
        result.push_back(static_cast<char>(0xf0U | (value >> 18U)));
        result.push_back(static_cast<char>(0x80U | ((value >> 12U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | ((value >> 6U) & 0x3fU)));
        result.push_back(static_cast<char>(0x80U | (value & 0x3fU)));
    }
    return result;
}

class DropDownLayer final : public Panel {
public:
    explicit DropDownLayer(StableId stable_id) : Panel(std::move(stable_id)) {
        set_paint_plane(PaintPlane::overlay);
        set_border_style(BorderStyle::none);
        set_background(Color::rgba(0, 0, 0, 0));
    }

    [[nodiscard]] Event<>& dismissed() noexcept { return dismissed_; }

    void on_pointer(PointerEvent& event) override {
        if (event.action == PointerAction::down &&
            event.button == PointerButton::primary) {
            dismissed_.emit();
            event.handled = true;
        }
    }

    void on_key_preview(KeyEvent& event) override {
        if (event.action == KeyAction::down &&
            event.physical_key == PhysicalKey::escape) {
            dismissed_.emit();
            event.handled = true;
        }
    }

private:
    Event<> dismissed_;
};

class SpinButtons final : public Control {
public:
    explicit SpinButtons(StableId stable_id) : Control(std::move(stable_id)) {
        set_focusable(false);
        set_cursor(CursorKind::hand);
    }

    [[nodiscard]] Event<int>& stepped() noexcept { return stepped_; }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        const BasicControlStyle style;
        painter.fill_rect(bounds, style.face);
        painter.stroke_rect({0.5, 0.5, std::max(0.0, bounds.width - 1.0),
                             std::max(0.0, bounds.height - 1.0)}, style.border, 1.0);
        const double middle = std::floor(bounds.height * 0.5);
        painter.draw_line({0.0, middle}, {bounds.width, middle}, style.border, 1.0);
        const double x = bounds.width * 0.5;
        painter.draw_line({x - 3.5, middle - 4.0}, {x, middle - 7.0},
                          style.dark_border, 1.0);
        painter.draw_line({x, middle - 7.0}, {x + 3.5, middle - 4.0},
                          style.dark_border, 1.0);
        painter.draw_line({x - 3.5, middle + 4.0}, {x, middle + 7.0},
                          style.dark_border, 1.0);
        painter.draw_line({x, middle + 7.0}, {x + 3.5, middle + 4.0},
                          style.dark_border, 1.0);
    }

    void on_pointer(PointerEvent& event) override {
        if (!eligible_for_input() || event.button != PointerButton::primary) return;
        if (event.action == PointerAction::down) {
            const Rect bounds = absolute_bounds();
            stepped_.emit(event.position.y < bounds.y + bounds.height * 0.5 ? 1 : -1);
            event.handled = true;
        } else if (event.action == PointerAction::up) {
            event.handled = true;
        }
    }

private:
    Event<int> stepped_;
};

} // namespace

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
        [this] { return BindingValue{std::string(store_.utf8())}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(value, BindingValueKind::text);
            if (!converted) throw std::invalid_argument("TextBox.Text binding requires text");
            set_text(std::get<std::string>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return text_changed_.subscribe(owner,
                [changed = std::move(changed)](const std::string&) { changed(); });
        }, {}, {}});
}

void TextBox::set_text(std::string text) {
    require_mutable();
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

void TextBox::set_password_character(char32_t character) {
    require_mutable();
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
        auto* source = !undo_.empty() ? &undo_ : &redo_;
        history_bytes_ -= source->front().text.size();
        source->pop_front();
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
    if (read_only_ || start.value() > end.value() ||
        !store_.is_grapheme_boundary(start) ||
        !store_.is_grapheme_boundary(end) ||
        !validate_utf8(replacement).valid()) {
        return false;
    }
    const std::string_view existing = store_.utf8().substr(
        start.value(), end.value() - start.value());
    if (existing == replacement) {
        set_selection({Utf8Offset(start.value() + replacement.size()),
                       Utf8Offset(start.value() + replacement.size())});
        return true;
    }
    if (record_history) {
        push_history(undo_, snapshot());
        clear_redo();
    }
    static_cast<void>(store_.replace({start, end}, replacement));
    const Utf8Offset next(start.value() + replacement.size());
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
        window()->host_services() == nullptr) {
        return false;
    }
    return window()->host_services()->write_clipboard_text(selected_text()).accepted();
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
    if (read_only_ || window() == nullptr || window()->host_services() == nullptr) {
        return false;
    }
    const HostClipboardTextResult result =
        window()->host_services()->read_clipboard_text();
    return result.status.accepted() && result.has_text &&
        replace_selection(result.text_utf8);
}

void TextBox::set_selection(TextSelection selection, bool reveal_caret) {
    if (selection_ == selection) {
        if (reveal_caret) {
            reset_caret_blink();
        }
        return;
    }
    selection_ = selection;
    if (reveal_caret) {
        reset_caret_blink();
    }
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
    const auto found = std::lower_bound(layout_offsets_.begin(),
                                        layout_offsets_.end(), offset.value());
    const std::size_t index = found == layout_offsets_.end()
        ? layout_offsets_.size() - 1U
        : static_cast<std::size_t>(std::distance(layout_offsets_.begin(), found));
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
    const auto right = std::lower_bound(layout_positions_.begin(),
                                        layout_positions_.end(), content_x);
    if (right == layout_positions_.begin()) {
        return Utf8Offset(layout_offsets_.front());
    }
    if (right == layout_positions_.end()) {
        return Utf8Offset(layout_offsets_.back());
    }
    const std::size_t index = static_cast<std::size_t>(
        std::distance(layout_positions_.begin(), right));
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
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        if (window() != nullptr) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        const Utf8Offset position = position_at(local_x);
        const bool extend = includes(event.modifiers, Modifier::shift);
        set_selection(extend ? TextSelection{selection_.anchor, position}
                             : TextSelection{position, position});
        selecting_ = true;
        set_pointer_capture(true);
        event.handled = true;
    } else if (event.action == PointerAction::move && selecting_ &&
               has_pointer_capture()) {
        set_selection({selection_.anchor, position_at(local_x)});
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary && selecting_) {
        set_selection({selection_.anchor, position_at(local_x)});
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
    if (event.physical_key == PhysicalKey::enter) {
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
    caret_visible_ = true;
    caret_frame_.disconnect();
    schedule_caret_blink();
}

void TextBox::schedule_caret_blink() {
    if (focused_ && attached() && window() != nullptr) {
        caret_frame_ = window()->schedule_paint(
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
        set_text(std::string(value));
        select(Utf8Offset(text().size()), Utf8Offset(text().size()));
        return true;
    }
    return Panel::on_semantic_action(action, value);
}

void TextBox::on_detached_from_window() noexcept {
    caret_frame_.disconnect();
    focused_ = false;
    selecting_ = false;
}

ListBox::ListBox(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
}

void ListBox::set_items(std::vector<std::string> items) {
    require_mutable();
    for (const std::string& item : items) {
        if (!validate_utf8(item).valid()) {
            throw std::invalid_argument("ListBox items must be valid UTF-8");
        }
    }
    items_ = std::move(items);
    item_stable_ids_.clear();
    const std::vector<std::size_t> previous = selected_;
    selected_.erase(std::remove_if(selected_.begin(), selected_.end(),
        [this](std::size_t index) { return index >= items_.size(); }), selected_.end());
    if (active_index_ && *active_index_ >= items_.size()) active_index_.reset();
    if (anchor_index_ && *anchor_index_ >= items_.size()) anchor_index_.reset();
    top_index_ = items_.empty() ? 0U : std::min(top_index_, items_.size() - 1U);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    if (previous != selected_) {
        publish_change(selection_changed_,
                       ListSelectionChange{previous, selected_, active_index_});
    }
}

void ListBox::add_item(std::string item) {
    require_mutable();
    if (!validate_utf8(item).valid()) {
        throw std::invalid_argument("ListBox item must be valid UTF-8");
    }
    // Appending without an accompanying model identity returns the collection
    // to the deterministic index-derived identity policy.
    item_stable_ids_.clear();
    items_.push_back(std::move(item));
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ListBox::remove_item(std::size_t index) {
    require_mutable();
    if (index >= items_.size()) {
        throw std::out_of_range("ListBox item index is outside the collection");
    }
    const std::vector<std::size_t> previous = selected_;
    items_.erase(items_.begin() + static_cast<std::ptrdiff_t>(index));
    if (!item_stable_ids_.empty()) {
        item_stable_ids_.erase(item_stable_ids_.begin() +
                               static_cast<std::ptrdiff_t>(index));
    }
    std::vector<std::size_t> adjusted;
    for (const std::size_t selected : selected_) {
        if (selected != index) adjusted.push_back(selected > index ? selected - 1U : selected);
    }
    selected_ = std::move(adjusted);
    if (active_index_) {
        if (*active_index_ == index) active_index_.reset();
        else if (*active_index_ > index) --*active_index_;
    }
    if (anchor_index_) {
        if (*anchor_index_ == index) anchor_index_.reset();
        else if (*anchor_index_ > index) --*anchor_index_;
    }
    top_index_ = items_.empty() ? 0U : std::min(top_index_, items_.size() - 1U);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    if (previous != selected_) {
        publish_change(selection_changed_,
                       ListSelectionChange{previous, selected_, active_index_});
    }
}

void ListBox::clear_items() {
    set_items({});
}

void ListBox::set_item_stable_ids(std::vector<std::string> stable_ids) {
    require_mutable();
    if (stable_ids.size() != items_.size()) {
        throw std::invalid_argument(
            "ListBox stable item IDs must match the item count");
    }
    std::vector<std::string> sorted = stable_ids;
    for (const std::string& stable_id : sorted) {
        if (stable_id.empty() || !validate_utf8(stable_id).valid()) {
            throw std::invalid_argument(
                "ListBox stable item IDs must be nonempty valid UTF-8");
        }
    }
    std::sort(sorted.begin(), sorted.end());
    if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end()) {
        throw std::invalid_argument("ListBox stable item IDs must be unique");
    }
    item_stable_ids_ = std::move(stable_ids);
    invalidate(Dirty::semantics);
}

std::string ListBox::item_stable_id(std::size_t index) const {
    if (index >= items_.size()) {
        throw std::out_of_range(
            "ListBox item stable ID index is outside the collection");
    }
    if (!item_stable_ids_.empty()) return item_stable_ids_[index];
    return std::string(stable_id().value()) + ".item." +
           std::to_string(index);
}

void ListBox::set_selection_mode(ListSelectionMode mode) {
    require_mutable();
    if (selection_mode_ == mode) return;
    selection_mode_ = mode;
    if (mode == ListSelectionMode::one && selected_.size() > 1U) {
        const std::size_t retained = active_index_.value_or(selected_.front());
        apply_selection({retained}, retained);
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

std::optional<std::size_t> ListBox::selected_index() const noexcept {
    return selected_.empty() ? std::optional<std::size_t>{}
                             : std::optional<std::size_t>{selected_.front()};
}

void ListBox::apply_selection(std::vector<std::size_t> selection,
                              std::optional<std::size_t> active) {
    std::sort(selection.begin(), selection.end());
    selection.erase(std::unique(selection.begin(), selection.end()), selection.end());
    const std::vector<std::size_t> previous = selected_;
    const auto previous_active = active_index_;
    selected_ = std::move(selection);
    active_index_ = active;
    if (previous == selected_ && previous_active == active_index_) return;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selection_changed_,
                   ListSelectionChange{previous, selected_, active_index_});
}

void ListBox::select_index(std::size_t index, bool extend, bool toggle) {
    require_mutable();
    if (index >= items_.size()) {
        throw std::out_of_range("ListBox selection index is outside the collection");
    }
    if (selection_mode_ == ListSelectionMode::one) {
        selected_ == std::vector<std::size_t>{index}
            ? apply_selection(selected_, index)
            : apply_selection({index}, index);
        anchor_index_ = index;
    } else if (extend && anchor_index_) {
        std::vector<std::size_t> next;
        const std::size_t first = std::min(*anchor_index_, index);
        const std::size_t last = std::max(*anchor_index_, index);
        next.reserve(last - first + 1U);
        for (std::size_t item = first; item <= last; ++item) next.push_back(item);
        apply_selection(std::move(next), index);
    } else if (toggle) {
        std::vector<std::size_t> next = selected_;
        const auto found = std::find(next.begin(), next.end(), index);
        if (found == next.end()) next.push_back(index);
        else next.erase(found);
        apply_selection(std::move(next), index);
        anchor_index_ = index;
    } else {
        apply_selection({index}, index);
        anchor_index_ = index;
    }
    ensure_visible(index);
}

void ListBox::clear_selection() {
    require_mutable();
    apply_selection({}, {});
    anchor_index_.reset();
}

void ListBox::set_top_index(std::size_t index) {
    require_mutable();
    if (items_.empty()) {
        if (index != 0U) throw std::out_of_range("empty ListBox top index must be zero");
    } else if (index >= items_.size()) {
        throw std::out_of_range("ListBox top index is outside the collection");
    }
    if (top_index_ == index) return;
    top_index_ = index;
    invalidate(Dirty::paint | Dirty::semantics);
}

void ListBox::set_item_height(double height) {
    require_mutable();
    if (!std::isfinite(height) || height < 12.0 || height > 256.0) {
        throw std::invalid_argument("ListBox item height must be finite and between 12 and 256");
    }
    if (item_height_ == height) return;
    item_height_ = height;
    invalidate(Dirty::measure | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void ListBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("ListBox font specification is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

std::size_t ListBox::visible_row_count() const noexcept {
    double height = local_bounds().height;
    if (height <= 4.0) {
        height = requested_bounds().height;
    }
    const double row_height = item_height_ * effective_text_scale();
    return std::max<std::size_t>(1U, static_cast<std::size_t>(
        std::floor(std::max(0.0, height - 4.0) / row_height)));
}

void ListBox::ensure_visible(std::size_t index) {
    if (local_bounds().height <= 4.0 && requested_bounds().height <= 4.0) {
        return;
    }
    const std::size_t visible = visible_row_count();
    if (index < top_index_) top_index_ = index;
    else if (index >= top_index_ + visible) top_index_ = index - visible + 1U;
}

void ListBox::arrange(Rect final_bounds) {
    Panel::arrange(final_bounds);
    if (active_index_) {
        ensure_visible(*active_index_);
    }
}

std::optional<std::size_t> ListBox::index_at(Point absolute) const noexcept {
    const Rect bounds = absolute_bounds();
    if (!bounds.contains(absolute)) return {};
    const double local_y = absolute.y - bounds.y - 2.0;
    if (local_y < 0.0) return {};
    const double row_height = item_height_ * effective_text_scale();
    const std::size_t index = top_index_ +
        static_cast<std::size_t>(std::floor(local_y / row_height));
    return index < items_.size() ? std::optional<std::size_t>{index}
                                 : std::optional<std::size_t>{};
}

void ListBox::on_paint(Painter& painter, Rect damage) {
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
    const FontSpec font = effective_font(font_);
    const double row_height = item_height_ * effective_text_scale();
    painter.save();
    painter.clip_rect({2.0, 2.0, std::max(0.0, bounds.width - 4.0),
                       std::max(0.0, bounds.height - 4.0)});
    const std::size_t visible = visible_row_count() + 1U;
    const std::size_t end = std::min(items_.size(), top_index_ + visible);
    for (std::size_t index = top_index_; index < end; ++index) {
        const double y = 2.0 + static_cast<double>(index - top_index_) * row_height;
        const bool selected = std::binary_search(selected_.begin(), selected_.end(), index);
        const bool hovered = hovered_index_ == index;
        const Rect row{2.0, y, std::max(0.0, bounds.width - 4.0), row_height};
        const ControlVisualRecipe& row_recipe = effective_theme().resolve(
            ControlVisualRole::selection,
            visual_context(hovered, false, selected, focused_));
        if (themed && (selected || hovered)) {
            paint_surface_material(painter, row, row_recipe.material);
        } else if (selected) {
            painter.fill_rect(row, focused_ ? style().accent
                                            : style().accent_light);
        } else if (hovered) {
            painter.fill_rect(row, style().face_light);
        }
        paint_row_adornment(painter, index, row, selected, focused_);
        painter.draw_text_utf8({row_text_left(),
                                y + std::max(font.size,
                                             row_height * 0.5 + 4.0)},
                               items_[index], font,
                               themed ? (selected || hovered
                                             ? row_recipe.text
                                             : editor_recipe.text)
                                      : selected && focused_
                                            ? style().highlight
                                            : enabled() ? style().text
                                                        : style().disabled_text);
        if (focused_ && active_index_ == index) {
            painter.stroke_rect({3.5, y + 1.5, std::max(0.0, bounds.width - 7.0),
                                 std::max(0.0, row_height - 3.0)},
                                themed ? row_recipe.focus_ring
                                       : selected ? style().highlight
                                                  : style().accent,
                                themed ? row_recipe.focus_width : 1.0);
        }
    }
    painter.restore();
}

double ListBox::row_text_left() const noexcept {
    return 8.0;
}

void ListBox::paint_row_adornment(Painter&, std::size_t, Rect, bool,
                                  bool) const {}

void ListBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::move) {
        const auto next = index_at(event.position);
        if (next != hovered_index_) {
            hovered_index_ = next;
            invalidate(Dirty::paint);
        }
        return;
    }
    if (event.action == PointerAction::leave) {
        hovered_index_.reset();
        invalidate(Dirty::paint);
        return;
    }
    if (event.action == PointerAction::wheel) {
        if (!items_.empty() && event.wheel_delta.y != 0.0) {
            const int direction = event.wheel_delta.y > 0.0 ? -1 : 1;
            const std::ptrdiff_t next = std::clamp<std::ptrdiff_t>(
                static_cast<std::ptrdiff_t>(top_index_) + direction * 3,
                0, static_cast<std::ptrdiff_t>(items_.size() - 1U));
            set_top_index(static_cast<std::size_t>(next));
            event.handled = true;
        }
        return;
    }
    if (event.action == PointerAction::down && event.button == PointerButton::primary) {
        if (window() != nullptr) static_cast<void>(window()->request_focus(shared_from_this()));
        if (const auto index = index_at(event.position)) {
            select_index(*index, includes(event.modifiers, Modifier::shift),
                         command_modifier(event.modifiers));
            event.handled = true;
        }
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary) {
        if (const auto index = index_at(event.position);
            index && active_index_ == index) {
            item_activated_.emit(*index);
            event.handled = true;
        }
    }
}

void ListBox::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down || items_.empty()) return;
    if (event.physical_key == PhysicalKey::enter && active_index_) {
        item_activated_.emit(*active_index_);
        event.handled = true;
        return;
    }
    std::optional<std::size_t> next;
    const std::size_t current = active_index_.value_or(0U);
    if (event.physical_key == PhysicalKey::up) next = current == 0U ? 0U : current - 1U;
    else if (event.physical_key == PhysicalKey::down) next = std::min(current + 1U, items_.size() - 1U);
    else if (event.physical_key == PhysicalKey::home) next = 0U;
    else if (event.physical_key == PhysicalKey::end) next = items_.size() - 1U;
    else if (event.physical_key == PhysicalKey::page_up) next = current > visible_row_count() ? current - visible_row_count() : 0U;
    else if (event.physical_key == PhysicalKey::page_down) next = std::min(current + visible_row_count(), items_.size() - 1U);
    if (next) {
        select_index(*next, includes(event.modifiers, Modifier::shift), false);
        event.handled = true;
    }
}

void ListBox::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor ListBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::list;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    if (const auto selected = selected_index()) {
        descriptor.value = items_[*selected];
    } else if (!selected_.empty()) {
        descriptor.value = std::to_string(selected_.size()) + " items selected";
    }
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> ListBox::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    nodes.reserve(items_.size());
    const Rect list_bounds = absolute_bounds();
    const std::size_t visible_count = visible_row_count();
    const std::size_t visible_end = std::min(items_.size(), top_index_ + visible_count);
    const double row_height = item_height_ * effective_text_scale();
    for (std::size_t index = 0; index < items_.size(); ++index) {
        SemanticNode node;
        node.stable_id = item_stable_id(index);
        node.runtime_id = virtual_semantic_runtime_id(node.stable_id);
        node.role = SemanticRole::list_item;
        node.name = items_[index];
        node.value = items_[index];
        const double row_y = list_bounds.y + 2.0 +
            (static_cast<double>(index) - static_cast<double>(top_index_)) *
                row_height;
        node.bounds = Rect::intersection(
            {list_bounds.x + 2.0, row_y, std::max(0.0, list_bounds.width - 4.0),
             row_height},
            list_bounds);
        if (effectively_enabled()) node.states |= SemanticState::enabled;
        node.states |= SemanticState::focusable;
        if (index >= top_index_ && index < visible_end) {
            node.states |= SemanticState::visible;
        }
        if (std::binary_search(selected_.begin(), selected_.end(), index)) {
            node.states |= SemanticState::selected;
        }
        if (focused_ && active_index_ == index) node.states |= SemanticState::focused;
        node.actions = {SemanticAction::focus, SemanticAction::select,
                        SemanticAction::press};
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool ListBox::on_semantic_child_action(std::string_view child_stable_id,
                                       SemanticAction action,
                                       std::string_view) {
    std::size_t index{};
    if (!item_stable_ids_.empty()) {
        const auto found = std::find(item_stable_ids_.begin(),
                                     item_stable_ids_.end(), child_stable_id);
        if (found == item_stable_ids_.end()) return false;
        index = static_cast<std::size_t>(
            std::distance(item_stable_ids_.begin(), found));
    } else {
        const std::string prefix = std::string(stable_id().value()) + ".item.";
        if (!child_stable_id.starts_with(prefix)) return false;
        const std::string_view suffix = child_stable_id.substr(prefix.size());
        const auto parsed = std::from_chars(
            suffix.data(), suffix.data() + suffix.size(), index);
        if (parsed.ec != std::errc{} ||
            parsed.ptr != suffix.data() + suffix.size() ||
            index >= items_.size()) return false;
    }
    if (action != SemanticAction::focus && action != SemanticAction::select &&
        action != SemanticAction::press) return false;
    if (window() != nullptr) {
        static_cast<void>(window()->request_focus(shared_from_this()));
    }
    select_index(index, false, false);
    if (action == SemanticAction::press && is_alive()) {
        item_activated_.emit(index);
    }
    return true;
}

CheckedListBox::CheckedListBox(StableId stable_id)
    : ListBox(std::move(stable_id)) {}

void CheckedListBox::set_items(std::vector<std::string> items) {
    const std::vector<CheckState> previous = check_states_;
    check_states_.assign(items.size(), CheckState::unchecked);
    try {
        ListBox::set_items(std::move(items));
    } catch (...) {
        // If ListBox rejected the new collection before mutation, restore the
        // matching check model. If a user callback threw after mutation, the
        // new sizes already match and are the only coherent retained state.
        if (this->items().size() == previous.size()) check_states_ = previous;
        throw;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::add_item(std::string item) {
    add_item(std::move(item), CheckState::unchecked);
}

void CheckedListBox::add_item(std::string item, CheckState state) {
    if (state != CheckState::unchecked && state != CheckState::checked &&
        state != CheckState::indeterminate) {
        throw std::invalid_argument("invalid CheckedListBox item state");
    }
    ListBox::add_item(std::move(item));
    try {
        check_states_.push_back(state);
    } catch (...) {
        ListBox::remove_item(items().size() - 1U);
        throw;
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::remove_item(std::size_t index) {
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    check_states_.erase(check_states_.begin() + static_cast<std::ptrdiff_t>(index));
    ListBox::remove_item(index);
    invalidate(Dirty::paint | Dirty::semantics);
}

void CheckedListBox::clear_items() {
    ListBox::clear_items();
    check_states_.clear();
    invalidate(Dirty::paint | Dirty::semantics);
}

CheckState CheckedListBox::item_check_state(std::size_t index) const {
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    return check_states_[index];
}

bool CheckedListBox::item_checked(std::size_t index) const {
    return item_check_state(index) == CheckState::checked;
}

void CheckedListBox::set_item_check_state(std::size_t index,
                                          CheckState state) {
    require_mutable();
    if (index >= check_states_.size()) {
        throw std::out_of_range("CheckedListBox item index");
    }
    if (state != CheckState::unchecked && state != CheckState::checked &&
        state != CheckState::indeterminate) {
        throw std::invalid_argument("invalid CheckedListBox item state");
    }
    if (check_states_[index] == state) return;
    ItemCheckEvent change{index, check_states_[index], state, false};
    item_checking_.emit(change);
    if (!is_alive() || change.cancel) return;
    if (change.new_state != CheckState::unchecked &&
        change.new_state != CheckState::checked &&
        change.new_state != CheckState::indeterminate) {
        throw std::invalid_argument(
            "CheckedListBox ItemCheck callback produced an invalid state");
    }
    if (check_states_[index] == change.new_state) return;
    check_states_[index] = change.new_state;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(item_check_state_changed_, index, check_states_[index]);
}

void CheckedListBox::set_item_checked(std::size_t index, bool checked) {
    set_item_check_state(index, checked ? CheckState::checked
                                        : CheckState::unchecked);
}

void CheckedListBox::toggle_item(std::size_t index) {
    const CheckState current = item_check_state(index);
    set_item_check_state(index, current == CheckState::checked
                                    ? CheckState::unchecked
                                    : CheckState::checked);
}

std::vector<std::size_t> CheckedListBox::checked_indices() const {
    std::vector<std::size_t> result;
    for (std::size_t index = 0U; index < check_states_.size(); ++index) {
        if (check_states_[index] == CheckState::checked) result.push_back(index);
    }
    return result;
}

void CheckedListBox::set_check_on_click(bool enabled) {
    require_mutable();
    if (check_on_click_ == enabled) return;
    check_on_click_ = enabled;
    invalidate(Dirty::semantics);
}

void CheckedListBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) {
        ListBox::on_pointer(event);
        return;
    }
    const auto index = event.action == PointerAction::down &&
                           event.button == PointerButton::primary
        ? index_at(event.position) : std::optional<std::size_t>{};
    const bool was_selected = index &&
        std::binary_search(selected_indices().begin(), selected_indices().end(),
                           *index);
    ListBox::on_pointer(event);
    if (!is_alive() || !index || *index >= items().size()) return;
    if (check_on_click_ || was_selected) {
        toggle_item(*index);
        event.handled = true;
    }
}

void CheckedListBox::on_key(KeyEvent& event) {
    if (event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::space && focused_for_extension() &&
        enabled()) {
        if (const auto index = active_index_for_extension()) {
            toggle_item(*index);
            event.handled = true;
        }
        return;
    }
    ListBox::on_key(event);
}

double CheckedListBox::row_text_left() const noexcept {
    return 28.0;
}

void CheckedListBox::paint_row_adornment(Painter& painter, std::size_t index,
                                          Rect row, bool selected,
                                          bool focused) const {
    if (index >= check_states_.size()) return;
    const double size = std::min(14.0, std::max(8.0, row.height - 8.0));
    const double x = row.x + 5.0;
    const double y = row.y + (row.height - size) * 0.5;
    const Color paper = selected && focused ? style().highlight : style().paper;
    const Color mark = selected && focused ? style().accent : style().accent;
    painter.fill_rect({x, y, size, size}, paper);
    painter.stroke_rect({x + 0.5, y + 0.5, size - 1.0, size - 1.0},
                        style().dark_border, 1.0);
    if (check_states_[index] == CheckState::checked) {
        painter.draw_line({x + 3.0, y + size * 0.52},
                          {x + size * 0.44, y + size - 3.0}, mark, 2.0);
        painter.draw_line({x + size * 0.44, y + size - 3.0},
                          {x + size - 2.5, y + 2.5}, mark, 2.0);
    } else if (check_states_[index] == CheckState::indeterminate) {
        painter.fill_rect({x + 3.0, y + size * 0.5 - 1.5,
                           std::max(0.0, size - 6.0), 3.0}, mark);
    }
}

std::vector<SemanticNode> CheckedListBox::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes = ListBox::semantic_virtual_children();
    for (std::size_t index = 0U;
         index < nodes.size() && index < check_states_.size(); ++index) {
        nodes[index].role = SemanticRole::check_list_item;
        if (check_states_[index] == CheckState::checked) {
            nodes[index].states |= SemanticState::checked;
            nodes[index].value = "checked";
        } else if (check_states_[index] == CheckState::indeterminate) {
            nodes[index].states |= SemanticState::mixed;
            nodes[index].value = "indeterminate";
        } else {
            nodes[index].value = "unchecked";
        }
    }
    return nodes;
}

bool CheckedListBox::on_semantic_child_action(std::string_view stable_id,
                                               SemanticAction action,
                                               std::string_view value) {
    if (action != SemanticAction::press) {
        return ListBox::on_semantic_child_action(stable_id, action, value);
    }
    const std::string prefix = std::string(this->stable_id().value()) + ".item.";
    if (!stable_id.starts_with(prefix)) return false;
    const std::string_view suffix = stable_id.substr(prefix.size());
    std::size_t index{};
    const auto parsed = std::from_chars(suffix.data(), suffix.data() + suffix.size(),
                                        index);
    if (parsed.ec != std::errc{} || parsed.ptr != suffix.data() + suffix.size() ||
        index >= items().size()) return false;
    if (window() != nullptr) {
        static_cast<void>(window()->request_focus(shared_from_this()));
    }
    select_index(index, false, false);
    if (is_alive()) toggle_item(index);
    return true;
}

ComboBox::ComboBox(StableId stable_id) : Panel(std::move(stable_id)) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::sunken);
    set_focusable(true);
    set_cursor(CursorKind::hand);
    PropertyDescriptor items_descriptor;
    items_descriptor.name = "Items";
    items_descriptor.kind = BindingValueKind::collection;
    items_descriptor.category = "Data";
    items_descriptor.description =
        "Ordered text items presented by the drop-down.";
    items_descriptor.default_value = BindingValue{make_property_collection(
        "String", BindingValueKind::text, {})};
    items_descriptor.invalidation_effects =
        Dirty::measure | Dirty::paint | Dirty::semantics;
    items_descriptor.serialization_visibility =
        PropertySerializationVisibility::content;
    items_descriptor.bindable = false;
    define_bindable_property({
        std::move(items_descriptor),
        [this] {
            std::vector<BindingValue> values;
            values.reserve(items_.size());
            for (const std::string& item : items_) values.emplace_back(item);
            return BindingValue{make_property_collection(
                "String", BindingValueKind::text, std::move(values))};
        },
        [this](const BindingValue& value) {
            const auto* collection =
                std::get_if<PropertyCollectionValue>(&value);
            if (!collection || !*collection ||
                collection->item_kind() != BindingValueKind::text) {
                throw std::invalid_argument(
                    "ComboBox.Items requires a homogeneous text collection");
            }
            std::vector<std::string> items;
            const auto values = property_collection_items(*collection);
            items.reserve(values.size());
            for (const BindingValue& item : values) {
                items.push_back(std::get<std::string>(item));
            }
            set_items(std::move(items));
        },
        [this](Component& owner, std::function<void()> changed) {
            return items_changed_.subscribe(owner, std::move(changed));
        },
        [this] { set_items({}); },
        [this] { return !items_.empty(); }});
    define_bindable_property({
        {"SelectedIndex", BindingValueKind::signed_integer, "Behavior",
         "Zero-based selected item index, or -1 when no item is selected.",
         BindingValue{std::int64_t{-1}}, Dirty::paint | Dirty::semantics},
        [this] {
            return BindingValue{selected_index_
                ? static_cast<std::int64_t>(*selected_index_)
                : std::int64_t{-1}};
        },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(
                value, BindingValueKind::signed_integer);
            if (!converted) {
                throw std::invalid_argument(
                    "ComboBox.SelectedIndex binding requires an integer");
            }
            const std::int64_t index = std::get<std::int64_t>(*converted);
            if (index == -1) {
                set_selected_index(std::nullopt);
                return;
            }
            if (index < 0) {
                throw std::out_of_range(
                    "ComboBox.SelectedIndex binding must be -1 or non-negative");
            }
            set_selected_index(static_cast<std::size_t>(index));
        },
        [this](Component& owner, std::function<void()> changed) {
            return selected_index_changed_.subscribe(owner,
                [changed = std::move(changed)](
                    std::optional<std::size_t>) { changed(); });
        }, {}, {}});
    define_bindable_property({
        {"Text", BindingValueKind::text, "Appearance",
         "Text of the selected item, or empty when no item is selected.",
         BindingValue{std::string{}}, Dirty::paint | Dirty::semantics},
        [this] { return BindingValue{std::string(selected_text())}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(
                value, BindingValueKind::text);
            if (!converted) {
                throw std::invalid_argument("ComboBox.Text binding requires text");
            }
            const std::string& text = std::get<std::string>(*converted);
            const auto found = std::find(items_.begin(), items_.end(), text);
            set_selected_index(found == items_.end()
                ? std::optional<std::size_t>{}
                : std::optional<std::size_t>{static_cast<std::size_t>(
                      std::distance(items_.begin(), found))});
        },
        [this](Component& owner, std::function<void()> changed) {
            return selected_index_changed_.subscribe(owner,
                [changed = std::move(changed)](
                    std::optional<std::size_t>) { changed(); });
        },
        [this] { set_selected_index(std::nullopt); },
        [this] { return selected_index_.has_value(); }});
}

void ComboBox::set_items(std::vector<std::string> items) {
    require_mutable();
    for (const std::string& item : items) {
        if (!validate_utf8(item).valid()) {
            throw std::invalid_argument("ComboBox items must be valid UTF-8");
        }
    }
    if (items_ == items) return;
    items_ = std::move(items);
    if (selected_index_ && *selected_index_ >= items_.size()) {
        selected_index_.reset();
        publish_change(selected_index_changed_, selected_index_);
        if (!is_alive()) return;
    }
    if (popup_list_) popup_list_->set_items(items_);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(items_changed_);
}

void ComboBox::add_item(std::string item) {
    require_mutable();
    if (!validate_utf8(item).valid()) {
        throw std::invalid_argument("ComboBox item must be valid UTF-8");
    }
    items_.push_back(std::move(item));
    if (popup_list_) popup_list_->set_items(items_);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
    publish_change(items_changed_);
}

void ComboBox::set_selected_index(std::optional<std::size_t> index) {
    require_mutable();
    if (index && *index >= items_.size()) {
        throw std::out_of_range("ComboBox selection index is outside the collection");
    }
    if (selected_index_ == index) return;
    selected_index_ = index;
    if (popup_list_) {
        if (index) popup_list_->select_index(*index);
        else popup_list_->clear_selection();
    }
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(selected_index_changed_, selected_index_);
}

std::string_view ComboBox::selected_text() const noexcept {
    return selected_index_ ? std::string_view(items_[*selected_index_])
                           : std::string_view{};
}

void ComboBox::set_placeholder_text(std::string text) {
    require_mutable();
    if (!validate_utf8(text).valid()) {
        throw std::invalid_argument("ComboBox placeholder must be valid UTF-8");
    }
    if (placeholder_ == text) return;
    placeholder_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics);
}

void ComboBox::set_maximum_drop_down_items(std::size_t count) {
    require_mutable();
    if (count == 0U || count > 64U) {
        throw std::invalid_argument("ComboBox drop-down row count must be between 1 and 64");
    }
    if (maximum_drop_down_items_ == count) return;
    maximum_drop_down_items_ = count;
    if (dropped_down_) {
        close_drop_down();
        open_drop_down();
    }
    invalidate(Dirty::semantics);
}

void ComboBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("ComboBox font specification is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    if (popup_list_) popup_list_->set_font(font);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void ComboBox::set_dropped_down(bool dropped_down) {
    require_mutable();
    if (dropped_down == dropped_down_) return;
    if (dropped_down) open_drop_down();
    else close_drop_down();
}

void ComboBox::open_drop_down() {
    if (dropped_down_ || !attached() || window() == nullptr || items_.empty()) return;
    const Control::Ptr owner = window()->root();
    if (!owner) return;
    const Rect combo = absolute_bounds();
    const Size client = window()->client_size();
    const std::size_t rows = std::min(maximum_drop_down_items_, items_.size());
    const double popup_height = static_cast<double>(rows) *
        26.0 * effective_text_scale() + 4.0;
    const double popup_y = combo.y + combo.height + popup_height <= client.height
        ? combo.y + combo.height : std::max(0.0, combo.y - popup_height);

    const std::string prefix(stable_id().value());
    auto layer = make_control<DropDownLayer>(StableId(prefix + ".popup.layer"));
    layer->set_requested_bounds({0.0, 0.0, client.width, client.height});
    auto list = make_control<ListBox>(StableId(prefix + ".popup.list"));
    list->set_paint_plane(PaintPlane::overlay);
    list->set_items(items_);
    list->set_font(font_);
    list->set_requested_bounds({combo.x, popup_y, combo.width, popup_height});
    if (selected_index_) {
        list->select_index(*selected_index_);
    }
    layer->add_child(list);
    PopupToken popup_token = window()->open_popup(shared_from_this(), layer);

    popup_layer_ = layer;
    popup_list_ = list;
    popup_token_ = std::move(popup_token);
    const std::weak_ptr<ComboBox> weak =
        std::static_pointer_cast<ComboBox>(shared_from_this());
    popup_selection_ = list->selection_changed().subscribe(
        *this, [weak](const ListSelectionChange& change) {
            if (const auto combo = weak.lock(); change.active_index) {
                combo->set_selected_index(change.active_index);
            }
        });
    popup_activation_ = list->item_activated().subscribe(
        *this, [weak](std::size_t index) {
            if (const auto combo = weak.lock()) combo->commit_popup_selection(index);
        });
    popup_dismissal_ = layer->dismissed().subscribe(*this, [weak] {
        if (const auto combo = weak.lock()) combo->close_drop_down();
    });
    if (Event<>* closed = popup_token_.closed_event()) {
        popup_revocation_ = closed->subscribe(*this, [weak] {
            if (const auto combo = weak.lock()) combo->on_popup_revoked();
        });
    }
    popup_scope_ = window()->begin_focus_scope(layer, list).value;
    dropped_down_ = true;
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(drop_down_changed_, true);
}

void ComboBox::close_drop_down() {
    if (!dropped_down_ && !popup_layer_) return;
    closing_popup_ = true;
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(FocusScopeId{popup_scope_}));
    }
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_revocation_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    closing_popup_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void ComboBox::on_popup_revoked() {
    if (closing_popup_) return;
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    if (window() != nullptr && popup_scope_ != 0U) {
        static_cast<void>(window()->end_focus_scope(FocusScopeId{popup_scope_},
                                                     FocusScopeCloseReason::owner_unavailable));
    }
    popup_scope_ = 0U;
    popup_list_.reset();
    popup_layer_.reset();
    const bool changed = dropped_down_;
    dropped_down_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(drop_down_changed_, false);
}

void ComboBox::commit_popup_selection(std::size_t index) {
    set_selected_index(index);
    close_drop_down();
}

void ComboBox::on_paint(Painter& painter, Rect damage) {
    const Rect bounds = local_bounds();
    const bool themed = !has_background_override() && !has_style_override();
    const ControlVisualRecipe& editor_recipe = effective_theme().resolve(
        ControlVisualRole::editor,
        visual_context(false, dropped_down_, false, focused_));
    if (themed) {
        paint_surface_material(painter, bounds, editor_recipe.material);
    } else {
        Panel::on_paint(painter, damage);
    }
    const FontSpec font = effective_font(font_);
    const double button_width = std::min(24.0, bounds.width);
    const Rect button_bounds{std::max(0.0, bounds.width - button_width), 1.0,
                             std::max(0.0, button_width - 1.0),
                             std::max(0.0, bounds.height - 2.0)};
    const ControlVisualRecipe& button_recipe = effective_theme().resolve(
        ControlVisualRole::choice,
        visual_context(false, dropped_down_, dropped_down_, focused_));
    if (themed) {
        paint_surface_material(painter, button_bounds, button_recipe.material);
    } else {
        painter.fill_rect(button_bounds, style().face);
    }
    painter.draw_line({bounds.width - button_width, 1.0},
                      {bounds.width - button_width, bounds.height - 1.0},
                      themed ? (button_recipe.material.border
                                    ? button_recipe.material.border->color
                                    : button_recipe.glyph)
                             : style().border,
                      1.0);
    const double center_x = bounds.width - button_width * 0.5;
    const double center_y = bounds.height * 0.5 + (dropped_down_ ? 2.0 : -1.0);
    const double direction = dropped_down_ ? -1.0 : 1.0;
    painter.draw_line({center_x - 4.0, center_y - direction * 2.0},
                      {center_x, center_y + direction * 2.0},
                      themed ? button_recipe.glyph : style().dark_border, 1.0);
    painter.draw_line({center_x, center_y + direction * 2.0},
                      {center_x + 4.0, center_y - direction * 2.0},
                      themed ? button_recipe.glyph : style().dark_border, 1.0);
    const std::string_view text = selected_index_ ? selected_text()
                                                  : std::string_view(placeholder_);
    painter.save();
    painter.clip_rect({5.0, 2.0,
                       std::max(0.0, bounds.width - button_width - 8.0),
                       std::max(0.0, bounds.height - 4.0)});
    painter.draw_text_utf8({7.0, std::max(font.size,
                            (bounds.height + font.size) * 0.5 - 1.0)},
                           text, font,
                           themed ? (selected_index_ ? editor_recipe.text
                                                     : editor_recipe.muted_text)
                                  : selected_index_ ? style().text
                                                    : style().disabled_text);
    painter.restore();
    if (focused_ || dropped_down_) {
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

void ComboBox::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::down && event.button == PointerButton::primary) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
    } else if (event.action == PointerAction::up &&
               event.button == PointerButton::primary) {
        event.handled = true;
    }
}

void ComboBox::on_key(KeyEvent& event) {
    if (!focused_ || !enabled() || event.action != KeyAction::down) return;
    if (event.physical_key == PhysicalKey::escape && dropped_down_) {
        close_drop_down();
        event.handled = true;
        return;
    }
    if (event.physical_key == PhysicalKey::f4 ||
        (event.physical_key == PhysicalKey::down && includes(event.modifiers, Modifier::alt)) ||
        event.physical_key == PhysicalKey::space) {
        set_dropped_down(!dropped_down_);
        event.handled = true;
        return;
    }
    if (!dropped_down_ && !items_.empty() &&
        (event.physical_key == PhysicalKey::up ||
         event.physical_key == PhysicalKey::down ||
         event.physical_key == PhysicalKey::home ||
         event.physical_key == PhysicalKey::end)) {
        std::size_t next = selected_index_.value_or(0U);
        if (event.physical_key == PhysicalKey::up) next = next == 0U ? 0U : next - 1U;
        else if (event.physical_key == PhysicalKey::down) next = std::min(next + 1U, items_.size() - 1U);
        else if (event.physical_key == PhysicalKey::home) next = 0U;
        else next = items_.size() - 1U;
        set_selected_index(next);
        event.handled = true;
    }
}

void ComboBox::on_focus_changed(bool focused) {
    focused_ = focused;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor ComboBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::combo_box;
    descriptor.name = accessible_name();
    descriptor.value = std::string(selected_text());
    descriptor.description = accessible_description().empty()
        ? placeholder_ : accessible_description();
    if (dropped_down_) descriptor.states |= SemanticState::expanded;
    descriptor.actions = {SemanticAction::focus,
        dropped_down_ ? SemanticAction::collapse : SemanticAction::expand};
    descriptor.exposed = true;
    return descriptor;
}

bool ComboBox::on_semantic_action(SemanticAction action, std::string_view value) {
    if (action == SemanticAction::expand) {
        set_dropped_down(true);
        return true;
    }
    if (action == SemanticAction::collapse) {
        set_dropped_down(false);
        return true;
    }
    return Panel::on_semantic_action(action, value);
}

void ComboBox::on_detached_from_window() noexcept {
    popup_selection_.disconnect();
    popup_activation_.disconnect();
    popup_dismissal_.disconnect();
    popup_revocation_.disconnect();
    popup_scope_ = 0U;
    popup_token_.disconnect();
    popup_list_.reset();
    popup_layer_.reset();
    dropped_down_ = false;
    focused_ = false;
}

NumericUpDown::NumericUpDown(StableId stable_id) : Panel(std::move(stable_id)) {
    set_border_style(BorderStyle::line);
    set_background(style().paper);
    define_bindable_property({
        {"Value", BindingValueKind::number, "Behavior",
         "Current numeric value.", BindingValue{0.0},
         Dirty::paint | Dirty::semantics},
        [this] { return BindingValue{value_}; },
        [this](const BindingValue& value) {
            const auto converted = convert_binding_value(
                value, BindingValueKind::number);
            if (!converted) {
                throw std::invalid_argument(
                    "NumericUpDown.Value binding requires a number");
            }
            set_value(std::get<double>(*converted));
        },
        [this](Component& owner, std::function<void()> changed) {
            return value_changed_.subscribe(owner,
                [changed = std::move(changed)](double) { changed(); });
        }, {}, {}});
}

void NumericUpDown::initialize_control_tree() {
    const std::string prefix(stable_id().value());
    editor_ = make_control<TextBox>(StableId(prefix + ".editor"));
    editor_->set_border_style(BorderStyle::none);
    spinner_ = make_control<SpinButtons>(StableId(prefix + ".spinner"));
    add_child(editor_);
    add_child(spinner_);
    const std::weak_ptr<NumericUpDown> weak =
        std::static_pointer_cast<NumericUpDown>(shared_from_this());
    editor_change_ = editor_->text_changed().subscribe(
        *this, [weak](const std::string&) {
            if (const auto numeric = weak.lock(); numeric && !numeric->synchronizing_) {
                numeric->commit_editor_text();
            }
        });
    auto spin = std::dynamic_pointer_cast<SpinButtons>(spinner_);
    spinner_step_ = spin->stepped().subscribe(*this, [weak](int direction) {
        if (const auto numeric = weak.lock()) numeric->step(direction);
    });
    synchronize_editor();
}

void NumericUpDown::set_range(double minimum, double maximum) {
    require_mutable();
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum > maximum) {
        throw std::invalid_argument("NumericUpDown range must be finite and ordered");
    }
    if (minimum_ == minimum && maximum_ == maximum) return;
    minimum_ = minimum;
    maximum_ = maximum;
    const double clamped = std::clamp(value_, minimum_, maximum_);
    if (clamped != value_) {
        value_ = clamped;
        synchronize_editor();
        publish_change(value_changed_, value_);
    }
    invalidate(Dirty::paint | Dirty::semantics);
}

void NumericUpDown::set_value(double value) {
    require_mutable();
    if (!std::isfinite(value) || value < minimum_ || value > maximum_) {
        throw std::out_of_range("NumericUpDown value is outside its range");
    }
    if (value_ == value) return;
    value_ = value;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
    publish_change(value_changed_, value_);
}

void NumericUpDown::set_increment(double increment) {
    require_mutable();
    if (!std::isfinite(increment) || increment <= 0.0) {
        throw std::invalid_argument("NumericUpDown increment must be finite and positive");
    }
    increment_ = increment;
}

void NumericUpDown::set_decimal_places(std::uint8_t places) {
    require_mutable();
    if (places > 12U) {
        throw std::invalid_argument("NumericUpDown decimal places may not exceed 12");
    }
    if (decimal_places_ == places) return;
    decimal_places_ = places;
    if (hexadecimal_ && places != 0U) hexadecimal_ = false;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
}

void NumericUpDown::set_hexadecimal(bool hexadecimal) {
    require_mutable();
    if (hexadecimal_ == hexadecimal) return;
    hexadecimal_ = hexadecimal;
    if (hexadecimal_) decimal_places_ = 0U;
    synchronize_editor();
    invalidate(Dirty::paint | Dirty::semantics);
}

std::string NumericUpDown::formatted_value() const {
    std::ostringstream stream;
    if (hexadecimal_) {
        stream << std::uppercase << std::hex << static_cast<std::int64_t>(std::llround(value_));
    } else {
        stream << std::fixed << std::setprecision(decimal_places_) << value_;
    }
    return stream.str();
}

void NumericUpDown::synchronize_editor() {
    if (!editor_) return;
    synchronizing_ = true;
    editor_->set_text(formatted_value());
    synchronizing_ = false;
}

void NumericUpDown::commit_editor_text() {
    if (!editor_ || editor_->text().empty()) return;
    try {
        std::size_t consumed{};
        double parsed{};
        if (hexadecimal_) {
            parsed = static_cast<double>(std::stoll(std::string(editor_->text()),
                                                    &consumed, 16));
        } else {
            parsed = std::stod(std::string(editor_->text()), &consumed);
        }
        if (consumed == editor_->text().size() && std::isfinite(parsed) &&
            parsed >= minimum_ && parsed <= maximum_ && parsed != value_) {
            value_ = parsed;
            invalidate(Dirty::paint | Dirty::semantics);
            publish_change(value_changed_, value_);
        }
    } catch (const std::exception&) {
    }
}

void NumericUpDown::step(int direction) {
    const double next = std::clamp(value_ + static_cast<double>(direction) * increment_,
                                   minimum_, maximum_);
    if (next != value_) set_value(next);
    if (editor_ && window() != nullptr) {
        static_cast<void>(window()->request_focus(editor_));
        editor_->select_all();
    }
}

void NumericUpDown::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double button_width = std::min(22.0, std::max(0.0, final_bounds.width));
    if (editor_) set_child_layout(
        editor_, {0.0, 0.0, std::max(0.0, final_bounds.width - button_width),
                  final_bounds.height});
    if (spinner_) set_child_layout(
        spinner_, {std::max(0.0, final_bounds.width - button_width), 0.0,
                   button_width, final_bounds.height});
}

void NumericUpDown::on_key_preview(KeyEvent& event) {
    if (!enabled() || event.action != KeyAction::down) return;
    if (event.physical_key == PhysicalKey::up || event.physical_key == PhysicalKey::down) {
        step(event.physical_key == PhysicalKey::up ? 1 : -1);
        event.handled = true;
    } else if (event.physical_key == PhysicalKey::enter) {
        commit_editor_text();
        synchronize_editor();
        event.handled = true;
    }
}

void NumericUpDown::on_pointer_preview(PointerEvent& event) {
    if (!enabled() || event.action != PointerAction::wheel || event.wheel_delta.y == 0.0) return;
    step(event.wheel_delta.y > 0.0 ? 1 : -1);
    event.handled = true;
}

SemanticDescriptor NumericUpDown::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::numeric_field;
    descriptor.name = accessible_name();
    descriptor.value = formatted_value();
    descriptor.numeric_value = value_;
    descriptor.minimum_value = minimum_;
    descriptor.maximum_value = maximum_;
    descriptor.description = accessible_description();
    descriptor.actions = {SemanticAction::focus, SemanticAction::increment,
                          SemanticAction::decrement, SemanticAction::set_value};
    descriptor.exposed = true;
    descriptor.include_descendants = false;
    return descriptor;
}

bool NumericUpDown::on_semantic_action(SemanticAction action,
                                       std::string_view value_text) {
    if (action == SemanticAction::increment || action == SemanticAction::decrement) {
        step(action == SemanticAction::increment ? 1 : -1);
        return true;
    }
    if (action == SemanticAction::set_value) {
        try {
            std::size_t consumed{};
            const double parsed = std::stod(std::string(value_text), &consumed);
            if (consumed != value_text.size()) return false;
            set_value(parsed);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    if (action == SemanticAction::focus && editor_ && window() != nullptr) {
        return window()->request_focus(editor_);
    }
    return Panel::on_semantic_action(action, value_text);
}

} // namespace gui_forms
