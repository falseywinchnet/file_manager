#pragma once

#include "gui_forms/prepared_text.hpp"

#include <span>

namespace gui_forms::host::detail {

struct PreparedCompositeTarget final {
    std::span<std::uint32_t> pixels{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::size_t stride{};
};

struct PreparedCompositeClip final {
    Rect rect{};
    Rect rounded_rect{};
    double radius{};
    bool rounded{};
};

// Caller owns exclusive candidate pixels, disjoint from the mask, and flushes
// outstanding native DC writes first. No allocation or failure after validation.
// Baseline is absolute DIP position (translation already applied); its device
// origin rounds nearest with half ties away from zero. Glyphs are not rescaled.
[[nodiscard]] PreparedTextStatus composite_prepared_mask(const GrayTextMask& mask,
    const PreparedCompositeTarget target, const PreparedCompositeClip clip, const Point baseline,
    const double scale, const Color color) noexcept;

} // namespace gui_forms::host::detail
