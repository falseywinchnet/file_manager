#include "basic_control_rendering.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {

std::shared_ptr<const PropertyEnumDescriptor> picture_box_size_mode_enum() {
    static const std::shared_ptr<const gui_forms::PropertyEnumDescriptor> value = std::make_shared<const PropertyEnumDescriptor>(
        PropertyEnumDescriptor{
            "System.Windows.Forms.PictureBoxSizeMode",
            {{"Normal", 0}, {"StretchImage", 1}, {"AutoSize", 2},
             {"CenterImage", 3}, {"Zoom", 4}},
            false});
    return value;
}

BindingValue picture_box_size_mode_value(PictureBoxSizeMode mode) {
    PropertyDescriptor descriptor;
    descriptor.kind = BindingValueKind::enumeration;
    descriptor.enumeration = picture_box_size_mode_enum();
    const std::optional<BindingValue> normalized = convert_property_value(
        BindingValue{static_cast<std::int64_t>(mode)}, descriptor);
    if (!normalized) {
        throw std::logic_error(
            "GUI.Forms retained PictureBoxSizeMode is outside its property schema");
    }
    return *normalized;
}

double estimated_text_width(std::string_view text, FontSpec font) noexcept {
    std::size_t scalars{};
    for (const unsigned char byte : text) {
        if ((byte & 0xc0U) != 0x80U) {
            ++scalars;
        }
    }
    return static_cast<double>(scalars) * font.size * 0.56;
}

bool valid_content_alignment(ContentAlignment alignment) noexcept {
    switch (alignment) {
    case ContentAlignment::top_left:
    case ContentAlignment::top_center:
    case ContentAlignment::top_right:
    case ContentAlignment::middle_left:
    case ContentAlignment::middle_center:
    case ContentAlignment::middle_right:
    case ContentAlignment::bottom_left:
    case ContentAlignment::bottom_center:
    case ContentAlignment::bottom_right: return true;
    }
    return false;
}

bool valid_text_image_relation(TextImageRelation relation) noexcept {
    switch (relation) {
    case TextImageRelation::overlay:
    case TextImageRelation::image_before_text:
    case TextImageRelation::text_before_image:
    case TextImageRelation::image_above_text:
    case TextImageRelation::text_above_image: return true;
    }
    return false;
}

Rect aligned_rect(Rect bounds, Size size,
                  ContentAlignment alignment) noexcept {
    const double width = std::min(std::max(0.0, size.width), bounds.width);
    const double height = std::min(std::max(0.0, size.height), bounds.height);
    double x = bounds.x;
    double y = bounds.y;
    switch (alignment) {
    case ContentAlignment::top_center:
    case ContentAlignment::middle_center:
    case ContentAlignment::bottom_center:
        x += (bounds.width - width) * 0.5;
        break;
    case ContentAlignment::top_right:
    case ContentAlignment::middle_right:
    case ContentAlignment::bottom_right:
        x += bounds.width - width;
        break;
    default: break;
    }
    switch (alignment) {
    case ContentAlignment::middle_left:
    case ContentAlignment::middle_center:
    case ContentAlignment::middle_right:
        y += (bounds.height - height) * 0.5;
        break;
    case ContentAlignment::bottom_left:
    case ContentAlignment::bottom_center:
    case ContentAlignment::bottom_right:
        y += bounds.height - height;
        break;
    default: break;
    }
    return {x, y, width, height};
}

std::vector<std::string> label_lines(std::string_view text, FontSpec font,
                                     double width, TextWrapping wrapping) {
    std::vector<std::string> lines;
    std::size_t paragraph_start{};
    while (paragraph_start <= text.size()) {
        const std::size_t newline = text.find('\n', paragraph_start);
        const std::size_t paragraph_end = newline == std::string_view::npos
            ? text.size() : newline;
        const std::string_view paragraph =
            text.substr(paragraph_start, paragraph_end - paragraph_start);
        if (wrapping == TextWrapping::no_wrap || width <= 4.0 || paragraph.empty()) {
            lines.emplace_back(paragraph);
        } else {
            std::string line;
            std::size_t cursor{};
            while (cursor < paragraph.size()) {
                while (cursor < paragraph.size() &&
                       std::isspace(static_cast<unsigned char>(paragraph[cursor])) != 0) {
                    ++cursor;
                }
                if (cursor >= paragraph.size()) {
                    break;
                }
                std::size_t word_end = cursor;
                while (word_end < paragraph.size() &&
                       std::isspace(static_cast<unsigned char>(paragraph[word_end])) == 0) {
                    ++word_end;
                }
                const std::string_view word = paragraph.substr(cursor, word_end - cursor);
                std::string candidate = line;
                if (!candidate.empty()) {
                    candidate.push_back(' ');
                }
                candidate.append(word);
                if (!line.empty() && estimated_text_width(candidate, font) > width) {
                    lines.push_back(std::move(line));
                    line.assign(word);
                } else {
                    line = std::move(candidate);
                }
                cursor = word_end;
            }
            if (!line.empty()) {
                lines.push_back(std::move(line));
            } else if (paragraph.empty()) {
                lines.emplace_back();
            }
        }
        if (newline == std::string_view::npos) {
            break;
        }
        paragraph_start = newline + 1U;
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

void paint_relief(Painter& painter, Rect bounds, const BasicControlStyle& style,
                  bool pressed) {
    painter.fill_rect(bounds, style.face);
    if (!pressed && bounds.width > 3.0 && bounds.height > 3.0) {
        painter.fill_rect({bounds.x + 1.0, bounds.y + 1.0,
                           std::max(0.0, bounds.width - 2.0),
                           std::floor(std::max(0.0, bounds.height - 2.0) * 0.45)},
                          style.face_light);
    }
    const Color top = pressed ? style.dark_border : style.highlight;
    const Color bottom = pressed ? style.highlight : style.dark_border;
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x + bounds.width - 1.0, bounds.y}, top, 1.0);
    painter.draw_line({bounds.x, bounds.y},
                      {bounds.x, bounds.y + bounds.height - 1.0}, top, 1.0);
    painter.draw_line({bounds.x, bounds.y + bounds.height - 1.0},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, bottom, 1.0);
    painter.draw_line({bounds.x + bounds.width - 1.0, bounds.y},
                      {bounds.x + bounds.width - 1.0,
                       bounds.y + bounds.height - 1.0}, bottom, 1.0);
    if (bounds.width > 4.0 && bounds.height > 4.0) {
        painter.stroke_rect({bounds.x + 1.5, bounds.y + 1.5,
                             bounds.width - 3.0, bounds.height - 3.0},
                            pressed ? style.border : style.face_light, 1.0);
    }
}

void fill_radio_disc(Painter& painter, double left, double top, Color color,
                     bool inner) {
    // Painter deliberately exposes only renderer-neutral primitives today.
    // These one-pixel chords preserve a genuinely round 15 px WinForms radio
    // indicator without adding an ellipse primitive to every host backend.
    static constexpr int outer_left[] = {5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 5};
    static constexpr int outer_width[] = {5, 9, 11, 13, 15, 15, 15, 15, 15, 15, 15, 13, 11, 9, 5};
    static constexpr int inner_left[] = {2, 1, 0, 0, 0, 1, 2};
    static constexpr int inner_width[] = {3, 5, 7, 7, 7, 5, 3};
    if (inner) {
        for (int row = 0; row < 7; ++row) {
            painter.fill_rect({left + 4.0 + inner_left[row], top + 4.0 + row,
                               static_cast<double>(inner_width[row]), 1.0}, color);
        }
        return;
    }
    for (int row = 0; row < 15; ++row) {
        painter.fill_rect({left + outer_left[row], top + row,
                           static_cast<double>(outer_width[row]), 1.0}, color);
    }
}

void fill_radio_face(Painter& painter, double left, double top, Color color) {
    static constexpr int face_left[] = {5, 4, 3, 2, 1, 1, 1, 1, 1, 2, 3, 4, 5};
    static constexpr int face_width[] = {5, 7, 9, 11, 13, 13, 13, 13, 13, 11, 9, 7, 5};
    for (int row = 0; row < 13; ++row) {
        painter.fill_rect({left + face_left[row], top + 1.0 + row,
                           static_cast<double>(face_width[row]), 1.0}, color);
    }
}

void paint_focus(Painter& painter, Rect bounds, Color color) {
    if (bounds.width > 9.0 && bounds.height > 9.0) {
        const double left = bounds.x + 4.5;
        const double top = bounds.y + 4.5;
        const double right = bounds.x + bounds.width - 4.5;
        const double bottom = bounds.y + bounds.height - 4.5;
        for (double x = left; x < right; x += 2.0) {
            painter.draw_line({x, top}, {std::min(x + 1.0, right), top}, color, 1.0);
            painter.draw_line({x, bottom}, {std::min(x + 1.0, right), bottom}, color, 1.0);
        }
        for (double y = top + 1.0; y < bottom; y += 2.0) {
            painter.draw_line({left, y}, {left, std::min(y + 1.0, bottom)}, color, 1.0);
            painter.draw_line({right, y}, {right, std::min(y + 1.0, bottom)}, color, 1.0);
        }
    }
}

void paint_theme_cues(Painter& painter, Rect bounds,
                      const ControlVisualRecipe& recipe,
                      ControlVisualContext context) {
    if (context.defaulted && recipe.default_width > 0.0 &&
        bounds.width > recipe.default_width * 2.0 &&
        bounds.height > recipe.default_width * 2.0) {
        const double inset = recipe.default_width * 0.5;
        painter.stroke_rounded_rect(
            {bounds.x + inset, bounds.y + inset,
             bounds.width - inset * 2.0, bounds.height - inset * 2.0},
            std::max(0.0, recipe.material.corner_radius - inset),
            recipe.default_ring, recipe.default_width);
    }
    if (context.focused && recipe.focus_width > 0.0 &&
        bounds.width > 8.0 && bounds.height > 8.0) {
        if (recipe.focus_external) {
            const double outset = recipe.focus_offset + recipe.focus_width * 0.5;
            painter.stroke_rounded_rect(
                {bounds.x - outset, bounds.y - outset,
                 bounds.width + outset * 2.0,
                 bounds.height + outset * 2.0},
                recipe.material.corner_radius + outset,
                recipe.focus_ring, recipe.focus_width);
            return;
        }
        const double inset = std::max(3.0, recipe.default_width + 1.0);
        painter.stroke_rounded_rect(
            {bounds.x + inset, bounds.y + inset,
             bounds.width - inset * 2.0, bounds.height - inset * 2.0},
            std::max(0.0, recipe.material.corner_radius - inset),
            recipe.focus_ring, recipe.focus_width);
    }
}

} // namespace gui_forms
