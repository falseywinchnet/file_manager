#include "gui_forms/popup_controls.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

[[nodiscard]] bool finite_rect(Rect value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
           std::isfinite(value.width) && std::isfinite(value.height);
}

void validate_placement(const AnchoredPopupPlacement& placement) {
    if (!std::isfinite(placement.preferred_size.width) ||
        !std::isfinite(placement.preferred_size.height) ||
        placement.preferred_size.width < 0.0 ||
        placement.preferred_size.height < 0.0 ||
        !std::isfinite(placement.gap) || placement.gap < 0.0 ||
        !std::isfinite(placement.viewport_margin) ||
        placement.viewport_margin < 0.0) {
        throw std::invalid_argument(
            "anchored popup placement requires finite nonnegative geometry");
    }
}

} // namespace

AnchoredPopupPlacementResult resolve_anchored_popup(
    Rect anchor_bounds, Size client_size, AnchoredPopupPlacement placement) {
    validate_placement(placement);
    if (!finite_rect(anchor_bounds) || anchor_bounds.width < 0.0 ||
        anchor_bounds.height < 0.0 || !std::isfinite(client_size.width) ||
        !std::isfinite(client_size.height) || client_size.width < 0.0 ||
        client_size.height < 0.0) {
        throw std::invalid_argument(
            "anchored popup resolution requires finite nonnegative bounds");
    }

    const double margin_x = std::min(placement.viewport_margin,
                                     client_size.width * 0.5);
    const double margin_y = std::min(placement.viewport_margin,
                                     client_size.height * 0.5);
    const double available_width = std::max(0.0,
        client_size.width - margin_x * 2.0);
    const double available_height = std::max(0.0,
        client_size.height - margin_y * 2.0);
    const double width = std::min(placement.preferred_size.width,
                                  available_width);
    const double height = std::min(placement.preferred_size.height,
                                   available_height);

    double x = anchor_bounds.x;
    if (placement.horizontal_alignment == PopupHorizontalAlignment::center) {
        x = anchor_bounds.x + (anchor_bounds.width - width) * 0.5;
    } else if (placement.horizontal_alignment == PopupHorizontalAlignment::far) {
        x = anchor_bounds.x + anchor_bounds.width - width;
    }
    x = std::clamp(x, margin_x,
                   std::max(margin_x, client_size.width - margin_x - width));

    const double below_y = anchor_bounds.y + anchor_bounds.height + placement.gap;
    const double above_y = anchor_bounds.y - placement.gap - height;
    const bool fits_below = below_y + height <= client_size.height - margin_y;
    const bool fits_above = above_y >= margin_y;
    bool above = placement.vertical_preference == PopupVerticalPreference::above;
    if (placement.allow_vertical_flip) {
        if (!above && !fits_below && fits_above) above = true;
        else if (above && !fits_above && fits_below) above = false;
    }
    double y = above ? above_y : below_y;
    y = std::clamp(y, margin_y,
                   std::max(margin_y, client_size.height - margin_y - height));

    return {{x, y, width, height}, above,
            width != placement.preferred_size.width,
            height != placement.preferred_size.height};
}

AnchoredPopupLayer::AnchoredPopupLayer(
    StableId stable_id, Control::Ptr anchor, AnchoredPopupPlacement placement)
    : Panel(std::move(stable_id)), anchor_(anchor), placement_(placement) {
    validate_placement(placement_);
    validate_anchor(anchor);
    set_paint_plane(PaintPlane::overlay);
    set_border_style(BorderStyle::none);
    set_background(Color::rgba(0, 0, 0, 0));
}

void AnchoredPopupLayer::validate_anchor(const Control::Ptr& anchor) const {
    if (!anchor || !anchor->is_alive()) {
        throw std::invalid_argument("anchored popup requires a live owner control");
    }
}

void AnchoredPopupLayer::set_anchor(Control::Ptr anchor) {
    require_mutable();
    validate_anchor(anchor);
    if (anchor_.lock() == anchor) return;
    anchor_ = std::move(anchor);
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
}

void AnchoredPopupLayer::set_content(Control::Ptr content) {
    require_mutable();
    if (content == content_) return;
    if (!content || !content->is_alive() || content->parent() ||
        content->attached()) {
        throw std::invalid_argument(
            "anchored popup replacement content must be live and detached");
    }
    if (content_) {
        static_cast<void>(remove_child(content_->runtime_id()));
    }
    content_ = std::move(content);
    add_child(content_);
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void AnchoredPopupLayer::set_placement(AnchoredPopupPlacement placement) {
    require_mutable();
    validate_placement(placement);
    if (placement_.preferred_size == placement.preferred_size &&
        placement_.horizontal_alignment == placement.horizontal_alignment &&
        placement_.vertical_preference == placement.vertical_preference &&
        placement_.gap == placement.gap &&
        placement_.viewport_margin == placement.viewport_margin &&
        placement_.allow_vertical_flip == placement.allow_vertical_flip) {
        return;
    }
    placement_ = placement;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void AnchoredPopupLayer::set_dismiss_on_click_away(bool enabled) {
    require_mutable();
    dismiss_on_click_away_ = enabled;
}

void AnchoredPopupLayer::set_dismiss_on_escape(bool enabled) {
    require_mutable();
    dismiss_on_escape_ = enabled;
}

Size AnchoredPopupLayer::measure(Size available) {
    if (content_) static_cast<void>(content_->measure(placement_.preferred_size));
    return available;
}

void AnchoredPopupLayer::arrange(Rect final_bounds) {
    Rect layer_bounds = final_bounds;
    if (Window* owner = window()) {
        const Size client = owner->client_size();
        layer_bounds = {0.0, 0.0, client.width, client.height};
    }
    arrange_self(layer_bounds);
    const Control::Ptr anchor = anchor_.lock();
    if (!content_ || !anchor || !anchor->attached() ||
        !anchor->effectively_visible()) {
        if (content_) set_child_layout(content_, {});
        resolved_ = {};
        return;
    }
    resolved_ = resolve_anchored_popup(
        anchor->absolute_bounds(), {layer_bounds.width, layer_bounds.height},
        placement_);
    set_child_layout(content_, resolved_.bounds);
}

void AnchoredPopupLayer::on_pointer(PointerEvent& event) {
    if (dismiss_on_click_away_ && event.action == PointerAction::down &&
        event.button == PointerButton::primary) {
        dismiss_requested_.emit(PopupDismissReason::click_away);
        event.handled = true;
    }
}

void AnchoredPopupLayer::on_key_preview(KeyEvent& event) {
    if (dismiss_on_escape_ && event.action == KeyAction::down &&
        event.physical_key == PhysicalKey::escape) {
        dismiss_requested_.emit(PopupDismissReason::escape_key);
        event.handled = true;
    }
}

SemanticDescriptor AnchoredPopupLayer::semantic_descriptor() const {
    SemanticDescriptor descriptor = Panel::semantic_descriptor();
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty();
    return descriptor;
}

} // namespace gui_forms
