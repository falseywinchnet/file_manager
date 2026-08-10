#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace gui_forms {

template <typename Tag> class TextUnitIndex final {
public:
    explicit constexpr TextUnitIndex(std::size_t value = 0) noexcept
        : value_(value) {}
    [[nodiscard]] constexpr std::size_t value() const noexcept { return value_; }
    friend constexpr auto operator<=>(const TextUnitIndex&,
                                      const TextUnitIndex&) = default;
private:
    std::size_t value_{};
};

struct Utf8OffsetTag;
struct Utf16OffsetTag;
struct ScalarIndexTag;
struct GraphemeIndexTag;
struct LineIndexTag;
using Utf8Offset = TextUnitIndex<Utf8OffsetTag>;
using Utf16Offset = TextUnitIndex<Utf16OffsetTag>;
using ScalarIndex = TextUnitIndex<ScalarIndexTag>;
using GraphemeIndex = TextUnitIndex<GraphemeIndexTag>;
using LineIndex = TextUnitIndex<LineIndexTag>;

template <typename Position> struct TextRange final {
    Position start{};
    Position end{};
    friend constexpr bool operator==(const TextRange&,
                                     const TextRange&) = default;
    [[nodiscard]] constexpr bool empty() const noexcept { return start == end; }
};

using Utf8Range = TextRange<Utf8Offset>;
using Utf16Range = TextRange<Utf16Offset>;
using ScalarRange = TextRange<ScalarIndex>;
using GraphemeRange = TextRange<GraphemeIndex>;

enum class Utf8ValidationError : std::uint8_t {
    none, unexpected_continuation, invalid_continuation, truncated_sequence,
    overlong_encoding, surrogate_code_point, code_point_out_of_range,
};

struct Utf8ValidationResult final {
    Utf8ValidationError error{Utf8ValidationError::none};
    Utf8Offset error_offset{};
    std::size_t scalar_count{};
    std::size_t utf16_unit_count{};
    [[nodiscard]] constexpr bool valid() const noexcept {
        return error == Utf8ValidationError::none;
    }
};

[[nodiscard]] Utf8ValidationResult validate_utf8(
    std::string_view text) noexcept;
[[nodiscard]] std::string_view to_string(Utf8ValidationError error) noexcept;
[[nodiscard]] std::string_view grapheme_unicode_version() noexcept;

struct TextStyleId final {
    std::uint32_t value{};
    friend constexpr auto operator<=>(const TextStyleId&,
                                      const TextStyleId&) = default;
};

struct TextStyleSpan final {
    Utf8Range range{};
    TextStyleId style{};
    friend constexpr bool operator==(const TextStyleSpan&,
                                     const TextStyleSpan&) = default;
};

struct TextStoreLimits final {
    std::size_t maximum_utf8_bytes{16U * 1024U * 1024U};
    std::size_t maximum_style_spans{65'536U};
};

struct TextStoreSnapshot final {
    std::size_t utf8_bytes{};
    std::size_t utf16_units{};
    std::size_t scalars{};
    std::size_t graphemes{};
    std::size_t lines{};
    std::size_t style_spans{};
    std::uint64_t revision{};
    std::uint64_t edit_count{};
    std::uint64_t style_update_count{};
    std::uint64_t metadata_rebuild_count{};
    std::uint64_t rejected_mutation_count{};
    std::uint64_t rejected_position_query_count{};
    friend constexpr bool operator==(const TextStoreSnapshot&,
                                     const TextStoreSnapshot&) = default;
};

struct TextEditResult final {
    Utf8Range removed{};
    Utf8Range inserted{};
    std::size_t removed_scalars{};
    std::size_t inserted_scalars{};
    std::uint64_t revision{};
    bool changed{};
};

} // namespace gui_forms
