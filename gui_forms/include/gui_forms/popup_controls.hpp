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

enum class PopupDismissReason : std::uint8_t {
    click_away,
    escape_key,
};

// Full-client retained overlay for interactive popups. The layer owns one
// ordinary retained content subtree, continuously re-resolves its anchored
// bounds on host resize, and reports (rather than privately acting on)
// click-away and Escape dismissal requests. Popup lifetime and focus scope stay
// with Window, allowing product composites to implement staged Escape behavior
// without duplicating overlay hit testing or edge avoidance.
class AnchoredPopupLayer final : public Panel {
public:
    AnchoredPopupLayer(StableId stable_id, Control::Ptr anchor,
                       AnchoredPopupPlacement placement = {});

    [[nodiscard]] Control::Ptr anchor() const noexcept { return anchor_.lock(); }
    void set_anchor(Control::Ptr anchor);
    [[nodiscard]] Control::Ptr content() const noexcept { return content_; }
    void set_content(Control::Ptr content);
    [[nodiscard]] AnchoredPopupPlacement placement() const noexcept {
        return placement_;
    }
    void set_placement(AnchoredPopupPlacement placement);
    [[nodiscard]] AnchoredPopupPlacementResult resolved_placement() const noexcept {
        return resolved_;
    }
    [[nodiscard]] bool dismiss_on_click_away() const noexcept {
        return dismiss_on_click_away_;
    }
    void set_dismiss_on_click_away(bool enabled);
    [[nodiscard]] bool dismiss_on_escape() const noexcept {
        return dismiss_on_escape_;
    }
    void set_dismiss_on_escape(bool enabled);
    [[nodiscard]] Event<PopupDismissReason>& dismiss_requested() noexcept {
        return dismiss_requested_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_pointer(PointerEvent& event) override;
    void on_key_preview(KeyEvent& event) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    void validate_anchor(const Control::Ptr& anchor) const;

    Control::WeakPtr anchor_;
    Control::Ptr content_;
    AnchoredPopupPlacement placement_;
    AnchoredPopupPlacementResult resolved_;
    bool dismiss_on_click_away_{true};
    bool dismiss_on_escape_{true};
    Event<PopupDismissReason> dismiss_requested_;
};

} // namespace gui_forms
