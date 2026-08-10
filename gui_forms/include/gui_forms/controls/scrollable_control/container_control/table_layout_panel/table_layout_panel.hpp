#pragma once

#include "gui_forms/controls/scrollable_control/container_control/container_control.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

namespace gui_forms {

enum class TableSizeMode : std::uint8_t {
    absolute,
    percent,
    auto_size,
};

struct TableLayoutStyle final {
    TableSizeMode size_mode{TableSizeMode::auto_size};
    double size{};
    friend constexpr bool operator==(const TableLayoutStyle&,
                                     const TableLayoutStyle&) = default;
};

struct TableLayoutCellPosition final {
    std::size_t column{};
    std::size_t row{};
    friend constexpr bool operator==(const TableLayoutCellPosition&,
                                     const TableLayoutCellPosition&) = default;
};

enum class TableLayoutGrowStyle : std::uint8_t {
    fixed_size,
    add_rows,
    add_columns,
};

enum class TableCellBorderStyle : std::uint8_t {
    none,
    single,
    inset,
    outset,
};

class TableLayoutPanel final : public ContainerControl {
public:
    explicit TableLayoutPanel(StableId stable_id);

    [[nodiscard]] std::size_t column_count() const noexcept {
        return column_count_;
    }
    void set_column_count(std::size_t count);
    [[nodiscard]] std::size_t row_count() const noexcept { return row_count_; }
    void set_row_count(std::size_t count);
    [[nodiscard]] TableLayoutGrowStyle grow_style() const noexcept {
        return grow_style_;
    }
    void set_grow_style(TableLayoutGrowStyle style);
    [[nodiscard]] bool auto_size() const noexcept override {
        return Control::auto_size();
    }
    void set_auto_size(bool auto_size) override;
    [[nodiscard]] TableCellBorderStyle cell_border_style() const noexcept {
        return cell_border_style_;
    }
    void set_cell_border_style(TableCellBorderStyle style);

    [[nodiscard]] std::span<const TableLayoutStyle> column_styles() const noexcept {
        return column_styles_;
    }
    [[nodiscard]] std::span<const TableLayoutStyle> row_styles() const noexcept {
        return row_styles_;
    }
    void set_column_style(std::size_t column, TableLayoutStyle style);
    void set_row_style(std::size_t row, TableLayoutStyle style);

    void set_cell_position(const Control& child,
                           TableLayoutCellPosition position);
    void clear_cell_position(const Control& child);
    [[nodiscard]] std::optional<TableLayoutCellPosition> cell_position(
        const Control& child) const;
    void set_column_span(const Control& child, std::size_t span);
    [[nodiscard]] std::size_t column_span(const Control& child) const;
    void set_row_span(const Control& child, std::size_t span);
    [[nodiscard]] std::size_t row_span(const Control& child) const;
    [[nodiscard]] Control::Ptr control_from_position(std::size_t column,
                                                     std::size_t row) const;

    [[nodiscard]] std::span<const double> column_widths() const noexcept {
        return column_widths_;
    }
    [[nodiscard]] std::span<const double> row_heights() const noexcept {
        return row_heights_;
    }
    [[nodiscard]] bool layout_overflowed() const noexcept {
        return layout_overflowed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    struct CellMetadata final {
        std::optional<TableLayoutCellPosition> position;
        std::size_t column_span{1U};
        std::size_t row_span{1U};
    };

    [[nodiscard]] CellMetadata& metadata_for(const Control& child);
    [[nodiscard]] const CellMetadata* metadata_for(const Control& child) const;
    void reconcile_metadata();
    [[nodiscard]] Size layout_children(Size available, bool assign);
    static void validate_style(TableLayoutStyle style);

    std::unordered_map<std::uint64_t, CellMetadata> metadata_;
    std::unordered_map<std::uint64_t, TableLayoutCellPosition> resolved_cells_;
    std::vector<TableLayoutStyle> column_styles_{TableLayoutStyle{}};
    std::vector<TableLayoutStyle> row_styles_{TableLayoutStyle{}};
    std::vector<double> column_widths_;
    std::vector<double> row_heights_;
    std::size_t column_count_{1U};
    std::size_t row_count_{1U};
    TableLayoutGrowStyle grow_style_{TableLayoutGrowStyle::add_rows};
    TableCellBorderStyle cell_border_style_{TableCellBorderStyle::none};
    bool layout_overflowed_{};
};

} // namespace gui_forms
