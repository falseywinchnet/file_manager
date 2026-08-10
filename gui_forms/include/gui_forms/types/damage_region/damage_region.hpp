#pragma once

#include "gui_forms/types/geometry/geometry.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace gui_forms {

class DamageRegion {
public:
    static constexpr std::size_t maximum_rectangles = 64;

    void add(Rect rect);
    void clear() noexcept;

    [[nodiscard]] bool empty() const noexcept { return rectangles_.empty(); }
    [[nodiscard]] std::span<const Rect> rectangles() const noexcept { return rectangles_; }
    [[nodiscard]] std::size_t rectangle_count() const noexcept { return rectangles_.size(); }
    [[nodiscard]] std::uint64_t compaction_count() const noexcept { return compaction_count_; }
    [[nodiscard]] std::uint64_t collapse_count() const noexcept { return collapse_count_; }
    [[nodiscard]] Rect bounds() const noexcept;
    [[nodiscard]] double area() const noexcept;

private:
    std::vector<Rect> rectangles_;
    std::uint64_t compaction_count_{};
    std::uint64_t collapse_count_{};
};

} // namespace gui_forms
