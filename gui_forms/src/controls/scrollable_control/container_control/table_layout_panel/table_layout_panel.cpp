#include "gui_forms/controls/scrollable_control/container_control/table_layout_panel/table_layout_panel.hpp"
#include "../container_layout_utilities.hpp"

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
constexpr std::size_t maximum_layout_tracks = 64U;

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

} // namespace

using namespace container_layout_detail;

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
    const MetadataMap::const_iterator found =
        metadata_.find(child.runtime_id().value);
    return found == metadata_.end() ? nullptr : &(*found).second;
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
    const ResolvedCellMap::const_iterator resolved =
        resolved_cells_.find(child.runtime_id().value);
    if (resolved != resolved_cells_.end()) return (*resolved).second;
    const CellMetadata* metadata = metadata_for(child);
    return metadata == nullptr ? std::nullopt : (*metadata).position;
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
    return metadata == nullptr ? 1U : (*metadata).column_span;
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
    return metadata == nullptr ? 1U : (*metadata).row_span;
}

void TableLayoutPanel::reconcile_metadata() {
    std::unordered_set<std::uint64_t> live;
    for (const Control::Ptr& child : children()) {
        if (child && (*child).is_alive() && (*child).parent().get() == this) {
            live.insert((*child).runtime_id().value);
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
            for (std::vector<bool>& row : occupied) row.resize(new_columns, false);
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
        if (!is_current_layout_child(child) || !(*child).visible()) continue;
        const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);
        if (metadata == nullptr || !(*metadata).position) {
            automatic.push_back(child);
            continue;
        }
        const TableLayoutCellPosition position = *(*metadata).position;
        if (!grow_to_fit(position, (*metadata).column_span, (*metadata).row_span)) {
            overflow.push_back(child);
            continue;
        }
        if (!region_free(position.column, position.row, (*metadata).column_span,
                         (*metadata).row_span)) {
            overflow.push_back(child);
            continue;
        }
        occupy(position.column, position.row, (*metadata).column_span,
               (*metadata).row_span);
        resolved.push_back({child, position, (*metadata).column_span,
                            (*metadata).row_span, {}, (*child).margin()});
    }

    for (const Control::Ptr& child : automatic) {
        const CellMetadata* metadata = std::as_const(*this).metadata_for(*child);
        const std::size_t column_span = metadata == nullptr
            ? 1U : (*metadata).column_span;
        const std::size_t row_span = metadata == nullptr ? 1U : (*metadata).row_span;
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
        occupy((*position).column, (*position).row, column_span, row_span);
        resolved.push_back({child, *position, column_span, row_span, {},
                            (*child).margin()});
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
            !(*item.control).visible()) {
            item.control.reset();
            continue;
        }
        item.desired = preferred_child_size(item.control, inner);
        if (!is_alive()) return {};
        if (!is_current_layout_child(item.control) ||
            !(*item.control).visible()) {
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
        resolved_cells_[(*item.control).runtime_id().value] = item.position;
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
        switch ((*item.control).dock()) {
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
            const AnchorStyles anchor = (*item.control).anchor();
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
        if (!child || !(*child).is_alive() || !(*child).visible()) continue;
        const ResolvedCellMap::const_iterator position =
            resolved_cells_.find((*child).runtime_id().value);
        if (position == resolved_cells_.end()) continue;
        const CellMetadata* metadata = metadata_for(*child);
        const std::size_t column_span = metadata == nullptr
            ? 1U : (*metadata).column_span;
        const std::size_t row_span = metadata == nullptr ? 1U : (*metadata).row_span;
        if (column >= (*position).second.column &&
            column < (*position).second.column + column_span &&
            row >= (*position).second.row && row < (*position).second.row + row_span) {
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

} // namespace gui_forms
