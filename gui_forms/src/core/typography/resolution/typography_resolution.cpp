#include "gui_forms/typography.hpp"
#include "gui_forms/text.hpp"

#include <algorithm>
#include <cmath>

namespace gui_forms {

const char* text_resolution_status_name(TextResolutionStatus status) noexcept {
    switch (status) {
    case TextResolutionStatus::exact: return "exact";
    case TextResolutionStatus::estimated: return "estimated";
    case TextResolutionStatus::missing_primary_face:
        return "missing_primary_face";
    case TextResolutionStatus::missing_cluster_coverage:
        return "missing_cluster_coverage";
    case TextResolutionStatus::invalid_request: return "invalid_request";
    }
    return "invalid_request";
}

ResolvedTextLayout estimate_text_layout_utf8(
    std::string_view utf8, FontSpec effective_font) noexcept {
    ResolvedTextLayout result;
    result.effective_font = effective_font;
    if (!valid_font_spec(effective_font) || !validate_utf8(utf8).valid()) {
        result.status = TextResolutionStatus::invalid_request;
        return result;
    }
    std::size_t scalars{};
    for (const unsigned char byte : utf8) {
        if ((byte & 0xc0U) != 0x80U) ++scalars;
    }
    const double tracking = scalars > 1U
        ? static_cast<double>(scalars - 1U) * effective_font.letter_spacing
        : 0.0;
    result.logical_size = {
        std::max(0.0, static_cast<double>(scalars) *
                          effective_font.size * 0.55 + tracking),
        effective_font.size * 1.2};
    result.ascent = effective_font.size;
    result.descent = std::max(0.0, result.logical_size.height - result.ascent);
    result.status = TextResolutionStatus::estimated;
    return result;
}

double snap_text_baseline(double logical_baseline,
                          double device_scale) noexcept {
    if (!std::isfinite(logical_baseline) || !std::isfinite(device_scale) ||
        device_scale <= 0.0) {
        return logical_baseline;
    }
    return std::round(logical_baseline * device_scale) / device_scale;
}

} // namespace gui_forms
