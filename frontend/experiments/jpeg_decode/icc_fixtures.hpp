#pragma once

#include <vector>

namespace jpeg_research {

enum class IccFixture { srgb, linear_rgb, linear_gray };
[[nodiscard]] std::vector<unsigned char> make_icc_fixture(const IccFixture kind);
[[nodiscard]] int encode_linear_srgb(const unsigned char sample);
void verify_icc_conversion();

} // namespace jpeg_research
