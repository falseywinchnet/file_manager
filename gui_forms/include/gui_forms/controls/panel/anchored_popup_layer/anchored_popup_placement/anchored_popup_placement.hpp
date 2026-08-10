#pragma once

#include "gui_forms/basic_controls.hpp"

#include <cstdint>
#include <memory>

namespace gui_forms {

enum class PopupHorizontalAlignment : std::uint8_t {
    near,
    center,
    far,
};

enum class PopupVerticalPreference : std::uint8_t {
    below,
    above,
};

struct AnchoredPopupPlacement final {
    Size preferred_size{};
    PopupHorizontalAlignment horizontal_alignment{
        PopupHorizontalAlignment::near};
    PopupVerticalPreference vertical_preference{
        PopupVerticalPreference::below};
    double gap{};
    double viewport_margin{4.0};
    bool allow_vertical_flip{true};
};

struct AnchoredPopupPlacementResult final {
    Rect bounds{};
    bool placed_above{};
    bool width_clamped{};
    bool height_clamped{};
};

// Resolves an owner-relative popup into logical window-client coordinates.
// The result is deterministic, bounded to the client margin, and flips across
// the anchor only when the preferred side cannot contain the requested height.
[[nodiscard]] AnchoredPopupPlacementResult resolve_anchored_popup(
    Rect anchor_bounds, Size client_size,
    AnchoredPopupPlacement placement);

} // namespace gui_forms
