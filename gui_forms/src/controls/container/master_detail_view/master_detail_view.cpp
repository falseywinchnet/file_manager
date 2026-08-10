#include "gui_forms/controls/container/master_detail_view/master_detail_view.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

bool valid_layout(const MasterDetailLayout& layout) noexcept {
    const double values[] = {layout.master_extent, layout.splitter_width,
                             layout.splitter_hit_width, layout.master_minimum,
                             layout.detail_minimum, layout.compact_threshold};
    if (!std::all_of(std::begin(values), std::end(values), [](double value) {
            return std::isfinite(value) && value >= 0.0 && value <= 16384.0;
        })) {
        return false;
    }
    return layout.splitter_width > 0.0 &&
           layout.splitter_hit_width >= layout.splitter_width &&
           layout.master_extent >= layout.master_minimum;
}

MasterDetailLayout themed_master_detail_layout(const Theme& theme) noexcept {
    const ThemeGeometryTokens& geometry = theme.structure().geometry;
    return {Orientation::vertical,
            geometry.navigation_extent,
            geometry.splitter_width,
            geometry.splitter_hit_width,
            geometry.navigation_minimum,
            geometry.content_minimum,
            geometry.compact_breakpoint,
            true};
}

} // namespace

MasterDetailView::MasterDetailView(StableId stable_id)
    : ContainerControl(std::move(stable_id)),
      split_(make_control<SplitContainer>(
          StableId(std::string(this->stable_id().value()) + ".split"))) {
    configure_split();
}

void MasterDetailView::initialize_control_tree() {
    if (initialized_) return;
    add_child(split_);
    initialized_ = true;
}

Control::Ptr MasterDetailView::replace_role(
    Control::Ptr& slot, const std::shared_ptr<SplitterPanel>& panel,
    Control::Ptr replacement) {
    require_mutable();
    if (replacement && (replacement == master_ || replacement == detail_)) {
        if (replacement == slot) return {};
        throw std::logic_error(
            "one control cannot occupy both master/detail roles");
    }
    if (replacement &&
        (replacement.get() == this || replacement == split_ ||
         replacement == split_->first_panel() ||
         replacement == split_->second_panel() ||
         replacement == split_->splitter_control())) {
        throw std::logic_error(
            "master/detail role cannot own its composition infrastructure");
    }
    if (replacement && replacement->parent()) {
        throw std::logic_error("master/detail role control already has a parent");
    }
    Control::Ptr previous = slot;
    if (replacement) {
        replacement->set_dock(DockStyle::fill);
        panel->add_child(replacement);
    }
    if (slot) previous = panel->remove_child(slot->runtime_id());
    slot = std::move(replacement);
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
    return previous;
}

Control::Ptr MasterDetailView::set_master(Control::Ptr control) {
    return replace_role(master_, split_->first_panel(), std::move(control));
}

Control::Ptr MasterDetailView::set_detail(Control::Ptr control) {
    return replace_role(detail_, split_->second_panel(), std::move(control));
}

void MasterDetailView::configure_split() {
    const MasterDetailLayout layout = effective_master_detail_layout();
    split_->set_orientation(layout.orientation);
    split_->set_splitter_width(layout.splitter_width);
    split_->set_splitter_hit_width(layout.splitter_hit_width);
    split_->set_first_minimum(layout.master_minimum);
    split_->set_second_minimum(layout.detail_minimum);
    split_->set_splitter_fixed(!layout.resizable);
    split_->set_fixed_panel(SplitFixedPanel::first);
    split_->set_splitter_distance(layout.master_extent);
}

MasterDetailLayout MasterDetailView::effective_master_detail_layout() const noexcept {
    return uses_theme_layout_
        ? themed_master_detail_layout(effective_theme())
        : layout_;
}

void MasterDetailView::set_master_detail_layout(MasterDetailLayout layout) {
    require_mutable();
    if (!valid_layout(layout)) {
        throw std::invalid_argument(
            "master/detail layout values must be finite, ordered, and bounded");
    }
    if (!uses_theme_layout_ && layout_ == layout) return;
    layout_ = layout;
    uses_theme_layout_ = false;
    configure_split();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void MasterDetailView::reset_master_detail_layout_to_theme() {
    require_mutable();
    if (uses_theme_layout_) return;
    uses_theme_layout_ = true;
    configure_split();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void MasterDetailView::set_display_mode(MasterDetailDisplayMode mode) {
    require_mutable();
    switch (mode) {
    case MasterDetailDisplayMode::automatic:
    case MasterDetailDisplayMode::side_by_side:
    case MasterDetailDisplayMode::master_only:
    case MasterDetailDisplayMode::detail_only:
        break;
    default:
        throw std::invalid_argument("master/detail display mode is invalid");
    }
    if (display_mode_ == mode) return;
    display_mode_ = mode;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void MasterDetailView::set_compact_detail_visible(bool visible) {
    require_mutable();
    if (compact_detail_visible_ == visible) return;
    compact_detail_visible_ = visible;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void MasterDetailView::show_master() {
    set_compact_detail_visible(false);
}

void MasterDetailView::show_detail() {
    set_compact_detail_visible(true);
}

MasterDetailDisplayMode MasterDetailView::resolve_display_mode(
    Size available) const noexcept {
    if (display_mode_ != MasterDetailDisplayMode::automatic) {
        return display_mode_;
    }
    const MasterDetailLayout layout = effective_master_detail_layout();
    const double axis = layout.orientation == Orientation::vertical
        ? available.width : available.height;
    const double required = std::max(
        layout.compact_threshold,
        layout.master_minimum + layout.splitter_width +
            layout.detail_minimum);
    if (axis >= required) return MasterDetailDisplayMode::side_by_side;
    return compact_detail_visible_ ? MasterDetailDisplayMode::detail_only
                                   : MasterDetailDisplayMode::master_only;
}

void MasterDetailView::apply_display_mode(MasterDetailDisplayMode mode,
                                          bool automatic) {
    if (effective_display_mode_ == mode) return;
    const MasterDetailDisplayMode previous = effective_display_mode_;
    switch (mode) {
    case MasterDetailDisplayMode::side_by_side:
        if (split_->first_collapsed()) {
            split_->set_first_collapsed(false,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        if (split_->second_collapsed()) {
            split_->set_second_collapsed(false,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        break;
    case MasterDetailDisplayMode::master_only:
        if (split_->first_collapsed()) {
            split_->set_first_collapsed(false,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        if (!split_->second_collapsed()) {
            split_->set_second_collapsed(true,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        break;
    case MasterDetailDisplayMode::detail_only:
        if (split_->second_collapsed()) {
            split_->set_second_collapsed(false,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        if (!split_->first_collapsed()) {
            split_->set_first_collapsed(true,
                automatic ? SplitCollapseOrigin::automatic_accommodation
                          : SplitCollapseOrigin::programmatic);
        }
        break;
    case MasterDetailDisplayMode::automatic:
        return;
    }
    effective_display_mode_ = mode;
    publish_change(presentation_changed_,
                   MasterDetailPresentationChange{previous, mode, automatic});
}

Size MasterDetailView::measure(Size available) {
    initialize_control_tree();
    configure_split();
    const MasterDetailLayout layout = effective_master_detail_layout();
    const MasterDetailDisplayMode mode = resolve_display_mode(available);
    const Control::Ptr retained_master = master_;
    const Control::Ptr retained_detail = detail_;
    Size master_desired;
    Size detail_desired;
    if (retained_master && retained_master->is_alive() &&
        retained_master->parent() == split_->first_panel()) {
        master_desired = retained_master->measure(available);
        if (!is_alive()) return {};
        if (master_ != retained_master || !retained_master->is_alive() ||
            retained_master->parent() != split_->first_panel()) {
            master_desired = {};
            invalidate(Dirty::measure | Dirty::arrange);
        }
    }
    if (retained_detail && retained_detail->is_alive() &&
        retained_detail->parent() == split_->second_panel()) {
        detail_desired = retained_detail->measure(available);
        if (!is_alive()) return {};
        if (detail_ != retained_detail || !retained_detail->is_alive() ||
            retained_detail->parent() != split_->second_panel()) {
            detail_desired = {};
            invalidate(Dirty::measure | Dirty::arrange);
        }
    }
    if (mode == MasterDetailDisplayMode::master_only) return master_desired;
    if (mode == MasterDetailDisplayMode::detail_only) return detail_desired;
    if (layout.orientation == Orientation::vertical) {
        return {master_desired.width + layout.splitter_width +
                    detail_desired.width,
                std::max(master_desired.height, detail_desired.height)};
    }
    return {std::max(master_desired.width, detail_desired.width),
            master_desired.height + layout.splitter_width +
                detail_desired.height};
}

void MasterDetailView::arrange(Rect final_bounds) {
    initialize_control_tree();
    configure_split();
    arrange_self(final_bounds);
    apply_display_mode(resolve_display_mode(
                           {final_bounds.width, final_bounds.height}),
                       display_mode_ == MasterDetailDisplayMode::automatic);
    set_child_layout(split_,
                     {0.0, 0.0, final_bounds.width, final_bounds.height});
}

SemanticDescriptor MasterDetailView::semantic_descriptor() const {
    SemanticDescriptor descriptor = ContainerControl::semantic_descriptor();
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = effective_display_mode_ ==
            MasterDetailDisplayMode::side_by_side
        ? "side-by-side"
        : effective_display_mode_ == MasterDetailDisplayMode::master_only
            ? "master"
            : "detail";
    descriptor.exposed = !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

} // namespace gui_forms
