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
    const Utf8ValidationResult replacement_validation =
        validate_utf8(replacement);
    if (read_only_ || start.value() > end.value() ||
        !store_.is_grapheme_boundary(start) ||
        !store_.is_grapheme_boundary(end) ||
        !replacement_validation.valid()) {
        return false;
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
            static_cast<void>((*window()).request_focus(shared_from_this()));
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
        select_all();
        return replace_selection(value);
    }
    return Panel::on_semantic_action(action, value);
}

void TextBox::on_detached_from_window() noexcept {
    caret_frame_.disconnect();
    focused_ = false;
    selecting_ = false;
}

} // namespace gui_forms
