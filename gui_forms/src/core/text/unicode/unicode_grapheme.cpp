#include "unicode_grapheme.hpp"

#include "unicode_grapheme_data.hpp"
#include "gui_forms/detail/algorithm/binary_search.hpp"
#include "gui_forms/text.hpp"

#include <cstdint>
#include <limits>
#include <utility>
#include <span>
#include <vector>

namespace gui_forms::detail {
namespace {

using unicode_data::GraphemeBreak;
using unicode_data::IndicConjunctBreak;

struct Scalar final {
  std::size_t byte_offset{};
  GraphemeBreak grapheme_break{GraphemeBreak::Other};
  IndicConjunctBreak indic_break{IndicConjunctBreak::None};
  bool extended_pictographic{};
};

template <typename Value>
struct ScalarBeforeUnicodeRange final {
  [[nodiscard]] constexpr bool operator()(
      char32_t scalar,
      const unicode_data::UnicodeRange<Value> &range) const noexcept {
    return scalar < range.first;
  }
};

template <typename Value, std::size_t Size>
[[nodiscard]] constexpr Value
lookup(char32_t scalar,
       const std::array<unicode_data::UnicodeRange<Value>, Size> &ranges,
       Value missing) noexcept {
  const std::size_t position = upper_bound_index(
      std::span<const unicode_data::UnicodeRange<Value>, Size>(ranges), scalar,
      ScalarBeforeUnicodeRange<Value>{});
  if (position == 0U) {
    return missing;
  }
  const unicode_data::UnicodeRange<Value> &candidate = ranges[position - 1U];
  return scalar <= candidate.last ? candidate.value : missing;
}

[[nodiscard]] char32_t decode(std::string_view text,
                              std::size_t &offset) noexcept {
  const std::uint8_t first = static_cast<std::uint8_t>(text[offset++]);
  if (first < 0x80U) {
    return first;
  }
  char32_t value = first < 0xe0U ? first & 0x1fU
                   : first < 0xf0U ? first & 0x0fU
                                    : first & 0x07U;
  const std::size_t continuation_count =
      first < 0xe0U ? 1U : first < 0xf0U ? 2U : 3U;
  for (std::size_t index = 0; index < continuation_count; ++index) {
    value = static_cast<char32_t>(
        (value << 6U) | (static_cast<std::uint8_t>(text[offset++]) & 0x3fU));
  }
  return value;
}

[[nodiscard]] bool is_control(GraphemeBreak value) noexcept {
  return value == GraphemeBreak::Control || value == GraphemeBreak::CR ||
         value == GraphemeBreak::LF;
}

[[nodiscard]] bool should_break(std::span<const Scalar> scalars,
                                std::size_t right_index) noexcept {
  const Scalar &left = scalars[right_index - 1U];
  const Scalar &right = scalars[right_index];
  const GraphemeBreak left_break = left.grapheme_break;
  const GraphemeBreak right_break = right.grapheme_break;

  // GB3-GB5: paired CRLF and mandatory control boundaries.
  if (left_break == GraphemeBreak::CR && right_break == GraphemeBreak::LF) {
    return false;
  }
  if (is_control(left_break) || is_control(right_break)) {
    return true;
  }

  // GB6-GB8: conjoining Hangul syllable sequences.
  if (left_break == GraphemeBreak::L &&
      (right_break == GraphemeBreak::L || right_break == GraphemeBreak::V ||
       right_break == GraphemeBreak::LV || right_break == GraphemeBreak::LVT)) {
    return false;
  }
  if ((left_break == GraphemeBreak::LV || left_break == GraphemeBreak::V) &&
      (right_break == GraphemeBreak::V || right_break == GraphemeBreak::T)) {
    return false;
  }
  if ((left_break == GraphemeBreak::LVT || left_break == GraphemeBreak::T) &&
      right_break == GraphemeBreak::T) {
    return false;
  }

  // GB9-GB9b: extending marks, spacing marks, and prepended characters.
  if (right_break == GraphemeBreak::Extend ||
      right_break == GraphemeBreak::ZWJ ||
      right_break == GraphemeBreak::SpacingMark) {
    return false;
  }
  if (left_break == GraphemeBreak::Prepend) {
    return false;
  }

  // GB9c: conjunct-linking viramas and consonants.
  if (right.indic_break == IndicConjunctBreak::Consonant) {
    bool has_linker = false;
    std::size_t index = right_index;
    while (index > 0U) {
      --index;
      const IndicConjunctBreak property = scalars[index].indic_break;
      if (property == IndicConjunctBreak::Linker) {
        has_linker = true;
        continue;
      }
      if (property == IndicConjunctBreak::Extend) {
        continue;
      }
      if (property == IndicConjunctBreak::Consonant && has_linker) {
        return false;
      }
      break;
    }
  }

  // GB11: emoji ZWJ sequences, with any extending marks before the joiner.
  if (left_break == GraphemeBreak::ZWJ && right.extended_pictographic) {
    std::size_t index = right_index - 1U;
    while (index > 0U &&
           scalars[index - 1U].grapheme_break == GraphemeBreak::Extend) {
      --index;
    }
    if (index > 0U && scalars[index - 1U].extended_pictographic) {
      return false;
    }
  }

  // GB12-GB13: regional indicators pair from the start of each RI run.
  if (left_break == GraphemeBreak::Regional_Indicator &&
      right_break == GraphemeBreak::Regional_Indicator) {
    std::size_t preceding_regional_indicators = 0;
    std::size_t index = right_index;
    while (index > 0U &&
           scalars[index - 1U].grapheme_break ==
               GraphemeBreak::Regional_Indicator) {
      --index;
      ++preceding_regional_indicators;
    }
    if ((preceding_regional_indicators % 2U) == 1U) {
      return false;
    }
  }

  return true; // GB999
}

} // namespace

GraphemeStorageRequirement grapheme_storage_requirement(std::string_view utf8) noexcept {
  const Utf8ValidationResult validation = validate_utf8(utf8);
  if (!validation.valid()) return {};
  const std::size_t count = validation.scalar_count;
  const std::size_t maximum = std::numeric_limits<std::size_t>::max();
  if (count == maximum || count > maximum / sizeof(Scalar)) {
    return {.status = GraphemeStorageStatus::budget_exceeded};
  }
  const std::size_t capacity = count + 1U;
  if (capacity > maximum / sizeof(std::size_t)) {
    return {.status = GraphemeStorageStatus::budget_exceeded};
  }
  const std::size_t scalar_bytes = count * sizeof(Scalar);
  const std::size_t boundary_bytes = capacity * sizeof(std::size_t);
  if (scalar_bytes > maximum - boundary_bytes) {
    return {.status = GraphemeStorageStatus::budget_exceeded};
  }
  const GraphemeStorageRequirement result{
      .status = GraphemeStorageStatus::success, .scalar_count = count,
      .scalar_bytes = scalar_bytes, .boundary_capacity = capacity,
      .boundary_bytes = boundary_bytes, .peak_bytes = scalar_bytes + boundary_bytes};
  return result;
}

GraphemeBoundaryBuffer::GraphemeBoundaryBuffer(GraphemeBoundaryBuffer&& other) noexcept
    : values_(std::move(other.values_)), count_(std::exchange(other.count_, 0U)),
      capacity_(std::exchange(other.capacity_, 0U)) {}

GraphemeBoundaryBuffer& GraphemeBoundaryBuffer::operator=(GraphemeBoundaryBuffer&& other) noexcept {
  if (this != &other) {
    values_ = std::move(other.values_);
    count_ = std::exchange(other.count_, 0U);
    capacity_ = std::exchange(other.capacity_, 0U);
  }
  return *this;
}

std::span<const std::size_t> GraphemeBoundaryBuffer::boundaries() const noexcept {
  const std::span<const std::size_t> result(values_.get(), count_);
  return result;
}

std::size_t GraphemeBoundaryBuffer::capacity_bytes() const noexcept {
  const std::size_t bytes = capacity_ * sizeof(std::size_t);
  return bytes;
}

GraphemeStorageStatus bounded_grapheme_boundaries(std::string_view utf8,
    std::size_t maximum_live_bytes, GraphemeBoundaryBuffer& output) {
  const GraphemeStorageRequirement requirement = grapheme_storage_requirement(utf8);
  if (requirement.status != GraphemeStorageStatus::success) return requirement.status;
  const std::size_t retained = output.capacity_bytes();
  if (retained > maximum_live_bytes || requirement.peak_bytes > maximum_live_bytes - retained) {
    return GraphemeStorageStatus::budget_exceeded;
  }
  GraphemeBoundaryBuffer candidate{};
  try {
    std::unique_ptr<Scalar[]> scalars{};
    if (requirement.scalar_count != 0U) scalars = std::make_unique<Scalar[]>(requirement.scalar_count);
    candidate.values_ = std::make_unique<std::size_t[]>(requirement.boundary_capacity);
    candidate.capacity_ = requirement.boundary_capacity;
    std::size_t offset = 0U;
    for (std::size_t index = 0U; index < requirement.scalar_count; ++index) {
      const std::size_t byte_offset = offset;
      const char32_t value = decode(utf8, offset);
      scalars[index] = Scalar{
          .byte_offset = byte_offset,
          .grapheme_break = lookup(value, unicode_data::grapheme_break_ranges, GraphemeBreak::Other),
          .indic_break = lookup(value, unicode_data::indic_conjunct_break_ranges, IndicConjunctBreak::None),
          .extended_pictographic = lookup(value, unicode_data::extended_pictographic_ranges, false)};
    }
    candidate.values_[0] = 0U;
    candidate.count_ = 1U;
    const std::span<const Scalar> decoded(scalars.get(), requirement.scalar_count);
    for (std::size_t index = 1U; index < requirement.scalar_count; ++index) {
      if (should_break(decoded, index)) {
        candidate.values_[candidate.count_] = decoded[index].byte_offset;
        ++candidate.count_;
      }
    }
    if (utf8.size() != candidate.values_[candidate.count_ - 1U]) {
      candidate.values_[candidate.count_] = utf8.size();
      ++candidate.count_;
    }
  } catch (const std::bad_alloc&) {
    return GraphemeStorageStatus::resource_failure;
  }
  output = std::move(candidate);
  return GraphemeStorageStatus::success;
}

std::vector<std::size_t>
extended_grapheme_boundaries(std::string_view valid_utf8,
                             std::size_t scalar_count) {
  std::vector<Scalar> scalars;
  scalars.reserve(scalar_count);
  std::size_t offset = 0;
  while (offset < valid_utf8.size()) {
    const std::size_t byte_offset = offset;
    const char32_t value = decode(valid_utf8, offset);
    scalars.push_back(
        {byte_offset,
         lookup(value, unicode_data::grapheme_break_ranges,
                GraphemeBreak::Other),
         lookup(value, unicode_data::indic_conjunct_break_ranges,
                IndicConjunctBreak::None),
         lookup(value, unicode_data::extended_pictographic_ranges, false)});
  }

  std::vector<std::size_t> boundaries;
  boundaries.reserve(scalars.size() + 1U);
  boundaries.push_back(0);
  for (std::size_t index = 1; index < scalars.size(); ++index) {
    if (should_break(scalars, index)) {
      boundaries.push_back(scalars[index].byte_offset);
    }
  }
  if (valid_utf8.size() != boundaries.back()) {
    boundaries.push_back(valid_utf8.size());
  }
  return boundaries;
}

} // namespace gui_forms::detail
