#pragma once

#include "../../../core/text/shaping/shaped_text_geometry.hpp"
#include "../../../core/text/unicode/unicode_grapheme.hpp"

#include <cstddef>
#include <memory>

namespace gui_forms::render::text {

class HarfBuzzFontEngine;
struct ShapeStorageLimits;

// Face indices refer only to the current engine's stable registration table.
// They are overwritten before use on each call; no native face borrow persists.
struct WorkspaceFontSegment final {
    std::size_t face_index{};
    Utf8Range range{};
};
struct WorkspaceDirectionRun final {
    Utf8Range range{};
    bool rtl{};
};
struct WorkspaceVisualSegment final {
    std::size_t face_index{};
    Utf8Range range{};
    bool rtl{};
};

// Private, serial-executor scratch. Prepare before repeated shaping. No native
// owners, input borrows, callbacks or retained output aliases are kept here.
// Nonmovable so the executor's scratch identity cannot migrate during a call.
class BoundedShapeWorkspace final {
public:
    BoundedShapeWorkspace() = default;
    ~BoundedShapeWorkspace() = default;
    BoundedShapeWorkspace(const BoundedShapeWorkspace&) = delete;
    BoundedShapeWorkspace& operator=(const BoundedShapeWorkspace&) = delete;
    BoundedShapeWorkspace(BoundedShapeWorkspace&&) = delete;
    BoundedShapeWorkspace& operator=(BoundedShapeWorkspace&&) = delete;
    // Includes this owner and its controlled arrays, not the engine/call records.
    [[nodiscard]] std::size_t controlled_bytes() const noexcept;
private:
    friend class HarfBuzzFontEngine;
    void prepare(const ShapeStorageLimits& limits, std::size_t external_bytes);
    void clear_result() noexcept;
    // Copies only completed active values. No workspace array is transferred.
    [[nodiscard]] std::unique_ptr<BoundedShapedText> copy_result(
        std::size_t output_limit, std::size_t workspace_peak) const;

    detail::GraphemeWorkspace graphemes_{};
    std::unique_ptr<char32_t[]> scalars_{};
    std::unique_ptr<WorkspaceFontSegment[]> segments_{};
    std::unique_ptr<WorkspaceDirectionRun[]> directions_{};
    std::unique_ptr<WorkspaceVisualSegment[]> visual_{};
    BoundedShapedText staging_{};
    std::size_t scalar_capacity_{};
    std::size_t visual_capacity_{};
    std::size_t array_bytes_{};
};

} // namespace gui_forms::render::text
