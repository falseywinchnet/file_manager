#include "gui_forms/controls/panel/color_value_editor/color_value_editor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <utility>

namespace gui_forms {

ColorValueEditor::ColorValueEditor(StableId stable_id, Color value)
    : Panel(std::move(stable_id)), value_(value) {
    set_paint_plane(PaintPlane::control);
    set_border_style(BorderStyle::none);
}

void ColorValueEditor::initialize_control_tree() {
    if (editor_) return;
    editor_ = make_control<TextBox>(
        StableId(std::string(stable_id().value()) + ".text"),
        format_value(value_));
    (*editor_).set_font({FontRole::monospace, 9.5, 400, false});
    add_child(editor_);
    const std::weak_ptr<ColorValueEditor> weak =
        std::static_pointer_cast<ColorValueEditor>(shared_from_this());
    committed_ = (*editor_).committed().subscribe(
        *this, [weak](const std::string& text) {
            if (const std::shared_ptr<gui_forms::ColorValueEditor> retained = weak.lock()) (*retained).commit(text);
        });
    cancelled_ = (*editor_).cancelled().subscribe(*this, [weak] {
        if (const std::shared_ptr<gui_forms::ColorValueEditor> retained = weak.lock()) (*retained).cancel();
    });
}

std::string ColorValueEditor::format_value(Color value) {
    constexpr char digits[] = "0123456789ABCDEF";
    std::string result{"#00000000"};
    const std::uint8_t channels[]{value.red, value.green, value.blue, value.alpha};
    for (std::size_t index = 0U; index < 4U; ++index) {
        result[1U + index * 2U] = digits[channels[index] >> 4U];
        result[2U + index * 2U] = digits[channels[index] & 0x0fU];
    }
    return result;
}

std::optional<Color> ColorValueEditor::parse_value(std::string_view text) {
    const std::string_view::size_type first =
        text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const std::string_view::size_type last =
        text.find_last_not_of(" \t\r\n");
    text = text.substr(first, last - first + 1U);
    if ((text.size() != 7U && text.size() != 9U) || text.front() != '#') {
        return {};
    }
    const auto nibble = [](char value) -> std::optional<std::uint8_t> {
        if (value >= '0' && value <= '9') {
            return static_cast<std::uint8_t>(value - '0');
        }
        if (value >= 'a' && value <= 'f') {
            return static_cast<std::uint8_t>(value - 'a' + 10);
        }
        if (value >= 'A' && value <= 'F') {
            return static_cast<std::uint8_t>(value - 'A' + 10);
        }
        return {};
    };
    std::uint8_t channels[4]{0U, 0U, 0U, 255U};
    const std::size_t count = text.size() == 9U ? 4U : 3U;
    for (std::size_t index = 0U; index < count; ++index) {
        const std::optional<std::uint8_t> high = nibble(text[1U + index * 2U]);
        const std::optional<std::uint8_t> low = nibble(text[2U + index * 2U]);
        if (!high || !low) return {};
        channels[index] = static_cast<std::uint8_t>((*high << 4U) | *low);
    }
    return Color::rgba(channels[0], channels[1], channels[2], channels[3]);
}

void ColorValueEditor::set_value(Color value) {
    require_mutable();
    if (value_ == value) {
        if (editor_ && (*editor_).text() != format_value(value)) {
            synchronizing_ = true;
            (*editor_).set_text(format_value(value));
            synchronizing_ = false;
        }
        if (editor_) (*editor_).set_visual_status(ControlVisualStatus::normal);
        set_visual_status(ControlVisualStatus::normal);
        invalidate(Dirty::paint | Dirty::semantics);
        return;
    }
    value_ = value;
    if (editor_) {
        synchronizing_ = true;
        (*editor_).set_text(format_value(value_));
        (*editor_).set_visual_status(ControlVisualStatus::normal);
        synchronizing_ = false;
    }
    set_visual_status(ControlVisualStatus::normal);
    invalidate(Dirty::paint | Dirty::semantics);
}

void ColorValueEditor::set_swatch_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 12.0 || width > 96.0) {
        throw std::invalid_argument(
            "color editor swatch width must be within [12, 96]");
    }
    if (swatch_width_ == width) return;
    swatch_width_ = width;
    invalidate(Dirty::layout | Dirty::paint | Dirty::hit_test);
}

void ColorValueEditor::commit(std::string_view text) {
    if (synchronizing_) return;
    const std::optional<Color> parsed = parse_value(text);
    if (!parsed) {
        set_visual_status(ControlVisualStatus::invalid);
        if (editor_) (*editor_).set_visual_status(ControlVisualStatus::invalid);
        const PropertyEditorInputError failure{
            std::string(text), "Color must be #RRGGBB or #RRGGBBAA"};
        edit_failed_.emit(failure);
        return;
    }
    const bool changed = *parsed != value_;
    value_ = *parsed;
    set_visual_status(ControlVisualStatus::normal);
    if (editor_) {
        synchronizing_ = true;
        (*editor_).set_text(format_value(value_));
        (*editor_).set_visual_status(ControlVisualStatus::normal);
        synchronizing_ = false;
    }
    invalidate(Dirty::paint | Dirty::semantics);
    if (changed) publish_change(value_changed_, value_);
}

void ColorValueEditor::cancel() {
    if (!editor_) return;
    synchronizing_ = true;
    (*editor_).set_text(format_value(value_));
    (*editor_).set_visual_status(ControlVisualStatus::normal);
    synchronizing_ = false;
    set_visual_status(ControlVisualStatus::normal);
    invalidate(Dirty::paint | Dirty::semantics);
}

Size ColorValueEditor::measure(Size available) {
    return {std::max(0.0, available.width), 28.0 * effective_text_scale()};
}

void ColorValueEditor::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double swatch = std::min(
        swatch_width_, std::max(0.0, final_bounds.width));
    if (editor_) {
        set_child_layout(editor_, {swatch, 0.0,
            std::max(0.0, final_bounds.width - swatch), final_bounds.height});
    }
}

void ColorValueEditor::on_paint(Painter& painter, Rect) {
    const Rect bounds = local_bounds();
    const double swatch_width = std::min(swatch_width_, bounds.width);
    const Rect swatch{2.0, 2.0, std::max(0.0, swatch_width - 5.0),
                      std::max(0.0, bounds.height - 4.0)};
    const double half_width = swatch.width * 0.5;
    const double half_height = swatch.height * 0.5;
    painter.fill_rect({swatch.x, swatch.y, half_width, half_height},
                      Color::rgba(232, 232, 232));
    painter.fill_rect({swatch.x + half_width, swatch.y, half_width, half_height},
                      Color::rgba(178, 178, 178));
    painter.fill_rect({swatch.x, swatch.y + half_height, half_width, half_height},
                      Color::rgba(178, 178, 178));
    painter.fill_rect({swatch.x + half_width, swatch.y + half_height,
                       half_width, half_height}, Color::rgba(232, 232, 232));
    painter.fill_rect(swatch, value_);
    painter.stroke_rect({swatch.x + 0.5, swatch.y + 0.5,
                         std::max(0.0, swatch.width - 1.0),
                         std::max(0.0, swatch.height - 1.0)},
                        style().dark_border, 1.0);
}

SemanticDescriptor ColorValueEditor::semantic_descriptor() const {
    SemanticDescriptor result;
    result.role = SemanticRole::group;
    result.name = accessible_name();
    result.value = format_value(value_);
    result.description = accessible_description();
    result.exposed = true;
    if (visual_status() == ControlVisualStatus::invalid) {
        result.states |= SemanticState::invalid;
    }
    return result;
}

void ColorValueEditor::on_dispose() noexcept {
    committed_.disconnect();
    cancelled_.disconnect();
    editor_.reset();
    Control::on_dispose();
}

} // namespace gui_forms
