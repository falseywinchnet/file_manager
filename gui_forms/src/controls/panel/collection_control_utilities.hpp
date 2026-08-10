#pragma once

#include "gui_forms/text.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>

namespace gui_forms::collection_detail {

inline constexpr std::chrono::milliseconds type_timeout{850};

inline std::uint64_t virtual_runtime_id(std::string_view id) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : id) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    return hash | (1ULL << 63U);
}

inline std::string fold_ascii(std::string_view text) {
    std::string result(text);
    for (char& character : result) {
        if (character >= 'A' && character <= 'Z') {
            character = static_cast<char>(character - 'A' + 'a');
        }
    }
    return result;
}

inline void validate_identity_text(std::string_view id,
                                   std::string_view text) {
    if (id.empty() || !validate_utf8(id).valid() ||
        !validate_utf8(text).valid()) {
        throw std::invalid_argument(
            "collection item identity and text must be valid UTF-8");
    }
}

inline Color with_alpha(Color color, std::uint8_t alpha) noexcept {
    color.alpha = alpha;
    return color;
}

inline Rect fit_image_rect(Rect bounds, Size source) noexcept {
    if (bounds.empty() || source.width <= 0.0 || source.height <= 0.0) return {};
    const double scale = std::min(bounds.width / source.width,
                                  bounds.height / source.height);
    const double width = source.width * scale;
    const double height = source.height * scale;
    return {bounds.x + (bounds.width - width) * 0.5,
            bounds.y + (bounds.height - height) * 0.5, width, height};
}

inline void draw_emphasized_text(Painter& painter, Point origin,
                                 std::string_view text,
                                 std::span<const std::string> terms,
                                 FontSpec font, Color foreground, Color mark) {
    if (terms.empty() || text.empty()) {
        painter.draw_text_utf8(origin, text, font, foreground);
        return;
    }
    const std::string folded = fold_ascii(text);
    std::size_t cursor{};
    double x = origin.x;
    while (cursor < text.size()) {
        std::size_t next = text.size();
        std::size_t length{};
        for (const std::string& term : terms) {
            if (term.empty()) continue;
            const std::string folded_term = fold_ascii(term);
            const std::size_t found = folded.find(folded_term, cursor);
            if (found < next || (found == next && folded_term.size() > length)) {
                next = found;
                length = folded_term.size();
            }
        }
        if (next == std::string::npos || next >= text.size()) {
            const std::string_view tail = text.substr(cursor);
            painter.draw_text_utf8({x, origin.y}, tail, font, foreground);
            break;
        }
        if (next > cursor) {
            const std::string_view prefix = text.substr(cursor, next - cursor);
            painter.draw_text_utf8({x, origin.y}, prefix, font, foreground);
            x += painter.measure_text_utf8(prefix, font).width;
        }
        const std::string_view emphasized = text.substr(next, length);
        const double width = painter.measure_text_utf8(emphasized, font).width;
        painter.fill_rect({x - 1.0, origin.y - font.size,
                           width + 2.0, font.size + 3.0}, mark);
        painter.draw_text_utf8({x, origin.y}, emphasized, font, foreground);
        x += width;
        cursor = next + length;
    }
}

} // namespace gui_forms::collection_detail
