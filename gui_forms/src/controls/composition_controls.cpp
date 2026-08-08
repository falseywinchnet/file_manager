#include "gui_forms/composition_controls.hpp"

#include "gui_forms/text.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

bool valid_layout(const CardLayout& layout) noexcept {
    const double values[] = {layout.padding.left, layout.padding.top,
                             layout.padding.right, layout.padding.bottom,
                             layout.section_gap, layout.header_extent,
                             layout.footer_extent};
    return std::all_of(std::begin(values), std::end(values), [](double value) {
        return std::isfinite(value) && value >= 0.0 && value <= 4096.0;
    });
}

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

bool valid_review_disposition(ReviewDisposition disposition) noexcept {
    switch (disposition) {
    case ReviewDisposition::neutral:
    case ReviewDisposition::information:
    case ReviewDisposition::accepted:
    case ReviewDisposition::pending:
    case ReviewDisposition::warning:
    case ReviewDisposition::rejected:
        return true;
    }
    return false;
}

bool valid_review_record(const ReviewRecord& record) noexcept {
    return !record.key.empty() && record.key.size() <= 128U &&
           !record.title.empty() && record.title.size() <= 512U &&
           record.summary.size() <= 8192U && record.verdict.size() <= 1024U &&
           validate_utf8(record.key).valid() &&
           validate_utf8(record.title).valid() &&
           validate_utf8(record.summary).valid() &&
           validate_utf8(record.verdict).valid() &&
           valid_review_disposition(record.disposition);
}

CardLayout themed_card_layout(const Theme& theme) noexcept {
    const ThemeStructureTokens& structure = theme.structure();
    return {{structure.spacing.large, structure.spacing.medium,
             structure.spacing.large, structure.spacing.medium},
            structure.spacing.medium,
            structure.geometry.control_height,
            structure.geometry.control_height};
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

Card::Card(StableId stable_id) : Panel(std::move(stable_id)) {
    set_visual_role(ControlVisualRole::card);
}

Control::Ptr Card::replace_section(Control::Ptr& slot,
                                   Control::Ptr replacement) {
    require_mutable();
    if (replacement &&
        (replacement == header_ || replacement == body_ || replacement == footer_)) {
        if (replacement == slot) return {};
        throw std::logic_error("one control cannot occupy two card sections");
    }
    if (replacement && replacement->parent()) {
        throw std::logic_error("card section control already has a parent");
    }
    Control::Ptr previous = slot;
    if (replacement) add_child(replacement);
    if (slot) previous = remove_child(slot->runtime_id());
    slot = std::move(replacement);
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
    return previous;
}

Control::Ptr Card::set_header(Control::Ptr control) {
    return replace_section(header_, std::move(control));
}

Control::Ptr Card::set_body(Control::Ptr control) {
    return replace_section(body_, std::move(control));
}

Control::Ptr Card::set_footer(Control::Ptr control) {
    return replace_section(footer_, std::move(control));
}

CardLayout Card::effective_card_layout() const noexcept {
    return uses_theme_layout_ ? themed_card_layout(effective_theme()) : layout_;
}

void Card::set_card_layout(CardLayout layout) {
    require_mutable();
    if (!valid_layout(layout)) {
        throw std::invalid_argument("card layout values must be finite and bounded");
    }
    if (!uses_theme_layout_ && layout_ == layout) return;
    layout_ = layout;
    uses_theme_layout_ = false;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void Card::reset_card_layout_to_theme() {
    require_mutable();
    if (uses_theme_layout_) return;
    uses_theme_layout_ = true;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void Card::set_interactive(bool interactive) {
    require_mutable();
    if (interactive_ == interactive) return;
    interactive_ = interactive;
    set_focusable(interactive_);
    if (!interactive_) {
        hovered_ = false;
        pressed_ = false;
        focused_ = false;
        keyboard_key_ = 0U;
    }
    invalidate(Dirty::style | Dirty::paint | Dirty::hit_test | Dirty::semantics);
}

void Card::set_selected(bool selected) {
    require_mutable();
    if (selected_ == selected) return;
    selected_ = selected;
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
    publish_change(selected_changed_, selected_);
}

Size Card::measure(Size available) {
    const CardLayout layout = effective_card_layout();
    const double horizontal = layout.padding.left + layout.padding.right;
    const double vertical = layout.padding.top + layout.padding.bottom;
    const double inner_width = std::max(0.0, available.width - horizontal);
    double width{};
    double height = vertical;
    unsigned sections{};
    const Control::Ptr retained_header = header_;
    const Control::Ptr retained_body = body_;
    const Control::Ptr retained_footer = footer_;
    if (retained_header && is_current_layout_child(retained_header)) {
        const Size desired = retained_header->measure(
            {inner_width, layout.header_extent});
        if (!is_alive()) return {};
        if (header_ != retained_header ||
            !is_current_layout_child(retained_header)) {
            invalidate(Dirty::measure | Dirty::arrange);
        } else {
            width = std::max(width, desired.width);
            height += std::max(layout.header_extent, desired.height);
            ++sections;
        }
    }
    if (retained_body && is_current_layout_child(retained_body)) {
        const Size desired = retained_body->measure(
            {inner_width, available.height});
        if (!is_alive()) return {};
        if (body_ != retained_body || !is_current_layout_child(retained_body)) {
            invalidate(Dirty::measure | Dirty::arrange);
        } else {
            width = std::max(width, desired.width);
            height += desired.height;
            ++sections;
        }
    }
    if (retained_footer && is_current_layout_child(retained_footer)) {
        const Size desired = retained_footer->measure(
            {inner_width, layout.footer_extent});
        if (!is_alive()) return {};
        if (footer_ != retained_footer ||
            !is_current_layout_child(retained_footer)) {
            invalidate(Dirty::measure | Dirty::arrange);
        } else {
            width = std::max(width, desired.width);
            height += std::max(layout.footer_extent, desired.height);
            ++sections;
        }
    }
    if (sections > 1U) height += layout.section_gap * (sections - 1U);
    const Rect requested = requested_bounds();
    if (requested.width > 0.0) width = requested.width - horizontal;
    if (requested.height > 0.0) height = requested.height;
    return {std::min(available.width, std::max(0.0, width + horizontal)),
            std::min(available.height, std::max(0.0, height))};
}

void Card::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const CardLayout layout = effective_card_layout();
    const double left = layout.padding.left;
    const double width = std::max(
        0.0, final_bounds.width - layout.padding.left - layout.padding.right);
    double top = layout.padding.top;
    double bottom = std::max(top, final_bounds.height - layout.padding.bottom);
    if (header_) {
        const double extent = std::min(layout.header_extent,
                                       std::max(0.0, bottom - top));
        set_child_layout(header_, {left, top, width, extent});
        top += extent + layout.section_gap;
    }
    if (footer_) {
        const double extent = std::min(layout.footer_extent,
                                       std::max(0.0, bottom - top));
        bottom -= extent;
        set_child_layout(footer_, {left, bottom, width, extent});
        bottom -= layout.section_gap;
    }
    if (body_) {
        set_child_layout(body_, {left, top, width, std::max(0.0, bottom - top)});
    }
}

ControlVisualContext Card::current_context() const noexcept {
    return visual_context(hovered_, pressed_, selected_, focused_);
}

void Card::on_paint(Painter& painter, Rect local_damage) {
    if (has_background_override() || has_style_override()) {
        Panel::on_paint(painter, local_damage);
        return;
    }
    const Rect arranged = committed_arranged_bounds();
    const Rect bounds{0.0, 0.0, arranged.width, arranged.height};
    const ControlVisualContext context = current_context();
    const ControlVisualRecipe& recipe = effective_theme().resolve(
        ControlVisualRole::card, context);
    paint_surface_material(painter, bounds, recipe.material);
    if (context.focused && recipe.focus_width > 0.0 &&
        bounds.width > 8.0 && bounds.height > 8.0) {
        const double inset = 3.0;
        painter.stroke_rounded_rect(
            {inset, inset, bounds.width - inset * 2.0,
             bounds.height - inset * 2.0},
            std::max(0.0, recipe.material.corner_radius - inset),
            recipe.focus_ring, recipe.focus_width);
    }
}

Insets Card::visual_outsets() const noexcept {
    if (has_background_override() || has_style_override()) {
        return Panel::visual_outsets();
    }
    return surface_material_visual_outsets(
        effective_theme().resolve(ControlVisualRole::card,
                                  current_context()).material);
}

bool Card::hit_test_local(Point local_point) const {
    return interactive_ && Control::hit_test_local(local_point);
}

void Card::on_pointer(PointerEvent& event) {
    if (!interactive_) return;
    if (event.action == PointerAction::enter || event.action == PointerAction::leave) {
        const bool next = event.action == PointerAction::enter;
        if (hovered_ != next) {
            hovered_ = next;
            invalidate(Dirty::style | Dirty::paint);
        }
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::down) {
        pressed_ = true;
        invalidate(Dirty::style | Dirty::paint);
        event.handled = true;
    } else if (event.button == PointerButton::primary &&
               event.action == PointerAction::up) {
        pressed_ = false;
        invalidate(Dirty::style | Dirty::paint);
        event.handled = true;
    }
}

void Card::on_key(KeyEvent& event) {
    if (!interactive_) return;
    const bool activation = event.physical_key == PhysicalKey::space ||
                            event.physical_key == PhysicalKey::enter;
    if (!activation) return;
    if (event.action == KeyAction::down && !event.repeat && !pressed_) {
        keyboard_key_ = event.physical_key;
        pressed_ = true;
        invalidate(Dirty::style | Dirty::paint);
        event.handled = true;
    } else if (event.action == KeyAction::up && pressed_ &&
               keyboard_key_ == event.physical_key) {
        keyboard_key_ = 0U;
        pressed_ = false;
        invalidate(Dirty::style | Dirty::paint);
        event.handled = true;
        on_activate();
    }
}

void Card::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused_) {
        pressed_ = false;
        keyboard_key_ = 0U;
    }
    invalidate(Dirty::style | Dirty::paint | Dirty::semantics);
}

void Card::on_activate() {
    if (!interactive_) return;
    activated_.emit(*this);
}

SemanticDescriptor Card::semantic_descriptor() const {
    SemanticDescriptor descriptor = Panel::semantic_descriptor();
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = interactive_ || !descriptor.name.empty() ||
                         !descriptor.description.empty();
    if (interactive_) {
        descriptor.role = SemanticRole::list_item;
        descriptor.actions = {SemanticAction::focus, SemanticAction::press,
                              SemanticAction::select};
        if (selected_) descriptor.states |= SemanticState::selected;
        if (visual_status() == ControlVisualStatus::pending) {
            descriptor.states |= SemanticState::busy;
        } else if (visual_status() == ControlVisualStatus::invalid) {
            descriptor.states |= SemanticState::invalid;
        }
    }
    return descriptor;
}

bool Card::on_semantic_action(SemanticAction action, std::string_view value) {
    if (interactive_ && action == SemanticAction::press) {
        on_activate();
        return true;
    }
    if (interactive_ && action == SemanticAction::select) {
        set_selected(true);
        return true;
    }
    return Panel::on_semantic_action(action, value);
}

ReviewCard::ReviewCard(StableId stable_id)
    : Card(std::move(stable_id)),
      title_label_(make_control<Label>(
          StableId(std::string(this->stable_id().value()) + ".title"))),
      summary_label_(make_control<Label>(
          StableId(std::string(this->stable_id().value()) + ".summary"))),
      verdict_label_(make_control<Label>(
          StableId(std::string(this->stable_id().value()) + ".verdict"))) {
    title_label_->set_text_style_role(TextStyleRole::heading);
    title_label_->set_vertical_alignment(VerticalAlignment::center);
    summary_label_->set_text_style_role(TextStyleRole::body);
    summary_label_->set_text_wrapping(TextWrapping::word);
    summary_label_->set_line_spacing(1.25);
    summary_label_->set_vertical_alignment(VerticalAlignment::near);
    verdict_label_->set_text_style_role(TextStyleRole::caption);
    verdict_label_->set_vertical_alignment(VerticalAlignment::center);
}

void ReviewCard::initialize_control_tree() {
    if (initialized_) return;
    static_cast<void>(set_header(title_label_));
    static_cast<void>(set_body(summary_label_));
    static_cast<void>(set_footer(verdict_label_));
    initialized_ = true;
}

void ReviewCard::set_record(ReviewRecord record) {
    require_mutable();
    if (!valid_review_record(record)) {
        throw std::invalid_argument(
            "review record identity/text/disposition is invalid or unbounded");
    }
    if (record_ == record) return;
    initialize_control_tree();
    record_ = std::move(record);
    title_label_->set_text(record_.title);
    summary_label_->set_text(record_.summary);
    verdict_label_->set_text(record_.verdict);
    set_accessible_name(record_.title);
    set_accessible_description(record_.summary);
    set_visual_status(record_.disposition == ReviewDisposition::pending
        ? ControlVisualStatus::pending
        : record_.disposition == ReviewDisposition::rejected
            ? ControlVisualStatus::invalid
            : ControlVisualStatus::normal);
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::semantics);
    publish_change(record_changed_, record_);
}

Size ReviewCard::measure(Size available) {
    initialize_control_tree();
    return Card::measure(available);
}

void ReviewCard::arrange(Rect final_bounds) {
    initialize_control_tree();
    Card::arrange(final_bounds);
}

SemanticDescriptor ReviewCard::semantic_descriptor() const {
    SemanticDescriptor descriptor = Card::semantic_descriptor();
    descriptor.name = record_.title;
    descriptor.description = record_.summary;
    descriptor.value = record_.verdict;
    descriptor.exposed = !record_.key.empty();
    if (record_.disposition == ReviewDisposition::pending) {
        descriptor.states |= SemanticState::busy;
    } else if (record_.disposition == ReviewDisposition::rejected) {
        descriptor.states |= SemanticState::invalid;
    }
    return descriptor;
}

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
