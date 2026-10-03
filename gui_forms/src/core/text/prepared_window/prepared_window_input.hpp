#pragma once
#include "../prepared/prepared_storage.hpp"
#include <string_view>

namespace gui_forms::detail {

// Private candidate records; not an exported DocumentView or a renderable layout.
struct PreparedWindowKey final {
    PreparedTextKey text{};
    std::uint64_t controller_instance{};
    std::uint64_t projection_generation{};
    LayoutAuthority authority{};
};
enum class PreparedWindowSeparator { none, lf, cr, crlf };
struct PreparedWindowParagraph final {
    SourceByteRange content{};
    DisplayByteOffset display_begin{}, display_end{};
    PreparedParagraphProof proof{};
    PreparedWindowSeparator separator{PreparedWindowSeparator::none};
    SourceByteRange separator_source{};
    DisplayByteOffset separator_begin{}, separator_end{};
};
struct PreparedWindowView final {
    PreparedWindowKey key{};
    std::string_view display{};
    std::span<const DocumentMapSpan> mappings{};
    std::span<const PreparedSourceEndpoint> endpoints{};
    std::span<const PreparedWindowParagraph> paragraphs{};
    bool complete_eof{};
};
struct PreparedWindowInput final {
    PreparedWindowInput() = default;
    PreparedWindowInput(const PreparedWindowInput&) = delete;
    PreparedWindowInput& operator=(const PreparedWindowInput&) = delete;
    PreparedWindowInput(PreparedWindowInput&&) = delete;
    PreparedWindowInput& operator=(PreparedWindowInput&&) = delete;
    PreparedReservation reservation{};
    PreparedWindowKey key{};
    std::unique_ptr<const char[]> display{};
    std::unique_ptr<const DocumentMapSpan[]> mappings{};
    std::unique_ptr<const PreparedSourceEndpoint[]> endpoints{};
    std::unique_ptr<const PreparedWindowParagraph[]> paragraphs{};
    std::size_t display_bytes{}, mapping_count{}, endpoint_count{}, paragraph_count{};
    std::size_t charged_bytes{};
    bool complete_eof{};
};
[[nodiscard]] bool same_prepared_window_key(const PreparedWindowKey&, const PreparedWindowKey&) noexcept;
// Borrows input for this call only. Failure preserves bytes and output ownership.
// Success reports exact allocation requests, excluding allocator bookkeeping.
[[nodiscard]] PreparedTextStatus validate_prepared_window(const PreparedWindowView&, std::size_t& bytes) noexcept;
// An occupied output is busy. The existing A2 ledger enforces one input owner.
// The reservation precedes allocation and survives until the immutable owner dies.
[[nodiscard]] PreparedTextStatus own_prepared_window(const PreparedWindowView&,
    std::shared_ptr<PreparedLedger>, std::unique_ptr<const PreparedWindowInput>& output);
}
