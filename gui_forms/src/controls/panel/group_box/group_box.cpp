#include "gui_forms/controls/panel/group_box/group_box.hpp"
#include "../../basic/basic_control_rendering.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {

GroupBox::GroupBox(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_border_style(BorderStyle::none);
}

void GroupBox::set_text(std::string text) {
    require_mutable();
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("GroupBox font specification is invalid");
    }
    if (font_ == font) {
        return;
    }
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::set_use_mnemonic(bool value) {
    require_mutable();
    if (use_mnemonic_ == value) return;
    use_mnemonic_ = value;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void GroupBox::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const FontSpec font = effective_font(font_);
    const std::string caption = use_mnemonic_
        ? parse_mnemonic_text(text_).display_text : text_;
    const double caption_height = std::max(16.0, font.size + 4.0);
    const double rule_y = std::max(10.5, caption_height * 0.66);
    painter.fill_rect(bounds, background());
    painter.stroke_rect({0.5, rule_y, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - rule_y - 0.5)},
                        style().border, 1.0);
    const double caption_width = std::min(
        std::max(0.0, bounds.width - 18.0), estimated_text_width(caption, font) + 12.0);
    painter.fill_rect({9.0, 1.0, caption_width, caption_height}, background());
    painter.draw_text_utf8({13.0, std::max(font.size, rule_y + font.size * 0.36)},
                           caption, font,
                           effectively_enabled() ? style().text
                                                 : style().disabled_text);
}

bool GroupBox::mnemonic_matches(char32_t character) const noexcept {
    return use_mnemonic_ && is_mnemonic(character, text_);
}

bool GroupBox::process_mnemonic_self(char32_t character) {
    if (!mnemonic_matches(character)) return false;
    static_cast<void>(focus_next_after_self());
    return true;
}

SemanticDescriptor GroupBox::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty()
        ? (use_mnemonic_ ? parse_mnemonic_text(text_).display_text : text_)
        : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

} // namespace gui_forms
