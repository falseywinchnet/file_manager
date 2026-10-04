#pragma once

#include "jpeg_pixels.hpp"

#include <cstddef>
#include <span>

namespace jpeg_research {

constexpr std::size_t icc_profile_limit{1024U * 1024U};
enum class IccSource { rgb, gray };

// Research only: synchronously borrows profile and tightly packed opaque BGRA
// source (<=1024 square). Returns separately owned sRGB BGRA or throws without
// modifying either input. Relative colorimetric, no black-point compensation.
// Gray input requires equal B/G/R bytes. All resources and callbacks are local
// to this invocation. The byte cap is not a process memory/deadline guarantee.
[[nodiscard]] Pixels convert_icc_to_srgb(const Pixels& source,
    const std::span<const unsigned char> profile, const IccSource source_space);

} // namespace jpeg_research
