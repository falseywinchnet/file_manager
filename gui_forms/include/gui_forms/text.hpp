#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

template <typename Tag> class TextUnitIndex final {
public:
  explicit constexpr TextUnitIndex(std::size_t value = 0) noexcept
      : value_(value) {}

  [[nodiscard]] constexpr std::size_t value() const noexcept { return value_; }
  friend constexpr auto operator<=>(const TextUnitIndex &,
                                    const TextUnitIndex &) = default;

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
  friend constexpr bool operator==(const TextRange &,
                                   const TextRange &) = default;

  [[nodiscard]] constexpr bool empty() const noexcept { return start == end; }
};

using Utf8Range = TextRange<Utf8Offset>;
using Utf16Range = TextRange<Utf16Offset>;
using ScalarRange = TextRange<ScalarIndex>;
using GraphemeRange = TextRange<GraphemeIndex>;

enum class Utf8ValidationError : std::uint8_t {
  none,
  unexpected_continuation,
  invalid_continuation,
  truncated_sequence,
  overlong_encoding,
  surrogate_code_point,
  code_point_out_of_range,
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

[[nodiscard]] Utf8ValidationResult
validate_utf8(std::string_view text) noexcept;
[[nodiscard]] std::string_view to_string(Utf8ValidationError error) noexcept;
[[nodiscard]] std::string_view grapheme_unicode_version() noexcept;

struct TextStyleId final {
  std::uint32_t value{};
  friend constexpr auto operator<=>(const TextStyleId &,
                                    const TextStyleId &) = default;
};

struct TextStyleSpan final {
  Utf8Range range{};
  TextStyleId style{};
  friend constexpr bool operator==(const TextStyleSpan &,
                                   const TextStyleSpan &) = default;
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
  friend constexpr bool operator==(const TextStoreSnapshot &,
                                   const TextStoreSnapshot &) = default;
};

struct TextEditResult final {
  Utf8Range removed{};
  Utf8Range inserted{};
  std::size_t removed_scalars{};
  std::size_t inserted_scalars{};
  std::uint64_t revision{};
  bool changed{};
};

// Renderer- and platform-neutral mutable Unicode text baseline. Storage is
// contiguous UTF-8 in M4a; callers depend on typed positions and mutation
// semantics rather than that private representation. The class is not thread
// safe. GUI controls marshal mutations through their UI-thread contract.
class TextStore final {
public:
  explicit TextStore(std::string_view text = {}, TextStoreLimits limits = {});

  [[nodiscard]] std::string_view utf8() const noexcept { return text_; }
  [[nodiscard]] Utf8Offset utf8_size() const noexcept {
    return Utf8Offset(text_.size());
  }
  [[nodiscard]] Utf16Offset utf16_size() const noexcept {
    return Utf16Offset(utf16_units_);
  }
  [[nodiscard]] ScalarIndex scalar_count() const noexcept {
    return ScalarIndex(scalar_count_);
  }
  [[nodiscard]] GraphemeIndex grapheme_count() const noexcept {
    return GraphemeIndex(grapheme_boundaries_.size() - 1U);
  }
  [[nodiscard]] std::size_t line_count() const noexcept {
    return line_starts_.size();
  }
  [[nodiscard]] std::uint64_t revision() const noexcept { return revision_; }
  [[nodiscard]] const TextStoreLimits &limits() const noexcept {
    return limits_;
  }
  [[nodiscard]] std::span<const TextStyleSpan> style_spans() const noexcept {
    return style_spans_;
  }

  void set_text(std::string_view text);
  [[nodiscard]] TextEditResult
  replace(Utf8Range range, std::string_view replacement,
          std::optional<TextStyleId> inserted_style = std::nullopt);

  void set_style_spans(std::span<const TextStyleSpan> spans);
  [[nodiscard]] std::optional<TextStyleId> style_at(Utf8Offset position) const;

  [[nodiscard]] bool is_scalar_boundary(Utf8Offset position) const noexcept;
  [[nodiscard]] bool is_grapheme_boundary(Utf8Offset position) const noexcept;
  [[nodiscard]] Utf8Offset utf8_offset(Utf16Offset position) const;
  [[nodiscard]] Utf8Offset utf8_offset(ScalarIndex position) const;
  [[nodiscard]] Utf8Offset utf8_offset(GraphemeIndex position) const;
  [[nodiscard]] Utf16Offset utf16_offset(Utf8Offset position) const;
  [[nodiscard]] ScalarIndex scalar_index(Utf8Offset position) const;
  [[nodiscard]] GraphemeIndex grapheme_index(Utf8Offset position) const;
  [[nodiscard]] Utf8Offset next_scalar_boundary(Utf8Offset position) const;
  [[nodiscard]] Utf8Offset previous_scalar_boundary(Utf8Offset position) const;
  [[nodiscard]] Utf8Offset next_grapheme_boundary(Utf8Offset position) const;
  [[nodiscard]] Utf8Offset
  previous_grapheme_boundary(Utf8Offset position) const;
  [[nodiscard]] Utf8Range grapheme_range(GraphemeIndex grapheme) const;
  [[nodiscard]] char32_t scalar_at(Utf8Offset position) const;

  [[nodiscard]] Utf8Offset line_start(LineIndex line) const;
  [[nodiscard]] Utf8Range line_content_range(LineIndex line) const;
  [[nodiscard]] TextStoreSnapshot snapshot() const noexcept;

private:
  struct Metadata;

  [[nodiscard]] static Metadata analyze(std::string_view text);
  [[nodiscard]] std::vector<TextStyleSpan>
  normalize_style_spans(std::span<const TextStyleSpan> spans,
                        std::string_view candidate_text) const;
  [[nodiscard]] std::vector<TextStyleSpan>
  transform_style_spans(Utf8Range removed, std::size_t inserted_bytes,
                        std::optional<TextStyleId> inserted_style,
                        std::string_view candidate_text) const;
  void validate_position(Utf8Offset position) const;
  void validate_range(Utf8Range range) const;
  [[noreturn]] void reject_invalid_utf8(const Utf8ValidationResult &validation);

  TextStoreLimits limits_;
  std::string text_;
  std::vector<std::size_t> line_starts_;
  std::vector<std::size_t> line_content_ends_;
  std::vector<std::size_t> grapheme_boundaries_;
  std::vector<TextStyleSpan> style_spans_;
  std::size_t scalar_count_{};
  std::size_t utf16_units_{};
  std::uint64_t revision_{};
  std::uint64_t edit_count_{};
  std::uint64_t style_update_count_{};
  std::uint64_t metadata_rebuild_count_{};
  std::uint64_t rejected_mutation_count_{};
  mutable std::uint64_t rejected_position_query_count_{};
};

} // namespace gui_forms
