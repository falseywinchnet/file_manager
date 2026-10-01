#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace gui_forms::detail {

// Input must already have passed validate_utf8(). The result always contains
// byte zero and the end byte; equal values represent the empty text boundary.
[[nodiscard]] std::vector<std::size_t>
extended_grapheme_boundaries(std::string_view valid_utf8,
                             std::size_t scalar_count);

enum class GraphemeStorageStatus { success, invalid_utf8, budget_exceeded, resource_failure };

struct GraphemeStorageRequirement final {
    GraphemeStorageStatus status{GraphemeStorageStatus::invalid_utf8};
    std::size_t scalar_count{};
    std::size_t scalar_bytes{};
    std::size_t boundary_capacity{};
    std::size_t boundary_bytes{};
    std::size_t peak_bytes{};
};

// Counts controlled array payload, excluding allocator bookkeeping. Validation
// and arithmetic allocate nothing. No estimate of opaque native-library memory.
[[nodiscard]] GraphemeStorageRequirement grapheme_storage_requirement(std::string_view utf8) noexcept;

class GraphemeBoundaryBuffer final {
public:
    GraphemeBoundaryBuffer() = default;
    ~GraphemeBoundaryBuffer() = default;
    GraphemeBoundaryBuffer(const GraphemeBoundaryBuffer&) = delete;
    GraphemeBoundaryBuffer& operator=(const GraphemeBoundaryBuffer&) = delete;
    GraphemeBoundaryBuffer(GraphemeBoundaryBuffer&& other) noexcept;
    GraphemeBoundaryBuffer& operator=(GraphemeBoundaryBuffer&& other) noexcept;
    // Immutable borrow ends at successful replacement, move or destruction.
    [[nodiscard]] std::span<const std::size_t> boundaries() const noexcept;
    [[nodiscard]] std::size_t capacity_bytes() const noexcept;
private:
    friend GraphemeStorageStatus bounded_grapheme_boundaries(std::string_view,
        std::size_t, GraphemeBoundaryBuffer&);
    std::unique_ptr<std::size_t[]> values_{};
    std::size_t count_{};
    std::size_t capacity_{};
};

// Failure preserves output. Budget includes old output plus simultaneous new
// scalar/boundary arrays. Success destroys scratch before replacing old output.
[[nodiscard]] GraphemeStorageStatus bounded_grapheme_boundaries(std::string_view utf8,
    std::size_t maximum_live_bytes, GraphemeBoundaryBuffer& output);

} // namespace gui_forms::detail
