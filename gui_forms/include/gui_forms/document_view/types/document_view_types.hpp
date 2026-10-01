#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace gui_forms {

struct SourceByteOffset final {
    std::uint64_t value{};
    explicit constexpr SourceByteOffset(std::uint64_t offset = 0) noexcept : value(offset) {}
};
struct DisplayByteOffset final {
    std::uint32_t value{};
    explicit constexpr DisplayByteOffset(std::uint32_t offset = 0) noexcept : value(offset) {}
};
struct DocumentRevision final {
    std::uint64_t document{};
    std::uint64_t revision{};
};
struct SourceByteRange final {
    SourceByteOffset begin{};
    SourceByteOffset end{};
};
struct DocumentViewport final {
    SourceByteOffset anchor{};
    double horizontal_dip{};
};
struct DocumentPageRequest final {
    DocumentRevision revision{};
    std::uint64_t serial{};
    SourceByteRange permitted{};
    DocumentViewport viewport{};
};
enum class DocumentMapKind : std::uint8_t { identity_utf8, atomic_token };
struct DocumentMapSpan final {
    SourceByteRange source{};
    DisplayByteOffset begin{};
    DisplayByteOffset end{};
    DocumentMapKind kind{DocumentMapKind::identity_utf8};
};
enum class DocumentViewStatus : std::uint8_t {
    success, invalid_revision, invalid_range, invalid_page, budget_exceeded,
    stale, cancelled, busy, unavailable, context_required, serial_exhausted,
    invalid_boundary,
};
struct DocumentViewLimits final {
    static constexpr std::uint64_t source_bytes = 65'536;
    static constexpr std::size_t display_capacity = 1'048'576;
    static constexpr std::size_t mapping_capacity = 65'536;
    static constexpr std::size_t producer_slots = 2;
};

// Unique payload ownership; moves empty the former owner, including metadata.
// Producers check all capacity budgets before allocation; no source bytes are
// retained here. Consumer indexing/shaping workspaces have separate budgets.
struct DocumentPage final {
    DocumentPageRequest request{};
    SourceByteRange covered{};
    std::string display_utf8{};
    std::vector<DocumentMapSpan> mapping{};

    DocumentPage() = default;
    DocumentPage(const DocumentPage&) = delete;
    DocumentPage& operator=(const DocumentPage&) = delete;
    DocumentPage(DocumentPage&& other) noexcept;
    DocumentPage& operator=(DocumentPage&& other) noexcept;
    void swap(DocumentPage& other) noexcept;
};

struct DocumentRequestResult final {
    DocumentViewStatus status{DocumentViewStatus::unavailable};
    std::optional<DocumentPageRequest> request{};
};
struct SourceMappingResult final {
    DocumentViewStatus status{DocumentViewStatus::unavailable};
    std::optional<SourceByteOffset> position{};
};
struct DisplayMappingResult final {
    DocumentViewStatus status{DocumentViewStatus::unavailable};
    std::optional<DisplayByteOffset> position{};
};

} // namespace gui_forms
