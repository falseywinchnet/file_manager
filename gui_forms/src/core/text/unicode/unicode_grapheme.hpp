#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace gui_forms::detail {

// Input must already have passed validate_utf8(). The result always contains
// byte zero and the end byte; equal values represent the empty text boundary.
[[nodiscard]] std::vector<std::size_t>
extended_grapheme_boundaries(std::string_view valid_utf8,
                             std::size_t scalar_count);

} // namespace gui_forms::detail
