#include "gui_forms/container_controls.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

namespace gui_forms {
namespace {

void require_finite_nonnegative(double value, const char* message) {
    if (!std::isfinite(value) || value < 0.0) {
        throw std::invalid_argument(message);
    }
}

constexpr std::size_t maximum_layout_tracks = 64U;

[[nodiscard]] double horizontal_extent(Insets insets) noexcept {
    return insets.left + insets.right;
}

[[nodiscard]] double vertical_extent(Insets insets) noexcept {
    return insets.top + insets.bottom;
}

[[nodiscard]] Size preferred_child_size(const Control::Ptr& child,
                                        Size available) {
    const Rect requested = child->requested_bounds();
    const Size measure_available{
        std::max(std::max(0.0, available.width), requested.width),
        std::max(std::max(0.0, available.height), requested.height)};
    Size result = child->measure(measure_available);
    if (!std::isfinite(result.width) || result.width < 0.0) result.width = 0.0;
    if (!std::isfinite(result.height) || result.height < 0.0) result.height = 0.0;
    return result;
}

struct TrackSpanDemand final {
    std::size_t start{};
    std::size_t span{1U};
    double required{};
};

struct TrackResolution final {
    std::vector<double> actual;
    double desired{};
};

[[nodiscard]] TrackResolution resolve_table_tracks(
    std::span<const TableLayoutStyle> styles,
    std::vector<double> minimum,
    std::span<const TrackSpanDemand> spans,
    double available) {
    for (std::size_t index = 0U; index < styles.size(); ++index) {
        if (styles[index].size_mode == TableSizeMode::absolute) {
            minimum[index] = styles[index].size;
        }
    }
    for (const TrackSpanDemand& demand : spans) {
        const std::size_t end = std::min(styles.size(), demand.start + demand.span);
        double current = 0.0;
        double total_weight = 0.0;
        for (std::size_t index = demand.start; index < end; ++index) {
            current += minimum[index];
            if (styles[index].size_mode != TableSizeMode::absolute) {
                total_weight += styles[index].size_mode == TableSizeMode::percent
                    ? styles[index].size : 1.0;
            }
        }
        const double deficit = std::max(0.0, demand.required - current);
        if (deficit <= 0.0 || total_weight <= 0.0) continue;
        for (std::size_t index = demand.start; index < end; ++index) {
            if (styles[index].size_mode == TableSizeMode::absolute) continue;
            const double weight = styles[index].size_mode == TableSizeMode::percent
                ? styles[index].size : 1.0;
            minimum[index] += deficit * weight / total_weight;
        }
    }

    double fixed_extent = 0.0;
    double percent_weight = 0.0;
    double desired_percent_extent = 0.0;
    for (std::size_t index = 0U; index < styles.size(); ++index) {
        if (styles[index].size_mode == TableSizeMode::percent) {
            percent_weight += styles[index].size;
        } else {
            fixed_extent += minimum[index];
        }
    }
    if (percent_weight > 0.0) {
        for (std::size_t index = 0U; index < styles.size(); ++index) {
            if (styles[index].size_mode != TableSizeMode::percent) continue;
            desired_percent_extent = std::max(
                desired_percent_extent,
                minimum[index] * percent_weight / styles[index].size);
        }
    }

    TrackResolution result;
    result.actual.resize(styles.size());
    const double remaining = std::max(0.0, available - fixed_extent);
    for (std::size_t index = 0U; index < styles.size(); ++index) {
        result.actual[index] = styles[index].size_mode == TableSizeMode::percent
            ? (percent_weight > 0.0
                   ? remaining * styles[index].size / percent_weight
                   : 0.0)
            : minimum[index];
    }
    result.desired = fixed_extent + desired_percent_extent;
    return result;
}

[[nodiscard]] std::uint64_t virtual_semantic_runtime_id(
    std::string_view stable_id) noexcept {
    std::uint64_t value = 1469598103934665603ULL;
    for (const unsigned char byte : stable_id) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    return value | (std::uint64_t{1} << 63U);
}

class SplitterGrip final : public Control {
public:
    explicit SplitterGrip(StableId stable_id)
        : Control(std::move(stable_id)) {
        set_focusable(true);
    }

    void set_orientation(Orientation orientation) {
        if (orientation_ == orientation) return;
        orientation_ = orientation;
        set_cursor(orientation == Orientation::vertical
                       ? CursorKind::resize_horizontal
                       : CursorKind::resize_vertical);
        invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    }

    void set_visible_width(double width) {
        if (visible_width_ == width) return;
        visible_width_ = width;
        invalidate(Dirty::paint);
    }

    void set_collapse_appearance(SplitFixedPanel panel, bool collapsed) {
        if (collapse_panel_ == panel && collapsed_ == collapsed) return;
        collapse_panel_ = panel;
        collapsed_ = collapsed;
        invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
    }

    void on_focus_changed(bool focused) override {
        focused_ = focused;
        invalidate(invalidation::focus);
    }

    void on_paint(Painter& painter, Rect) override {
        const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                          committed_arranged_bounds().height};
        const BasicControlStyle style;
        if (orientation_ == Orientation::vertical) {
            const double width = std::min(visible_width_, bounds.width);
            const double x = std::floor((bounds.width - width) * 0.5);
            painter.fill_rect({x, 0.0, width, bounds.height}, style.face);
            painter.draw_line({x, 0.0}, {x, bounds.height}, style.highlight, 1.0);
            painter.draw_line({x + std::max(0.0, width - 1.0), 0.0},
                              {x + std::max(0.0, width - 1.0), bounds.height},
                              style.dark_border, 1.0);
        } else {
            const double height = std::min(visible_width_, bounds.height);
            const double y = std::floor((bounds.height - height) * 0.5);
            painter.fill_rect({0.0, y, bounds.width, height}, style.face);
            painter.draw_line({0.0, y}, {bounds.width, y}, style.highlight, 1.0);
            painter.draw_line({0.0, y + std::max(0.0, height - 1.0)},
                              {bounds.width, y + std::max(0.0, height - 1.0)},
                              style.dark_border, 1.0);
        }
        if (focused_) {
            painter.stroke_rect({1.0, 1.0, std::max(0.0, bounds.width - 2.0),
                                 std::max(0.0, bounds.height - 2.0)},
                                style.accent, 1.0);
        }
        if (collapse_panel_ != SplitFixedPanel::none) {
            const bool first = collapse_panel_ == SplitFixedPanel::first;
            std::string arrow;
            Rect tab;
            Point text_origin;
            if (orientation_ == Orientation::vertical) {
                arrow = first ? (collapsed_ ? "▶" : "◀")
                              : (collapsed_ ? "◀" : "▶");
                tab = {0.0, std::max(0.0, (bounds.height - 34.0) * 0.5),
                       bounds.width, std::min(34.0, bounds.height)};
                text_origin = {std::max(0.0, (bounds.width - 7.0) * 0.5),
                               tab.y + tab.height * 0.5 + 4.0};
            } else {
                arrow = first ? (collapsed_ ? "▼" : "▲")
                              : (collapsed_ ? "▲" : "▼");
                tab = {std::max(0.0, (bounds.width - 34.0) * 0.5), 0.0,
                       std::min(34.0, bounds.width), bounds.height};
                text_origin = {tab.x + tab.width * 0.5 - 4.0,
                               std::max(7.0, bounds.height * 0.5 + 4.0)};
            }
            painter.fill_rect(tab, focused_ ? style.accent_light : style.face_light);
            painter.stroke_rect({tab.x + 0.5, tab.y + 0.5,
                                 std::max(0.0, tab.width - 1.0),
                                 std::max(0.0, tab.height - 1.0)},
                                focused_ ? style.accent : style.border, 1.0);
            painter.draw_text_utf8(text_origin, arrow,
                                   effective_font(
                                       {FontRole::control, 8.0, 700, false}),
                                   style.text);
        }
    }

private:
    Orientation orientation_{Orientation::vertical};
    SplitFixedPanel collapse_panel_{SplitFixedPanel::none};
    double visible_width_{3.0};
    bool collapsed_{};
    bool focused_{};
};

} // namespace

ContainerControl::ContainerControl(StableId stable_id)
    : ScrollableControl(std::move(stable_id)) {}

bool ContainerControl::contains_descendant(const Control::Ptr& control) const noexcept {
    if (!control || control.get() == this) {
        return false;
    }
    for (Control::Ptr ancestor = control->parent(); ancestor;
         ancestor = ancestor->parent()) {
        if (ancestor.get() == this) {
            return true;
        }
    }
    return false;
}

Control::Ptr ContainerControl::active_control() const noexcept {
    const Window* owner = window();
    if (owner == nullptr) {
        return {};
    }
    Control::Ptr focused = owner->focused_control();
    return contains_descendant(focused) ? focused : Control::Ptr{};
}

bool ContainerControl::request_active_control(const Control::Ptr& control) {
    require_mutable();
    if (!contains_descendant(control) || window() == nullptr) {
        return false;
    }
    return window()->request_focus(control);
}

bool ContainerControl::clear_active_control() {
    require_mutable();
    if (window() == nullptr || !active_control()) {
        return false;
    }
    return window()->request_focus({});
}

AutoValidate ContainerControl::effective_auto_validate() const noexcept {
    for (const Control* current = this; current != nullptr;) {
        if (const auto* container = dynamic_cast<const ContainerControl*>(current);
            container != nullptr && container->auto_validate_ != AutoValidate::inherit) {
            return container->auto_validate_;
        }
        const Control::Ptr owner = current->parent();
        current = owner.get();
    }
    return AutoValidate::enable_prevent_focus_change;
}

void ContainerControl::set_auto_validate(AutoValidate value) {
    require_mutable();
    if (value != AutoValidate::inherit && value != AutoValidate::disable &&
        value != AutoValidate::enable_prevent_focus_change &&
        value != AutoValidate::enable_allow_focus_change) {
        throw std::invalid_argument("GUI.Forms AutoValidate value is invalid");
    }
    if (auto_validate_ == value) return;
    auto_validate_ = value;
    auto_validate_changed_.emit(value);
}

bool ContainerControl::validate(bool check_auto_validate) {
    require_mutable();
    if (window() == nullptr) return true;
    if (check_auto_validate &&
        effective_auto_validate() == AutoValidate::disable) return true;
    Control::Ptr current = active_control();
    bool accepted = true;
    while (current && current.get() != this) {
        accepted = window()->validate_control(current, this, false) && accepted;
        current = current->parent();
    }
    return accepted;
}

bool ContainerControl::validate_children(ValidationConstraints constraints) {
    require_mutable();
    return window() == nullptr
        ? true : window()->validate_children(shared_from_this(), constraints);
}

SemanticDescriptor ContainerControl::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() ||
                         !descriptor.description.empty();
    return descriptor;
}

UserControl::UserControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void UserControl::on_attached_to_window() {
    attached_ = true;
    if (!loaded_) {
        loaded_ = true;
        loaded_event_.emit();
    }
}

void UserControl::on_attachment_committed() noexcept {
    ++attachment_count_;
}

void UserControl::on_detached_from_window() noexcept {
    attached_ = false;
}

namespace {

void validate_scaled_design_size(Size size) {
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width <= 0.0 || size.height <= 0.0) {
        throw std::invalid_argument(
            "scaled layout design size must be finite and positive");
    }
}

void validate_scaled_design_bounds(Rect bounds) {
    if (!std::isfinite(bounds.x) || !std::isfinite(bounds.y) ||
        !std::isfinite(bounds.width) || !std::isfinite(bounds.height) ||
        bounds.width < 0.0 || bounds.height < 0.0) {
        throw std::invalid_argument(
            "scaled layout bounds must be finite and nonnegative");
    }
}

template <typename Parent>
void reconcile_scaled_slots(
    Parent& parent, std::unordered_map<std::uint64_t, Rect>& slots) {
    std::unordered_set<std::uint64_t> retained;
    retained.reserve(parent.children().size());
    for (const Control::Ptr& child : parent.children()) {
        retained.insert(child->runtime_id().value);
    }
    std::erase_if(slots, [&](const auto& entry) {
        return !retained.contains(entry.first);
    });
}

[[nodiscard]] Rect scaled_child_bounds(Size design_size, Rect bounds,
                                       Rect final_bounds) {
    const double scale_x = final_bounds.width / design_size.width;
    const double scale_y = final_bounds.height / design_size.height;
    return {bounds.x * scale_x, bounds.y * scale_y,
            bounds.width * scale_x, bounds.height * scale_y};
}

} // namespace

ScaledPanel::ScaledPanel(StableId stable_id, Size design_size)
    : Panel(std::move(stable_id)), design_size_(design_size) {
    validate_scaled_design_size(design_size_);
}

void ScaledPanel::set_design_size(Size size) {
    require_mutable();
    validate_scaled_design_size(size);
    if (design_size_ == size) return;
    design_size_ = size;
    invalidate(Dirty::layout);
}

void ScaledPanel::add_at(Control::Ptr child, Rect design_bounds) {
    require_mutable();
    if (!child) throw std::invalid_argument("scaled layout child may not be null");
    validate_scaled_design_bounds(design_bounds);
    const RuntimeId id = child->runtime_id();
    add_child(std::move(child));
    slots_[id.value] = design_bounds;
    invalidate(Dirty::layout);
}

void ScaledPanel::set_design_bounds(const Control& child, Rect design_bounds) {
    require_mutable();
    validate_scaled_design_bounds(design_bounds);
    if (child.parent().get() != this) {
        throw std::logic_error("scaled layout child must belong to the panel");
    }
    slots_[child.runtime_id().value] = design_bounds;
    invalidate(Dirty::layout);
}

std::optional<Rect> ScaledPanel::design_bounds(const Control& child) const {
    const auto found = slots_.find(child.runtime_id().value);
    return found == slots_.end() ? std::nullopt
                                : std::optional<Rect>(found->second);
}

void ScaledPanel::reconcile_slots() {
    reconcile_scaled_slots(*this, slots_);
}

void ScaledPanel::arrange(Rect final_bounds) {
    reconcile_slots();
    arrange_self(final_bounds);
    for (const Control::Ptr& child : children()) {
        const auto slot = slots_.find(child->runtime_id().value);
        if (slot == slots_.end()) continue;
        set_child_layout(child,
                         scaled_child_bounds(design_size_, slot->second,
                                             final_bounds));
    }
}

ScaledGroupBox::ScaledGroupBox(StableId stable_id, std::string text,
                               Size design_size)
    : GroupBox(std::move(stable_id), std::move(text)),
      design_size_(design_size) {
    validate_scaled_design_size(design_size_);
}

void ScaledGroupBox::set_design_size(Size size) {
    require_mutable();
    validate_scaled_design_size(size);
    if (design_size_ == size) return;
    design_size_ = size;
    invalidate(Dirty::layout);
}

void ScaledGroupBox::add_at(Control::Ptr child, Rect design_bounds) {
    require_mutable();
    if (!child) throw std::invalid_argument("scaled layout child may not be null");
    validate_scaled_design_bounds(design_bounds);
    const RuntimeId id = child->runtime_id();
    add_child(std::move(child));
    slots_[id.value] = design_bounds;
    invalidate(Dirty::layout);
}

void ScaledGroupBox::set_design_bounds(const Control& child, Rect design_bounds) {
    require_mutable();
    validate_scaled_design_bounds(design_bounds);
    if (child.parent().get() != this) {
        throw std::logic_error("scaled layout child must belong to the group");
    }
    slots_[child.runtime_id().value] = design_bounds;
    invalidate(Dirty::layout);
}

std::optional<Rect> ScaledGroupBox::design_bounds(const Control& child) const {
    const auto found = slots_.find(child.runtime_id().value);
    return found == slots_.end() ? std::nullopt
                                : std::optional<Rect>(found->second);
}

void ScaledGroupBox::reconcile_slots() {
    reconcile_scaled_slots(*this, slots_);
}

void ScaledGroupBox::arrange(Rect final_bounds) {
    reconcile_slots();
    arrange_self(final_bounds);
    for (const Control::Ptr& child : children()) {
        const auto slot = slots_.find(child->runtime_id().value);
        if (slot == slots_.end()) continue;
        set_child_layout(child,
                         scaled_child_bounds(design_size_, slot->second,
                                             final_bounds));
    }
}

FlowLayoutPanel::FlowLayoutPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void FlowLayoutPanel::set_flow_direction(FlowDirection direction) {
    require_mutable();
    if (flow_direction_ == direction) return;
    flow_direction_ = direction;
    invalidate(invalidation::bounds);
}

void FlowLayoutPanel::set_wrap_contents(bool wrap) {
    require_mutable();
    if (wrap_contents_ == wrap) return;
    wrap_contents_ = wrap;
    invalidate(invalidation::bounds);
}

void FlowLayoutPanel::set_auto_size(bool auto_size) {
    Control::set_auto_size(auto_size);
}

void FlowLayoutPanel::set_flow_break(const Control& child, bool flow_break) {
    require_mutable();
    if (child.parent().get() != this || !child.is_alive()) {
        throw std::invalid_argument(
            "FlowLayoutPanel flow break requires a live direct child");
    }
    const std::uint64_t id = child.runtime_id().value;
    const bool previous = flow_breaks_.contains(id) && flow_breaks_.at(id);
    if (previous == flow_break) return;
    if (flow_break) flow_breaks_[id] = true;
    else flow_breaks_.erase(id);
    invalidate(invalidation::bounds);
}

bool FlowLayoutPanel::flow_break(const Control& child) const {
    if (child.parent().get() != this || !child.is_alive()) return false;
    const auto found = flow_breaks_.find(child.runtime_id().value);
    return found != flow_breaks_.end() && found->second;
}

void FlowLayoutPanel::reconcile_flow_breaks() {
    std::unordered_set<std::uint64_t> live;
    for (const Control::Ptr& child : children()) {
        if (child && child->is_alive() && child->parent().get() == this) {
            live.insert(child->runtime_id().value);
        }
    }
    std::erase_if(flow_breaks_, [&live](const auto& entry) {
        return !live.contains(entry.first);
    });
}

Size FlowLayoutPanel::layout_children(Size available, bool assign) {
    reconcile_flow_breaks();
    const Insets inset = padding();
    const Size inner{std::max(0.0, available.width - horizontal_extent(inset)),
                     std::max(0.0, available.height - vertical_extent(inset))};
    const bool horizontal = flow_direction_ == FlowDirection::left_to_right ||
                            flow_direction_ == FlowDirection::right_to_left;
    const double main_limit = horizontal ? inner.width : inner.height;

    struct Item final {
        Control::Ptr control;
        Size desired;
        Insets margin;
        bool break_after{};
    };
    struct Line final {
        std::vector<Item> items;
        double main{};
        double cross{};
    };

    std::vector<Line> lines;
    Line line;
    const auto flush_line = [&lines, &line] {
        if (line.items.empty()) return;
        lines.push_back(std::move(line));
        line = {};
    };
    const std::vector<Control::Ptr> retained = snapshot_layout_children();
    for (const Control::Ptr& child : retained) {
        if (!is_current_layout_child(child) || !child->visible()) continue;
        const Size desired = preferred_child_size(child, inner);
        if (!is_alive()) return {};
        if (!is_current_layout_child(child) || !child->visible()) continue;
        Item item{child, desired, child->margin(), flow_break(*child)};
        const double item_main = horizontal
            ? horizontal_extent(item.margin) + item.desired.width
            : vertical_extent(item.margin) + item.desired.height;
        const double item_cross = horizontal
            ? vertical_extent(item.margin) + item.desired.height
            : horizontal_extent(item.margin) + item.desired.width;
        if (!line.items.empty() && wrap_contents_ &&
            line.main + item_main > main_limit) {
            flush_line();
        }
        line.main += item_main;
        line.cross = std::max(line.cross, item_cross);
        line.items.push_back(std::move(item));
        if (line.items.back().break_after) flush_line();
    }
    flush_line();

    double cross_origin = 0.0;
    double content_main = 0.0;
    for (const Line& current : lines) {
        double main_origin = 0.0;
        for (const Item& item : current.items) {
            Rect slot;
            if (horizontal) {
                slot.width = item.desired.width;
                slot.height = item.desired.height;
                slot.y = inset.top + cross_origin + item.margin.top;
                if (flow_direction_ == FlowDirection::left_to_right) {
                    slot.x = inset.left + main_origin + item.margin.left;
                } else {
                    slot.x = inset.left + inner.width - main_origin -
                             item.margin.right - item.desired.width;
                }
                main_origin += horizontal_extent(item.margin) + item.desired.width;
            } else {
                slot.width = item.desired.width;
                slot.height = item.desired.height;
                slot.x = inset.left + cross_origin + item.margin.left;
                if (flow_direction_ == FlowDirection::top_down) {
                    slot.y = inset.top + main_origin + item.margin.top;
                } else {
                    slot.y = inset.top + inner.height - main_origin -
                             item.margin.bottom - item.desired.height;
                }
                main_origin += vertical_extent(item.margin) + item.desired.height;
            }
            if (assign && is_current_layout_child(item.control)) {
                set_child_layout(item.control, slot);
            }
        }
        content_main = std::max(content_main, current.main);
        cross_origin += current.cross;
    }
    const Size content = horizontal ? Size{content_main, cross_origin}
                                    : Size{cross_origin, content_main};
    return {content.width + horizontal_extent(inset),
            content.height + vertical_extent(inset)};
}

Size FlowLayoutPanel::measure(Size available) {
    available = {std::max(0.0, available.width),
                 std::max(0.0, available.height)};
    if (auto_size()) {
        const Size desired = layout_children(available, false);
        return {std::min(available.width, desired.width),
                std::min(available.height, desired.height)};
    }
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void FlowLayoutPanel::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    static_cast<void>(layout_children({final_bounds.width, final_bounds.height}, true));
}

SemanticDescriptor FlowLayoutPanel::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() || !descriptor.description.empty();
    return descriptor;
}

TableLayoutPanel::TableLayoutPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void TableLayoutPanel::validate_style(TableLayoutStyle style) {
    if (!std::isfinite(style.size) || style.size < 0.0 ||
        (style.size_mode == TableSizeMode::percent && style.size <= 0.0)) {
        throw std::invalid_argument(
            "TableLayoutPanel style size must be finite and nonnegative; percent must be positive");
    }
}

void TableLayoutPanel::set_column_count(std::size_t count) {
    require_mutable();
    if (count == 0U || count > maximum_layout_tracks) {
        throw std::out_of_range("TableLayoutPanel column count must be 1 through 64");
    }
    if (column_count_ == count) return;
    column_count_ = count;
    column_styles_.resize(count);
    invalidate(invalidation::bounds);
}

void TableLayoutPanel::set_row_count(std::size_t count) {
    require_mutable();
    if (count == 0U || count > maximum_layout_tracks) {
        throw std::out_of_range("TableLayoutPanel row count must be 1 through 64");
    }
    if (row_count_ == count) return;
    row_count_ = count;
    row_styles_.resize(count);
    invalidate(invalidation::bounds);
}

void TableLayoutPanel::set_grow_style(TableLayoutGrowStyle style) {
    require_mutable();
    if (grow_style_ == style) return;
    grow_style_ = style;
    invalidate(invalidation::bounds);
}

void TableLayoutPanel::set_auto_size(bool auto_size) {
    Control::set_auto_size(auto_size);
}

void TableLayoutPanel::set_cell_border_style(TableCellBorderStyle style) {
    require_mutable();
    if (cell_border_style_ == style) return;
    cell_border_style_ = style;
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

void TableLayoutPanel::set_column_style(std::size_t column,
                                        TableLayoutStyle style) {
    require_mutable();
    if (column >= column_count_) {
        throw std::out_of_range("TableLayoutPanel column style index");
    }
    validate_style(style);
    if (column_styles_[column] == style) return;
    column_styles_[column] = style;
    invalidate(invalidation::bounds);
}

void TableLayoutPanel::set_row_style(std::size_t row, TableLayoutStyle style) {
    require_mutable();
    if (row >= row_count_) throw std::out_of_range("TableLayoutPanel row style index");
    validate_style(style);
    if (row_styles_[row] == style) return;
    row_styles_[row] = style;
    invalidate(invalidation::bounds);
}

TableLayoutPanel::CellMetadata& TableLayoutPanel::metadata_for(
    const Control& child) {
    if (child.parent().get() != this || !child.is_alive()) {
        throw std::invalid_argument(
            "TableLayoutPanel metadata requires a live direct child");
    }
    return metadata_[child.runtime_id().value];
}

const TableLayoutPanel::CellMetadata* TableLayoutPanel::metadata_for(
    const Control& child) const {
    if (child.parent().get() != this || !child.is_alive()) return nullptr;
    const auto found = metadata_.find(child.runtime_id().value);
    return found == metadata_.end() ? nullptr : &found->second;
}

void TableLayoutPanel::set_cell_position(
    const Control& child, TableLayoutCellPosition position) {
    require_mutable();
    if (position.column >= maximum_layout_tracks ||
        position.row >= maximum_layout_tracks) {
        throw std::out_of_range("TableLayoutPanel cell position exceeds 64 tracks");
    }
    CellMetadata& metadata = metadata_for(child);
    if (metadata.position == position) return;
    metadata.position = position;
    invalidate(invalidation::bounds);
}

void TableLayoutPanel::clear_cell_position(const Control& child) {
    require_mutable();
    CellMetadata& metadata = metadata_for(child);
    if (!metadata.position) return;
    metadata.position.reset();
    invalidate(invalidation::bounds);
}

std::optional<TableLayoutCellPosition> TableLayoutPanel::cell_position(
    const Control& child) const {
    if (child.parent().get() != this || !child.is_alive()) return std::nullopt;
    if (attached_window() != nullptr) static_cast<void>(arranged_bounds());
    const auto resolved = resolved_cells_.find(child.runtime_id().value);
    if (resolved != resolved_cells_.end()) return resolved->second;
    const CellMetadata* metadata = metadata_for(child);
    return metadata == nullptr ? std::nullopt : metadata->position;
}

void TableLayoutPanel::set_column_span(const Control& child, std::size_t span) {
    require_mutable();
    if (span == 0U || span > maximum_layout_tracks) {
        throw std::out_of_range("TableLayoutPanel column span must be 1 through 64");
    }
    CellMetadata& metadata = metadata_for(child);
    if (metadata.column_span == span) return;
    metadata.column_span = span;
    invalidate(invalidation::bounds);
}

std::size_t TableLayoutPanel::column_span(const Control& child) const {
    const CellMetadata* metadata = metadata_for(child);
    return metadata == nullptr ? 1U : metadata->column_span;
}

void TableLayoutPanel::set_row_span(const Control& child, std::size_t span) {
    require_mutable();
    if (span == 0U || span > maximum_layout_tracks) {
        throw std::out_of_range("TableLayoutPanel row span must be 1 through 64");
    }
    CellMetadata& metadata = metadata_for(child);
    if (metadata.row_span == span) return;
    metadata.row_span = span;
    invalidate(invalidation::bounds);
}

std::size_t TableLayoutPanel::row_span(const Control& child) const {
    const CellMetadata* metadata = metadata_for(child);
    return metadata == nullptr ? 1U : metadata->row_span;
}

void TableLayoutPanel::reconcile_metadata() {
    std::unordered_set<std::uint64_t> live;
    for (const Control::Ptr& child : children()) {
        if (child && child->is_alive() && child->parent().get() == this) {
            live.insert(child->runtime_id().value);
        }
    }
    std::erase_if(metadata_, [&live](const auto& entry) {
        return !live.contains(entry.first);
    });
    std::erase_if(resolved_cells_, [&live](const auto& entry) {
        return !live.contains(entry.first);
    });
}

Size TableLayoutPanel::layout_children(Size available, bool assign) {
    reconcile_metadata();
    const Insets inset = padding();
    const Size inner{std::max(0.0, available.width - horizontal_extent(inset)),
                     std::max(0.0, available.height - vertical_extent(inset))};
    std::size_t columns = column_count_;
    std::size_t rows = row_count_;
    std::vector<std::vector<bool>> occupied(rows,
                                            std::vector<bool>(columns, false));
    const auto resize_grid = [&occupied, &rows, &columns](std::size_t new_columns,
                                                          std::size_t new_rows) {
        if (new_columns != columns) {
            for (auto& row : occupied) row.resize(new_columns, false);
            columns = new_columns;
        }
        if (new_rows != rows) {
            occupied.resize(new_rows, std::vector<bool>(columns, false));
            rows = new_rows;
        }
    };
    const auto region_free = [&occupied, &rows, &columns](
        std::size_t column, std::size_t row,
        std::size_t column_span, std::size_t row_span) {
        if (column + column_span > columns || row + row_span > rows) return false;
        for (std::size_t y = row; y < row + row_span; ++y) {
            for (std::size_t x = column; x < column + column_span; ++x) {
                if (occupied[y][x]) return false;
            }
        }
        return true;
    };
    const auto occupy = [&occupied](std::size_t column, std::size_t row,
                                     std::size_t column_span,
                                     std::size_t row_span) {
        for (std::size_t y = row; y < row + row_span; ++y) {
            for (std::size_t x = column; x < column + column_span; ++x) {
                occupied[y][x] = true;
            }
        }
    };

    struct Item final {
        Control::Ptr control;
        TableLayoutCellPosition position;
        std::size_t column_span{1U};
        std::size_t row_span{1U};
        Size desired;
        Insets margin;
    };
    std::vector<Item> resolved;
    std::vector<Control::Ptr> automatic;
    std::vector<Control::Ptr> overflow;
    resolved_cells_.clear();
    layout_overflowed_ = false;

    const auto grow_to_fit = [&](TableLayoutCellPosition position,
                                 std::size_t column_span,
                                 std::size_t row_span) {
        const std::size_t required_columns = position.column + column_span;
        const std::size_t required_rows = position.row + row_span;
        if (required_columns > maximum_layout_tracks ||
            required_rows > maximum_layout_tracks) return false;
        if (required_columns > columns) {
            if (grow_style_ != TableLayoutGrowStyle::add_columns) return false;
            resize_grid(required_columns, rows);
        }
        if (required_rows > rows) {
            if (grow_style_ != TableLayoutGrowStyle::add_rows) return false;
            resize_grid(columns, required_rows);
        }
        return true;
    };

    const std::vector<Control::Ptr> retained = snapshot_layout_children();
    for (const Control::Ptr& child : retained) {
        if (!is_current_layout_child(child) || !child->visible()) continue;
        const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);
        if (metadata == nullptr || !metadata->position) {
            automatic.push_back(child);
            continue;
        }
        const TableLayoutCellPosition position = *metadata->position;
        if (!grow_to_fit(position, metadata->column_span, metadata->row_span)) {
            overflow.push_back(child);
            continue;
        }
        if (!region_free(position.column, position.row, metadata->column_span,
                         metadata->row_span)) {
            overflow.push_back(child);
            continue;
        }
        occupy(position.column, position.row, metadata->column_span,
               metadata->row_span);
        resolved.push_back({child, position, metadata->column_span,
                            metadata->row_span, {}, child->margin()});
    }

    for (const Control::Ptr& child : automatic) {
        const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);
        const std::size_t column_span = metadata == nullptr
            ? 1U : metadata->column_span;
        const std::size_t row_span = metadata == nullptr ? 1U : metadata->row_span;
        if ((column_span > columns &&
             grow_style_ != TableLayoutGrowStyle::add_columns) ||
            (row_span > rows && grow_style_ != TableLayoutGrowStyle::add_rows)) {
            overflow.push_back(child);
            continue;
        }
        if (column_span > columns) resize_grid(column_span, rows);
        if (row_span > rows) resize_grid(columns, row_span);
        std::optional<TableLayoutCellPosition> position;
        while (!position) {
            for (std::size_t row = 0U; row < rows && !position; ++row) {
                for (std::size_t column = 0U; column < columns; ++column) {
                    if (region_free(column, row, column_span, row_span)) {
                        position = TableLayoutCellPosition{column, row};
                        break;
                    }
                }
            }
            if (position) break;
            if (grow_style_ == TableLayoutGrowStyle::add_rows &&
                rows < maximum_layout_tracks) {
                resize_grid(columns, rows + 1U);
            } else if (grow_style_ == TableLayoutGrowStyle::add_columns &&
                       columns < maximum_layout_tracks) {
                resize_grid(columns + 1U, rows);
            } else {
                break;
            }
        }
        if (!position) {
            overflow.push_back(child);
            continue;
        }
        occupy(position->column, position->row, column_span, row_span);
        resolved.push_back({child, *position, column_span, row_span, {},
                            child->margin()});
    }

    std::vector<TableLayoutStyle> column_styles = column_styles_;
    std::vector<TableLayoutStyle> row_styles = row_styles_;
    column_styles.resize(columns);
    row_styles.resize(rows);
    std::vector<double> column_minimum(columns, 0.0);
    std::vector<double> row_minimum(rows, 0.0);
    std::vector<TrackSpanDemand> column_spans;
    std::vector<TrackSpanDemand> row_spans;
    for (Item& item : resolved) {
        if (!is_current_layout_child(item.control) ||
            !item.control->visible()) {
            item.control.reset();
            continue;
        }
        item.desired = preferred_child_size(item.control, inner);
        if (!is_alive()) return {};
        if (!is_current_layout_child(item.control) ||
            !item.control->visible()) {
            item.control.reset();
            continue;
        }
        const double required_width = item.desired.width +
                                      horizontal_extent(item.margin);
        const double required_height = item.desired.height +
                                       vertical_extent(item.margin);
        if (item.column_span == 1U &&
            column_styles[item.position.column].size_mode !=
                TableSizeMode::absolute) {
            column_minimum[item.position.column] = std::max(
                column_minimum[item.position.column], required_width);
        } else if (item.column_span > 1U) {
            column_spans.push_back(
                {item.position.column, item.column_span, required_width});
        }
        if (item.row_span == 1U &&
            row_styles[item.position.row].size_mode != TableSizeMode::absolute) {
            row_minimum[item.position.row] = std::max(
                row_minimum[item.position.row], required_height);
        } else if (item.row_span > 1U) {
            row_spans.push_back(
                {item.position.row, item.row_span, required_height});
        }
    }

    const TrackResolution horizontal = resolve_table_tracks(
        column_styles, std::move(column_minimum), column_spans, inner.width);
    const TrackResolution vertical = resolve_table_tracks(
        row_styles, std::move(row_minimum), row_spans, inner.height);
    if (assign) {
        column_widths_ = horizontal.actual;
        row_heights_ = vertical.actual;
    }
    std::vector<double> column_offsets(columns + 1U, 0.0);
    std::vector<double> row_offsets(rows + 1U, 0.0);
    std::partial_sum(horizontal.actual.begin(), horizontal.actual.end(),
                     column_offsets.begin() + 1);
    std::partial_sum(vertical.actual.begin(), vertical.actual.end(),
                     row_offsets.begin() + 1);

    for (const Item& item : resolved) {
        if (!item.control || !is_current_layout_child(item.control)) continue;
        resolved_cells_[item.control->runtime_id().value] = item.position;
        if (!assign) continue;
        const double cell_width =
            column_offsets[item.position.column + item.column_span] -
            column_offsets[item.position.column];
        const double cell_height =
            row_offsets[item.position.row + item.row_span] -
            row_offsets[item.position.row];
        const double available_width = std::max(
            0.0, cell_width - horizontal_extent(item.margin));
        const double available_height = std::max(
            0.0, cell_height - vertical_extent(item.margin));
        const double cell_x = inset.left +
            column_offsets[item.position.column] + item.margin.left;
        const double cell_y = inset.top +
            row_offsets[item.position.row] + item.margin.top;
        double child_width = std::min(item.desired.width, available_width);
        double child_height = std::min(item.desired.height, available_height);
        double child_x = cell_x;
        double child_y = cell_y;
        switch (item.control->dock()) {
        case DockStyle::fill:
            child_width = available_width;
            child_height = available_height;
            break;
        case DockStyle::top:
            child_width = available_width;
            break;
        case DockStyle::bottom:
            child_width = available_width;
            child_y += available_height - child_height;
            break;
        case DockStyle::left:
            child_height = available_height;
            break;
        case DockStyle::right:
            child_x += available_width - child_width;
            child_height = available_height;
            break;
        case DockStyle::none: {
            const AnchorStyles anchor = item.control->anchor();
            const bool left = has_anchor(anchor, AnchorStyles::left);
            const bool right = has_anchor(anchor, AnchorStyles::right);
            const bool top = has_anchor(anchor, AnchorStyles::top);
            const bool bottom = has_anchor(anchor, AnchorStyles::bottom);
            if (left && right) child_width = available_width;
            else if (right) child_x += available_width - child_width;
            else if (!left) child_x += (available_width - child_width) * 0.5;
            if (top && bottom) child_height = available_height;
            else if (bottom) child_y += available_height - child_height;
            else if (!top) child_y += (available_height - child_height) * 0.5;
            break;
        }
        }
        set_child_layout(item.control,
                         {child_x, child_y, child_width, child_height});
    }
    layout_overflowed_ = !overflow.empty();
    if (assign) {
        for (const Control::Ptr& child : overflow) {
            if (is_current_layout_child(child)) {
                set_child_layout(child, {inset.left, inset.top, 0.0, 0.0});
            }
        }
    }
    return {horizontal.desired + horizontal_extent(inset),
            vertical.desired + vertical_extent(inset)};
}

Control::Ptr TableLayoutPanel::control_from_position(std::size_t column,
                                                      std::size_t row) const {
    if (attached_window() != nullptr) static_cast<void>(arranged_bounds());
    for (const Control::Ptr& child : children()) {
        if (!child || !child->is_alive() || !child->visible()) continue;
        const auto position = resolved_cells_.find(child->runtime_id().value);
        if (position == resolved_cells_.end()) continue;
        const CellMetadata* metadata = metadata_for(*child);
        const std::size_t column_span = metadata == nullptr
            ? 1U : metadata->column_span;
        const std::size_t row_span = metadata == nullptr ? 1U : metadata->row_span;
        if (column >= position->second.column &&
            column < position->second.column + column_span &&
            row >= position->second.row && row < position->second.row + row_span) {
            return child;
        }
    }
    return {};
}

Size TableLayoutPanel::measure(Size available) {
    available = {std::max(0.0, available.width),
                 std::max(0.0, available.height)};
    if (auto_size()) {
        const Size desired = layout_children(available, false);
        return {std::min(available.width, desired.width),
                std::min(available.height, desired.height)};
    }
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void TableLayoutPanel::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    static_cast<void>(layout_children({final_bounds.width, final_bounds.height}, true));
}

void TableLayoutPanel::on_paint(Painter& painter, Rect) {
    if (cell_border_style_ == TableCellBorderStyle::none ||
        column_widths_.empty() || row_heights_.empty()) {
        return;
    }
    const BasicControlStyle style;
    const Insets inset = padding();
    const double width = std::accumulate(column_widths_.begin(),
                                         column_widths_.end(), 0.0);
    const double height = std::accumulate(row_heights_.begin(),
                                          row_heights_.end(), 0.0);
    const auto draw_grid = [&](Color color, double offset) {
        double x = inset.left;
        painter.draw_line({x + offset, inset.top},
                          {x + offset, inset.top + height}, color, 1.0);
        for (double extent : column_widths_) {
            x += extent;
            painter.draw_line({x + offset, inset.top},
                              {x + offset, inset.top + height}, color, 1.0);
        }
        double y = inset.top;
        painter.draw_line({inset.left, y + offset},
                          {inset.left + width, y + offset}, color, 1.0);
        for (double extent : row_heights_) {
            y += extent;
            painter.draw_line({inset.left, y + offset},
                              {inset.left + width, y + offset}, color, 1.0);
        }
    };
    if (cell_border_style_ == TableCellBorderStyle::single) {
        draw_grid(style.border, 0.0);
    } else if (cell_border_style_ == TableCellBorderStyle::inset) {
        draw_grid(style.dark_border, 0.0);
        draw_grid(style.highlight, 1.0);
    } else {
        draw_grid(style.highlight, 0.0);
        draw_grid(style.dark_border, 1.0);
    }
}

SemanticDescriptor TableLayoutPanel::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = !descriptor.name.empty() || !descriptor.description.empty();
    return descriptor;
}

TabPage::TabPage(StableId stable_id, std::string text)
    : Panel(std::move(stable_id)), text_(std::move(text)) {
    set_background(Color::rgba(255, 255, 255));
    set_border_style(BorderStyle::line);
}

void TabPage::set_text(std::string text) {
    require_mutable();
    if (text_ == text) return;
    text_ = std::move(text);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    if (const Control::Ptr owner = parent()) {
        owner->invalidate(Dirty::paint | Dirty::semantics | Dirty::measure);
    }
}

SemanticDescriptor TabPage::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = accessible_name().empty() ? text_ : accessible_name();
    descriptor.description = accessible_description();
    descriptor.exposed = true;
    return descriptor;
}

TabControl::TabControl(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    set_focusable(true);
}

std::vector<std::shared_ptr<TabPage>> TabControl::pages() const {
    std::vector<std::shared_ptr<TabPage>> result;
    result.reserve(pages_.size());
    for (const std::weak_ptr<TabPage>& weak : pages_) {
        if (const auto page = weak.lock();
            page && page->is_alive() && page->parent().get() == this) {
            result.push_back(page);
        }
    }
    return result;
}

std::size_t TabControl::page_count() const {
    return pages().size();
}

std::shared_ptr<TabPage> TabControl::page_at(std::size_t index) const {
    const auto live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl page index");
    return live[index];
}

std::optional<std::size_t> TabControl::index_of(
    const std::shared_ptr<TabPage>& page) const {
    if (!page) return std::nullopt;
    const auto live = pages();
    const auto found = std::find(live.begin(), live.end(), page);
    if (found == live.end()) return std::nullopt;
    return static_cast<std::size_t>(found - live.begin());
}

std::optional<std::size_t> TabControl::selected_index() const {
    return index_of(selected_page_.lock());
}

void TabControl::add_page(std::shared_ptr<TabPage> page) {
    require_mutable();
    if (!page) throw std::invalid_argument("TabControl page may not be null");
    if (index_of(page)) return;
    page->set_visible(false);
    add_child(page);
    pages_.push_back(page);
    if (!selected_page_.lock()) {
        selected_page_ = page;
        page->set_visible(true);
        const TabSelectionChange change{std::nullopt, 0U};
        selected_index_changed_.emit(change);
    }
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

std::shared_ptr<TabPage> TabControl::remove_page(const TabPage& page) {
    require_mutable();
    const auto live_before = pages();
    const auto found = std::find_if(
        live_before.begin(), live_before.end(),
        [&page](const auto& candidate) { return candidate.get() == &page; });
    if (found == live_before.end()) return {};
    const std::size_t removed_index =
        static_cast<std::size_t>(found - live_before.begin());
    const std::optional<std::size_t> old_selected = selected_index();
    const bool removing_selected = selected_page_.lock().get() == &page;
    pages_.erase(std::remove_if(
        pages_.begin(), pages_.end(), [&page](const std::weak_ptr<TabPage>& weak) {
            const auto candidate = weak.lock();
            return !candidate || candidate.get() == &page;
        }), pages_.end());
    const Control::Ptr removed = remove_child(page.runtime_id());
    if (!removed) return {};

    const auto live_after = pages();
    if (removing_selected) {
        selected_page_.reset();
        if (!live_after.empty()) {
            const std::size_t next = std::min(removed_index, live_after.size() - 1U);
            selected_page_ = live_after[next];
            live_after[next]->set_visible(true);
        }
    }
    const std::optional<std::size_t> new_selected = selected_index();
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
    if (old_selected != new_selected || removing_selected) {
        const TabSelectionChange change{old_selected, new_selected};
        selected_index_changed_.emit(change);
    }
    return std::dynamic_pointer_cast<TabPage>(removed);
}

void TabControl::remember_page_focus(const std::shared_ptr<TabPage>& page) {
    if (!page || window() == nullptr) return;
    const Control::Ptr focused = window()->focused_control();
    if (!focused) return;
    for (Control::Ptr current = focused; current; current = current->parent()) {
        if (current == page) {
            remembered_focus_[page->runtime_id().value] = focused;
            return;
        }
    }
}

void TabControl::restore_page_focus(const std::shared_ptr<TabPage>& page,
                                    bool selection_owned_focus) {
    if (!selection_owned_focus || !page || window() == nullptr) return;
    const auto found = remembered_focus_.find(page->runtime_id().value);
    if (found != remembered_focus_.end()) {
        if (const Control::Ptr candidate = found->second.lock();
            candidate && candidate->eligible_for_input()) {
            for (Control::Ptr current = candidate; current; current = current->parent()) {
                if (current == page) {
                    if (window()->request_focus(candidate)) return;
                    break;
                }
            }
        }
    }
    static_cast<void>(window()->request_focus(shared_from_this()));
}

void TabControl::set_selected_index(std::size_t index) {
    require_mutable();
    const auto live = pages();
    if (index >= live.size()) throw std::out_of_range("TabControl selected index");
    const std::optional<std::size_t> old_index = selected_index();
    if (old_index && *old_index == index) return;
    const auto old_page = selected_page_.lock();
    bool selection_owned_focus{};
    if (old_page && window() != nullptr) {
        const Control::Ptr focused = window()->focused_control();
        for (Control::Ptr current = focused; current; current = current->parent()) {
            if (current == old_page) {
                selection_owned_focus = true;
                break;
            }
        }
        remember_page_focus(old_page);
        old_page->set_visible(false);
    }
    selected_page_ = live[index];
    live[index]->set_visible(true);
    invalidate(Dirty::arrange | Dirty::paint | Dirty::hit_test |
               Dirty::semantics);
    restore_page_focus(live[index], selection_owned_focus);
    const TabSelectionChange change{old_index, index};
    selected_index_changed_.emit(change);
}

void TabControl::set_selected_tab(const std::shared_ptr<TabPage>& page) {
    const auto index = index_of(page);
    if (!index) throw std::invalid_argument("TabPage does not belong to TabControl");
    set_selected_index(*index);
}

void TabControl::set_alignment(TabAlignment alignment) {
    require_mutable();
    if (alignment_ == alignment) return;
    alignment_ = alignment;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void TabControl::set_appearance(TabAppearance appearance) {
    require_mutable();
    if (appearance_ == appearance) return;
    appearance_ = appearance;
    invalidate(Dirty::paint | Dirty::semantics);
}

void TabControl::set_item_size(Size size) {
    require_mutable();
    if (!std::isfinite(size.width) || !std::isfinite(size.height) ||
        size.width < 24.0 || size.height < 18.0) {
        throw std::invalid_argument(
            "TabControl item size must be finite and at least 24 by 18");
    }
    if (item_size_ == size) return;
    item_size_ = size;
    invalidate(Dirty::measure | Dirty::arrange | Dirty::paint |
               Dirty::hit_test | Dirty::semantics);
}

void TabControl::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = std::move(style);
    invalidate(Dirty::paint | Dirty::semantics);
}

Rect TabControl::tab_bounds(std::size_t index) const {
    if (index >= page_count()) throw std::out_of_range("TabControl tab index");
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const Size item{item_size_.width * effective_text_scale(),
                    item_size_.height * effective_text_scale()};
    if (alignment_ == TabAlignment::top || alignment_ == TabAlignment::bottom) {
        const double y = alignment_ == TabAlignment::top
            ? 0.0 : std::max(0.0, bounds.height - item.height);
        return {static_cast<double>(index) * item.width, y,
                item.width, std::min(item.height, bounds.height)};
    }
    const double x = alignment_ == TabAlignment::left
        ? 0.0 : std::max(0.0, bounds.width - item.width);
    return {x, static_cast<double>(index) * item.height,
            std::min(item.width, bounds.width), item.height};
}

Rect TabControl::display_bounds() const noexcept {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const Size item{item_size_.width * effective_text_scale(),
                    item_size_.height * effective_text_scale()};
    switch (alignment_) {
    case TabAlignment::top:
        return {0.0, std::min(bounds.height, item.height - 1.0),
                bounds.width,
                std::max(0.0, bounds.height - item.height + 1.0)};
    case TabAlignment::bottom:
        return {0.0, 0.0, bounds.width,
                std::max(0.0, bounds.height - item.height + 1.0)};
    case TabAlignment::left:
        return {std::min(bounds.width, item.width - 1.0), 0.0,
                std::max(0.0, bounds.width - item.width + 1.0),
                bounds.height};
    case TabAlignment::right:
        return {0.0, 0.0,
                std::max(0.0, bounds.width - item.width + 1.0),
                bounds.height};
    }
    return bounds;
}

Size TabControl::measure(Size available) {
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void TabControl::reconcile_pages() {
    pages_.erase(std::remove_if(
        pages_.begin(), pages_.end(), [this](const std::weak_ptr<TabPage>& weak) {
            const auto page = weak.lock();
            return !page || !page->is_alive() || page->parent().get() != this;
        }), pages_.end());
    auto selected = selected_page_.lock();
    if (selected && selected->is_alive() && selected->parent().get() == this) return;
    selected_page_.reset();
    const auto live = pages();
    if (!live.empty()) {
        selected_page_ = live.front();
        live.front()->set_visible(true);
    }
}

void TabControl::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    reconcile_pages();
    const Rect display = display_bounds();
    const auto selected = selected_page_.lock();
    for (const auto& page : pages()) {
        set_child_layout(page, display);
        page->set_visible(page == selected);
    }
}

void TabControl::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    painter.fill_rect(bounds, style_.face);
    const Rect display = display_bounds();
    painter.fill_rect(display, style_.paper);
    painter.stroke_rect({display.x + 0.5, display.y + 0.5,
                         std::max(0.0, display.width - 1.0),
                         std::max(0.0, display.height - 1.0)},
                        style_.border, 1.0);
    const auto live = pages();
    const FontSpec font = effective_font(font_);
    const std::optional<std::size_t> selected = selected_index();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        Rect tab = tab_bounds(index);
        const bool active = selected && *selected == index;
        if (appearance_ == TabAppearance::buttons && !active) {
            tab = {tab.x + 2.0, tab.y + 2.0,
                   std::max(0.0, tab.width - 4.0),
                   std::max(0.0, tab.height - 4.0)};
        }
        const Color face = active ? style_.paper
            : appearance_ == TabAppearance::flat_buttons ? style_.face_light
                                                         : style_.face;
        painter.fill_rect(tab, face);
        painter.stroke_rect({tab.x + 0.5, tab.y + 0.5,
                             std::max(0.0, tab.width - 1.0),
                             std::max(0.0, tab.height - 1.0)},
                            active ? style_.dark_border : style_.border, 1.0);
        if (active && appearance_ == TabAppearance::normal) {
            if (alignment_ == TabAlignment::top) {
                painter.fill_rect({tab.x + 1.0, tab.y + tab.height - 2.0,
                                   std::max(0.0, tab.width - 2.0), 3.0},
                                  style_.paper);
            } else if (alignment_ == TabAlignment::bottom) {
                painter.fill_rect({tab.x + 1.0, tab.y - 1.0,
                                   std::max(0.0, tab.width - 2.0), 3.0},
                                  style_.paper);
            } else if (alignment_ == TabAlignment::left) {
                painter.fill_rect({tab.x + tab.width - 2.0, tab.y + 1.0, 3.0,
                                   std::max(0.0, tab.height - 2.0)}, style_.paper);
            } else {
                painter.fill_rect({tab.x - 1.0, tab.y + 1.0, 3.0,
                                   std::max(0.0, tab.height - 2.0)}, style_.paper);
            }
        }
        const double text_width = static_cast<double>(live[index]->text().size()) *
                                  font.size * 0.55;
        painter.draw_text_utf8(
            {tab.x + std::max(5.0, (tab.width - text_width) * 0.5),
             tab.y + (tab.height + font.size) * 0.5 - 2.0},
            live[index]->text(), font, enabled() ? style_.text
                                                  : style_.disabled_text);
        if (active && focused_) {
            painter.stroke_rect({tab.x + 4.5, tab.y + 4.5,
                                 std::max(0.0, tab.width - 9.0),
                                 std::max(0.0, tab.height - 9.0)},
                                style_.accent, 1.0);
        }
    }
}

void TabControl::on_pointer(PointerEvent& event) {
    if (!eligible_for_input()) return;
    if (event.action == PointerAction::up &&
        event.button == PointerButton::primary && pointer_engaged_) {
        pointer_engaged_ = false;
        event.handled = true;
        return;
    }
    if (event.action != PointerAction::down ||
        event.button != PointerButton::primary) return;
    const Rect absolute = absolute_bounds();
    const Point local{event.position.x - absolute.x, event.position.y - absolute.y};
    const auto live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (!tab_bounds(index).contains(local)) continue;
        if (window() != nullptr) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        pointer_engaged_ = true;
        set_selected_index(index);
        event.handled = true;
        return;
    }
}

void TabControl::select_relative(int delta) {
    const auto live = pages();
    if (live.empty()) return;
    const std::size_t current = selected_index().value_or(0U);
    const auto count = static_cast<std::ptrdiff_t>(live.size());
    const auto next = (static_cast<std::ptrdiff_t>(current) + delta + count) % count;
    set_selected_index(static_cast<std::size_t>(next));
}

void TabControl::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || !enabled() || page_count() == 0U ||
        window() == nullptr) return;
    const auto modifiers = static_cast<std::uint8_t>(event.modifiers);
    const bool command =
        (modifiers & static_cast<std::uint8_t>(Modifier::control)) != 0U ||
        (modifiers & static_cast<std::uint8_t>(Modifier::meta)) != 0U;
    if (event.physical_key == PhysicalKey::tab && command) {
        const bool reverse =
            (modifiers & static_cast<std::uint8_t>(Modifier::shift)) != 0U;
        select_relative(reverse ? -1 : 1);
        event.handled = true;
        return;
    }
    if (window()->focused_control().get() != this) return;
    if (event.physical_key == PhysicalKey::home) {
        set_selected_index(0U);
    } else if (event.physical_key == PhysicalKey::end) {
        set_selected_index(page_count() - 1U);
    } else if ((alignment_ == TabAlignment::top ||
                alignment_ == TabAlignment::bottom) &&
               event.physical_key == PhysicalKey::left) {
        select_relative(-1);
    } else if ((alignment_ == TabAlignment::top ||
                alignment_ == TabAlignment::bottom) &&
               event.physical_key == PhysicalKey::right) {
        select_relative(1);
    } else if ((alignment_ == TabAlignment::left ||
                alignment_ == TabAlignment::right) &&
               event.physical_key == PhysicalKey::up) {
        select_relative(-1);
    } else if ((alignment_ == TabAlignment::left ||
                alignment_ == TabAlignment::right) &&
               event.physical_key == PhysicalKey::down) {
        select_relative(1);
    } else {
        return;
    }
    event.handled = true;
}

void TabControl::on_focus_changed(bool focused) {
    focused_ = focused;
    if (!focused) pointer_engaged_ = false;
    invalidate(Dirty::paint | Dirty::semantics);
}

SemanticDescriptor TabControl::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::tab_group;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    if (const auto selected = selected_page_.lock()) descriptor.value = selected->text();
    descriptor.actions = {SemanticAction::focus};
    descriptor.exposed = true;
    return descriptor;
}

std::vector<SemanticNode> TabControl::semantic_virtual_children() const {
    std::vector<SemanticNode> nodes;
    const auto live = pages();
    nodes.reserve(live.size());
    const Rect absolute = absolute_bounds();
    const auto selected = selected_page_.lock();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        SemanticNode node;
        node.stable_id = std::string(live[index]->stable_id().value()) + ".tab";
        node.runtime_id = virtual_semantic_runtime_id(node.stable_id);
        node.role = SemanticRole::tab;
        node.name = live[index]->text();
        const Rect local = tab_bounds(index);
        node.bounds = {absolute.x + local.x, absolute.y + local.y,
                       local.width, local.height};
        if (effectively_enabled()) node.states |= SemanticState::enabled;
        node.states |= SemanticState::visible | SemanticState::focusable;
        if (live[index] == selected) node.states |= SemanticState::selected;
        node.actions = {SemanticAction::focus, SemanticAction::select,
                        SemanticAction::press};
        nodes.push_back(std::move(node));
    }
    return nodes;
}

bool TabControl::on_semantic_child_action(std::string_view child_stable_id,
                                          SemanticAction action,
                                          std::string_view) {
    if (action != SemanticAction::focus && action != SemanticAction::select &&
        action != SemanticAction::press) return false;
    const auto live = pages();
    for (std::size_t index = 0U; index < live.size(); ++index) {
        if (child_stable_id !=
            std::string(live[index]->stable_id().value()) + ".tab") continue;
        if (window() != nullptr) {
            static_cast<void>(window()->request_focus(shared_from_this()));
        }
        set_selected_index(index);
        return true;
    }
    return false;
}

SplitterPanel::SplitterPanel(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {}

void SplitterPanel::set_background(Color color) {
    require_mutable();
    if (background_ == color) return;
    background_ = color;
    invalidate(invalidation::style_only);
}

void SplitterPanel::on_paint(Painter& painter, Rect) {
    if (background_.alpha == 0U) return;
    const Rect bounds = committed_arranged_bounds();
    painter.fill_rect({0.0, 0.0, bounds.width, bounds.height}, background_);
}

SplitContainer::SplitContainer(StableId stable_id)
    : ContainerControl(std::move(stable_id)) {
    const std::string prefix(this->stable_id().value());
    first_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel1"));
    second_panel_ = make_control<SplitterPanel>(StableId(prefix + ".panel2"));
    splitter_ = make_control<SplitterGrip>(StableId(prefix + ".splitter"));
    update_splitter_cursor();
}

void SplitContainer::initialize_control_tree() {
    require_mutable();
    if (tree_initialized_) return;
    add_child(first_panel_);
    add_child(second_panel_);
    add_child(splitter_);
    tree_initialized_ = true;
}

void SplitContainer::set_orientation(Orientation orientation) {
    require_mutable();
    if (orientation_ == orientation) return;
    orientation_ = orientation;
    previous_axis_extent_ = 0.0;
    previous_second_extent_ = 0.0;
    update_splitter_cursor();
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_distance(double distance) {
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    set_distance(distance, SplitChangeReason::programmatic);
}

void SplitContainer::set_splitter_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument("splitter width must be finite and positive");
    }
    if (splitter_width_ == width) return;
    splitter_width_ = width;
    if (splitter_hit_width_ < width) splitter_hit_width_ = width;
    static_cast<SplitterGrip&>(*splitter_).set_visible_width(width);
    invalidate(invalidation::bounds);
}

void SplitContainer::set_splitter_hit_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width <= 0.0) {
        throw std::invalid_argument(
            "splitter hit width must be finite and positive");
    }
    width = std::max(width, splitter_width_);
    if (splitter_hit_width_ == width) return;
    splitter_hit_width_ = width;
    invalidate(Dirty::arrange | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void SplitContainer::set_first_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "first panel minimum must be finite and nonnegative");
    if (first_maximum_ && extent > *first_maximum_) {
        throw std::invalid_argument(
            "first panel minimum may not exceed its maximum");
    }
    if (first_minimum_ == extent) return;
    first_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_second_minimum(double extent) {
    require_mutable();
    require_finite_nonnegative(extent,
                               "second panel minimum must be finite and nonnegative");
    if (second_maximum_ && extent > *second_maximum_) {
        throw std::invalid_argument(
            "second panel minimum may not exceed its maximum");
    }
    if (second_minimum_ == extent) return;
    second_minimum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_first_maximum(std::optional<double> extent) {
    require_mutable();
    if (extent) {
        require_finite_nonnegative(*extent,
                                   "first panel maximum must be finite and nonnegative");
        if (*extent < first_minimum_) {
            throw std::invalid_argument(
                "first panel maximum may not be below its minimum");
        }
    }
    if (first_maximum_ == extent) return;
    first_maximum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_second_maximum(std::optional<double> extent) {
    require_mutable();
    if (extent) {
        require_finite_nonnegative(*extent,
                                   "second panel maximum must be finite and nonnegative");
        if (*extent < second_minimum_) {
            throw std::invalid_argument(
                "second panel maximum may not be below its minimum");
        }
    }
    if (second_maximum_ == extent) return;
    second_maximum_ = extent;
    invalidate(invalidation::bounds);
}

void SplitContainer::set_first_collapsed(bool collapsed,
                                         SplitCollapseOrigin origin) {
    require_mutable();
    if (first_collapsed_ == collapsed) return;
    if (collapsed && second_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    const SplitCollapseOrigin previous_origin = first_collapse_origin_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(first_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    first_collapsed_ = collapsed;
    first_collapse_origin_ = collapsed ? origin : SplitCollapseOrigin::none;
    if (!collapsed && previous_origin == SplitCollapseOrigin::automatic_accommodation &&
        origin == SplitCollapseOrigin::user &&
        axis_extent(committed_arranged_bounds()) <
            automatic_collapse_threshold_) {
        automatic_collapse_suppressed_ = true;
    }
    first_panel_->set_visible(!collapsed);
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed ? 0.0
                                    : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse, origin};
    splitter_changed_.emit(change);
}

void SplitContainer::set_second_collapsed(bool collapsed,
                                          SplitCollapseOrigin origin) {
    require_mutable();
    if (second_collapsed_ == collapsed) return;
    if (collapsed && first_collapsed_) {
        throw std::logic_error("both split panels may not be collapsed");
    }
    const double old = effective_distance_;
    const SplitCollapseOrigin previous_origin = second_collapse_origin_;
    if (collapsed) {
        if (effective_distance_ > 0.0) remembered_distance_ = effective_distance_;
        transfer_focus_from(second_panel_);
    } else if (remembered_distance_ >= 0.0) {
        requested_distance_ = remembered_distance_;
    }
    second_collapsed_ = collapsed;
    second_collapse_origin_ = collapsed ? origin : SplitCollapseOrigin::none;
    if (!collapsed && previous_origin == SplitCollapseOrigin::automatic_accommodation &&
        origin == SplitCollapseOrigin::user &&
        axis_extent(committed_arranged_bounds()) <
            automatic_collapse_threshold_) {
        automatic_collapse_suppressed_ = true;
    }
    second_panel_->set_visible(!collapsed);
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = collapsed
        ? std::max(0.0, total - splitter_width_)
        : constrained_distance(requested_distance_, total);
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_,
                                  SplitChangeReason::collapse, origin};
    splitter_changed_.emit(change);
}

void SplitContainer::set_splitter_fixed(bool fixed) {
    require_mutable();
    if (splitter_fixed_ == fixed) return;
    splitter_fixed_ = fixed;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_fixed_panel(SplitFixedPanel panel) {
    require_mutable();
    if (fixed_panel_ == panel) return;
    fixed_panel_ = panel;
    invalidate(Dirty::semantics | Dirty::accessibility);
}

void SplitContainer::set_collapse_panel(SplitFixedPanel panel) {
    require_mutable();
    if (collapse_panel_ == panel) return;
    collapse_panel_ = panel;
    automatic_collapse_suppressed_ = false;
    static_cast<SplitterGrip&>(*splitter_).set_collapse_appearance(
        collapse_panel_, collapse_target_is_collapsed());
    invalidate(Dirty::paint | Dirty::hit_test | Dirty::semantics |
               Dirty::accessibility);
}

void SplitContainer::set_automatic_collapse_threshold(double extent) {
    require_mutable();
    require_finite_nonnegative(
        extent, "automatic collapse threshold must be finite and nonnegative");
    if (automatic_collapse_threshold_ == extent) return;
    automatic_collapse_threshold_ = extent;
    automatic_collapse_suppressed_ = false;
    if (extent == 0.0 && collapse_target_origin() ==
                             SplitCollapseOrigin::automatic_accommodation) {
        if (collapse_panel_ == SplitFixedPanel::first) {
            set_first_collapsed(false,
                                SplitCollapseOrigin::automatic_accommodation);
        } else if (collapse_panel_ == SplitFixedPanel::second) {
            set_second_collapsed(false,
                                 SplitCollapseOrigin::automatic_accommodation);
        }
    }
    invalidate(invalidation::bounds);
}

void SplitContainer::set_keyboard_increment(double increment) {
    require_mutable();
    if (!std::isfinite(increment) || increment <= 0.0) {
        throw std::invalid_argument(
            "splitter keyboard increment must be finite and positive");
    }
    keyboard_increment_ = increment;
}

Size SplitContainer::measure(Size available) {
    const Rect requested = requested_bounds();
    return {std::min(available.width,
                     requested.width > 0.0 ? requested.width : available.width),
            std::min(available.height,
                     requested.height > 0.0 ? requested.height : available.height)};
}

void SplitContainer::arrange(Rect final_bounds) {
    arrange_self(final_bounds);
    const double total = axis_extent(final_bounds);
    reconcile_automatic_collapse(total);
    const double available = std::max(0.0, total - splitter_width_);
    double desired = requested_distance_ < 0.0 ? available * 0.5
                                               : requested_distance_;
    if (previous_axis_extent_ > 0.0 && total != previous_axis_extent_ &&
        fixed_panel_ == SplitFixedPanel::second && !second_collapsed_) {
        desired = std::max(0.0, available - previous_second_extent_);
    }
    const double old = effective_distance_;
    effective_distance_ = constrained_distance(desired, total);
    if (requested_distance_ >= 0.0 || fixed_panel_ == SplitFixedPanel::second) {
        requested_distance_ = effective_distance_;
    }
    const double second_extent = std::max(0.0, available - effective_distance_);
    const double hit_width = std::min(total, std::max(splitter_width_,
                                                      splitter_hit_width_));
    const double hit_origin = std::clamp(
        effective_distance_ + (splitter_width_ - hit_width) * 0.5,
        0.0, std::max(0.0, total - hit_width));
    if (orientation_ == Orientation::vertical) {
        set_child_layout(first_panel_,
            {0.0, 0.0, effective_distance_, final_bounds.height});
        set_child_layout(second_panel_,
            {effective_distance_ + splitter_width_, 0.0, second_extent,
             final_bounds.height});
        set_child_layout(splitter_,
            {hit_origin, 0.0, hit_width, final_bounds.height});
    } else {
        set_child_layout(first_panel_,
            {0.0, 0.0, final_bounds.width, effective_distance_});
        set_child_layout(second_panel_,
            {0.0, effective_distance_ + splitter_width_, final_bounds.width,
             second_extent});
        set_child_layout(splitter_,
            {0.0, hit_origin, final_bounds.width, hit_width});
    }
    previous_axis_extent_ = total;
    previous_second_extent_ = second_extent;
    if (old != effective_distance_) {
        const SplitChangeEvent change{old, effective_distance_,
                                      SplitChangeReason::container_resize};
        splitter_changed_.emit(change);
    }
}

void SplitContainer::on_pointer_preview(PointerEvent& event) {
    const bool on_collapse_tab = collapse_panel_ != SplitFixedPanel::none &&
                                 collapse_tab_bounds().contains(event.position);
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary && on_collapse_tab) {
        collapse_tab_tracking_ = true;
        pointer_tracking_ = false;
        if (window() != nullptr) window()->request_focus(splitter_);
        splitter_->set_pointer_capture(true);
        event.handled = true;
        return;
    }
    if (collapse_tab_tracking_) {
        if (event.action == PointerAction::up) {
            collapse_tab_tracking_ = false;
            splitter_->set_pointer_capture(false);
            if (on_collapse_tab) {
                toggle_collapse_target(SplitCollapseOrigin::user);
            }
        }
        event.handled = true;
        return;
    }
    if (splitter_fixed_ || first_collapsed_ || second_collapsed_) return;
    if (event.action == PointerAction::down &&
        event.button == PointerButton::primary &&
        splitter_->absolute_bounds().contains(event.position)) {
        pointer_tracking_ = true;
        pointer_offset_ = pointer_axis(event.position) - effective_distance_;
        if (window() != nullptr) window()->request_focus(splitter_);
        splitter_->set_pointer_capture(true);
        event.handled = true;
    } else if (event.action == PointerAction::move && pointer_tracking_) {
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    } else if (event.action == PointerAction::up && pointer_tracking_) {
        pointer_tracking_ = false;
        set_distance(pointer_axis(event.position) - pointer_offset_,
                     SplitChangeReason::pointer);
        event.handled = true;
    }
}

void SplitContainer::on_key_preview(KeyEvent& event) {
    if (event.action != KeyAction::down || window() == nullptr ||
        window()->focused_control() != splitter_) return;
    if (collapse_panel_ != SplitFixedPanel::none &&
        (event.physical_key == PhysicalKey::enter ||
         event.physical_key == PhysicalKey::space)) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        event.handled = true;
        return;
    }
    if (splitter_fixed_ || first_collapsed_ || second_collapsed_) return;
    double delta{};
    if (orientation_ == Orientation::vertical) {
        if (event.physical_key == PhysicalKey::left) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::right) delta = keyboard_increment_;
        else return;
    } else {
        if (event.physical_key == PhysicalKey::up) delta = -keyboard_increment_;
        else if (event.physical_key == PhysicalKey::down) delta = keyboard_increment_;
        else return;
    }
    if ((static_cast<std::uint8_t>(event.modifiers) &
         static_cast<std::uint8_t>(Modifier::shift)) != 0U) {
        delta *= 10.0;
    }
    set_distance(effective_distance_ + delta, SplitChangeReason::keyboard);
    event.handled = true;
}

SemanticDescriptor SplitContainer::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::split_pane;
    descriptor.name = accessible_name();
    descriptor.description = accessible_description();
    descriptor.value = std::to_string(effective_distance_);
    if (collapse_panel_ != SplitFixedPanel::none) {
        const bool collapsed = collapse_target_is_collapsed();
        descriptor.description = collapsed
            ? "Split pane collapsed; activate to restore its remembered extent"
            : "Split pane expanded; activate to collapse it";
        descriptor.actions = {SemanticAction::focus,
            collapsed ? SemanticAction::expand : SemanticAction::collapse};
        if (!collapsed) descriptor.states |= SemanticState::expanded;
    }
    descriptor.exposed = true;
    return descriptor;
}

bool SplitContainer::on_semantic_action(SemanticAction action,
                                        std::string_view value) {
    if (action == SemanticAction::expand &&
        collapse_panel_ != SplitFixedPanel::none &&
        collapse_target_is_collapsed()) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        return true;
    }
    if (action == SemanticAction::collapse &&
        collapse_panel_ != SplitFixedPanel::none &&
        !collapse_target_is_collapsed()) {
        toggle_collapse_target(SplitCollapseOrigin::user);
        return true;
    }
    return ContainerControl::on_semantic_action(action, value);
}

double SplitContainer::axis_extent(Rect bounds) const noexcept {
    return orientation_ == Orientation::vertical ? bounds.width : bounds.height;
}

double SplitContainer::pointer_axis(Point point) const noexcept {
    const Rect absolute = absolute_bounds();
    return orientation_ == Orientation::vertical ? point.x - absolute.x
                                                  : point.y - absolute.y;
}

double SplitContainer::constrained_distance(double requested,
                                            double total_extent) const noexcept {
    const double available = std::max(0.0, total_extent - splitter_width_);
    if (first_collapsed_) return 0.0;
    if (second_collapsed_) return available;
    if (!std::isfinite(requested) || requested < 0.0) requested = available * 0.5;
    double lower = std::min(first_minimum_, available);
    double upper = std::max(0.0, available - second_minimum_);
    if (lower > upper) {
        const double requested_minimum = first_minimum_ + second_minimum_;
        const double compromise = requested_minimum > 0.0
            ? available * first_minimum_ / requested_minimum
            : available * 0.5;
        lower = upper = compromise;
    }
    if (first_maximum_) upper = std::min(upper, *first_maximum_);
    if (second_maximum_) {
        lower = std::max(lower, std::max(0.0, available - *second_maximum_));
    }
    if (lower > upper) {
        double compromise = available * 0.5;
        if (first_maximum_ && second_maximum_ &&
            *first_maximum_ + *second_maximum_ > 0.0) {
            compromise = available * *first_maximum_ /
                         (*first_maximum_ + *second_maximum_);
        }
        lower = upper = std::clamp(compromise, 0.0, available);
    }
    return std::clamp(requested, lower, upper);
}

Rect SplitContainer::collapse_tab_bounds() const noexcept {
    const Rect bounds = splitter_->absolute_bounds();
    if (orientation_ == Orientation::vertical) {
        const double height = std::min(34.0, bounds.height);
        return {bounds.x, bounds.y + std::max(0.0, (bounds.height - height) * 0.5),
                bounds.width, height};
    }
    const double width = std::min(34.0, bounds.width);
    return {bounds.x + std::max(0.0, (bounds.width - width) * 0.5), bounds.y,
            width, bounds.height};
}

bool SplitContainer::collapse_target_is_collapsed() const noexcept {
    if (collapse_panel_ == SplitFixedPanel::first) return first_collapsed_;
    if (collapse_panel_ == SplitFixedPanel::second) return second_collapsed_;
    return false;
}

SplitCollapseOrigin SplitContainer::collapse_target_origin() const noexcept {
    if (collapse_panel_ == SplitFixedPanel::first) return first_collapse_origin_;
    if (collapse_panel_ == SplitFixedPanel::second) return second_collapse_origin_;
    return SplitCollapseOrigin::none;
}

void SplitContainer::toggle_collapse_target(SplitCollapseOrigin origin) {
    if (collapse_panel_ == SplitFixedPanel::first) {
        set_first_collapsed(!first_collapsed_, origin);
    } else if (collapse_panel_ == SplitFixedPanel::second) {
        set_second_collapsed(!second_collapsed_, origin);
    }
}

void SplitContainer::reconcile_automatic_collapse(double total_extent) {
    if (collapse_panel_ == SplitFixedPanel::none ||
        automatic_collapse_threshold_ <= 0.0) return;
    const bool constrained = total_extent < automatic_collapse_threshold_;
    if (!constrained) {
        automatic_collapse_suppressed_ = false;
        if (collapse_target_origin() ==
            SplitCollapseOrigin::automatic_accommodation) {
            if (collapse_panel_ == SplitFixedPanel::first) {
                set_first_collapsed(
                    false, SplitCollapseOrigin::automatic_accommodation);
            } else {
                set_second_collapsed(
                    false, SplitCollapseOrigin::automatic_accommodation);
            }
        }
        return;
    }
    if (!automatic_collapse_suppressed_ && !collapse_target_is_collapsed()) {
        if (collapse_panel_ == SplitFixedPanel::first) {
            set_first_collapsed(true,
                SplitCollapseOrigin::automatic_accommodation);
        } else {
            set_second_collapsed(true,
                SplitCollapseOrigin::automatic_accommodation);
        }
    }
}

void SplitContainer::set_distance(double distance, SplitChangeReason reason) {
    require_mutable();
    require_finite_nonnegative(distance,
                               "splitter distance must be finite and nonnegative");
    const double old = effective_distance_;
    requested_distance_ = distance;
    const double total = axis_extent(committed_arranged_bounds());
    effective_distance_ = total > 0.0 ? constrained_distance(distance, total)
                                      : distance;
    if (old == effective_distance_) return;
    invalidate(invalidation::bounds);
    const SplitChangeEvent change{old, effective_distance_, reason};
    splitter_changed_.emit(change);
}

void SplitContainer::transfer_focus_from(
    const std::shared_ptr<SplitterPanel>& panel) {
    if (window() == nullptr) return;
    const Control::Ptr focused = window()->focused_control();
    if (focused == panel || panel->contains_descendant(focused)) {
        window()->request_focus(splitter_);
    }
}

void SplitContainer::update_splitter_cursor() {
    auto& splitter = static_cast<SplitterGrip&>(*splitter_);
    splitter.set_orientation(orientation_);
    splitter.set_visible_width(splitter_width_);
}

} // namespace gui_forms
