#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace jpeg_research {

enum class OrientationStatus : std::uint8_t { absent, present, malformed, limit };

struct OrientationResult final {
    OrientationStatus status{OrientationStatus::absent};
    std::uint16_t value{1U};
};

// Borrowed complete encoded JPEG, at most 16 MiB. No allocation, retained input,
// recursion or IFD-link traversal. Examines only the pre-SOS marker region and
// EXIF IFD0's orientation; this is not a JPEG/EXIF conformance validator.
// Missing orientation defaults to 1. Duplicate EXIF or orientation is refused.
[[nodiscard]] OrientationResult read_orientation(const std::span<const unsigned char> encoded) noexcept;

} // namespace jpeg_research
