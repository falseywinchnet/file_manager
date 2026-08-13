#pragma once

#include "gui_forms/types/geometry/geometry.hpp"
#include "gui_forms/types/paint_types/paint_types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

// Text resolution is deliberately separate from FontSpec. FontSpec is the
// committed renderer-neutral request; this record describes what a terminal
// text engine actually measured and which bundled faces it selected.
enum class TextResolutionStatus : std::uint8_t {
    exact,
    estimated,
    missing_primary_face,
    missing_cluster_coverage,
    invalid_request,
};

struct ResolvedFontRun final {
    std::size_t utf8_start{};
    std::size_t utf8_length{};
    std::string family;
    std::uint16_t registered_weight{400};
    bool registered_italic{};
    bool fallback{};
    friend bool operator==(const ResolvedFontRun& left,
                           const ResolvedFontRun& right) = default;
};

struct ResolvedTextLayout final {
    FontSpec effective_font{};
    Size logical_size{};
    double ascent{};
    double descent{};
    double line_gap{};
    std::string primary_family;
    std::vector<ResolvedFontRun> runs;
    std::size_t missing_clusters{};
    TextResolutionStatus status{TextResolutionStatus::estimated};

    [[nodiscard]] bool exact() const noexcept {
        return status == TextResolutionStatus::exact;
    }
    [[nodiscard]] double baseline() const noexcept { return ascent; }
    friend bool operator==(const ResolvedTextLayout& left,
                           const ResolvedTextLayout& right) = default;
};

[[nodiscard]] const char* text_resolution_status_name(
    TextResolutionStatus status) noexcept;

// Portable non-owning service seam. Implementations may use a private text
// engine, but this contract contains no HarfBuzz, FreeType, Skia, AppKit, or
// Win32 type. A Window invalidates measurement when the installed provider
// changes.
class TextMetricsProvider {
public:
    virtual ~TextMetricsProvider() = default;
    [[nodiscard]] virtual ResolvedTextLayout resolve_text_layout_utf8(
        std::string_view utf8, FontSpec effective_font) = 0;
};

// Deterministic renderer-free fallback. It is explicitly labelled estimated;
// callers must not report it as a resolved bundled face or a raster metric.
[[nodiscard]] ResolvedTextLayout estimate_text_layout_utf8(
    std::string_view utf8, FontSpec effective_font) noexcept;

// Baselines are authored in logical coordinates. This helper rounds only the
// final placement to a device pixel and therefore does not change line
// measurement or the font identity when a window crosses displays.
[[nodiscard]] double snap_text_baseline(double logical_baseline,
                                        double device_scale) noexcept;

} // namespace gui_forms
