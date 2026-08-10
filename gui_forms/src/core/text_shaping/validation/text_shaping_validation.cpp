#include "gui_forms/text_shaping.hpp"

#include <algorithm>
#include <cmath>

namespace gui_forms {
namespace {

[[nodiscard]] bool continuation(std::uint8_t byte) noexcept {
  return (byte & 0xc0U) == 0x80U;
}

[[nodiscard]] bool scalar_boundary(std::string_view text,
                                   Utf8Offset position) noexcept {
  return position.value() <= text.size() &&
         (position.value() == text.size() ||
          !continuation(static_cast<std::uint8_t>(text[position.value()])));
}

[[nodiscard]] bool finite(const GlyphPlacement &placement) noexcept {
  return std::isfinite(placement.advance_x) &&
         std::isfinite(placement.advance_y) &&
         std::isfinite(placement.offset_x) &&
         std::isfinite(placement.offset_y);
}

} // namespace

ShapingRequestError
validate_shaping_request(const ShapingRequest &request) {
  if (!validate_utf8(request.utf8_text).valid()) {
    return ShapingRequestError::invalid_utf8;
  }
  if (!request.face.valid()) {
    return ShapingRequestError::invalid_face;
  }
  if (!std::isfinite(request.font_size) || request.font_size <= 0.0F) {
    return ShapingRequestError::invalid_font_size;
  }
  if (request.range.start > request.range.end) {
    return ShapingRequestError::reversed_range;
  }
  if (request.range.end.value() > request.utf8_text.size()) {
    return ShapingRequestError::range_out_of_bounds;
  }
  if (!scalar_boundary(request.utf8_text, request.range.start) ||
      !scalar_boundary(request.utf8_text, request.range.end)) {
    return ShapingRequestError::range_splits_scalar;
  }
  const TextStore text(request.utf8_text,
                       {.maximum_utf8_bytes = request.utf8_text.size(),
                        .maximum_style_spans = 0});
  if (!text.is_grapheme_boundary(request.range.start) ||
      !text.is_grapheme_boundary(request.range.end)) {
    return ShapingRequestError::range_splits_grapheme;
  }
  for (const ShapingFeature &feature : request.features) {
    if (!feature.tag.valid()) {
      return ShapingRequestError::invalid_feature_tag;
    }
    if (feature.range.start > feature.range.end ||
        feature.range.start < request.range.start ||
        feature.range.end > request.range.end) {
      return ShapingRequestError::feature_outside_range;
    }
    if (!scalar_boundary(request.utf8_text, feature.range.start) ||
        !scalar_boundary(request.utf8_text, feature.range.end)) {
      return ShapingRequestError::feature_splits_scalar;
    }
  }
  return ShapingRequestError::none;
}

GlyphRunError validate_glyph_run(const ShapingRequest &request,
                                 const GlyphRun &run) {
  if (validate_shaping_request(request) != ShapingRequestError::none) {
    return GlyphRunError::invalid_request;
  }
  if (run.source_range != request.range) {
    return GlyphRunError::mismatched_source_range;
  }
  if (run.face != request.face) {
    return GlyphRunError::mismatched_face;
  }
  if (run.direction != request.direction) {
    return GlyphRunError::mismatched_direction;
  }

  const TextStore text(request.utf8_text,
                       {.maximum_utf8_bytes = request.utf8_text.size(),
                        .maximum_style_spans = 0});
  std::optional<Utf8Offset> previous;
  for (const GlyphPlacement &glyph : run.glyphs) {
    if (glyph.cluster < request.range.start ||
        glyph.cluster >= request.range.end) {
      return GlyphRunError::cluster_out_of_range;
    }
    if (!text.is_grapheme_boundary(glyph.cluster)) {
      return GlyphRunError::cluster_splits_grapheme;
    }
    if (previous) {
      const bool ordered = request.direction == TextDirection::left_to_right
                               ? *previous <= glyph.cluster
                               : *previous >= glyph.cluster;
      if (!ordered) {
        return GlyphRunError::nonmonotonic_clusters;
      }
    }
    if (!finite(glyph)) {
      return GlyphRunError::nonfinite_placement;
    }
    previous = glyph.cluster;
  }
  return GlyphRunError::none;
}

std::string_view to_string(ShapingRequestError error) noexcept {
  switch (error) {
  case ShapingRequestError::none:
    return "none";
  case ShapingRequestError::invalid_utf8:
    return "invalid_utf8";
  case ShapingRequestError::invalid_face:
    return "invalid_face";
  case ShapingRequestError::invalid_font_size:
    return "invalid_font_size";
  case ShapingRequestError::reversed_range:
    return "reversed_range";
  case ShapingRequestError::range_out_of_bounds:
    return "range_out_of_bounds";
  case ShapingRequestError::range_splits_scalar:
    return "range_splits_scalar";
  case ShapingRequestError::range_splits_grapheme:
    return "range_splits_grapheme";
  case ShapingRequestError::invalid_feature_tag:
    return "invalid_feature_tag";
  case ShapingRequestError::feature_outside_range:
    return "feature_outside_range";
  case ShapingRequestError::feature_splits_scalar:
    return "feature_splits_scalar";
  }
  return "unknown";
}

std::string_view to_string(GlyphRunError error) noexcept {
  switch (error) {
  case GlyphRunError::none:
    return "none";
  case GlyphRunError::invalid_request:
    return "invalid_request";
  case GlyphRunError::mismatched_source_range:
    return "mismatched_source_range";
  case GlyphRunError::mismatched_face:
    return "mismatched_face";
  case GlyphRunError::mismatched_direction:
    return "mismatched_direction";
  case GlyphRunError::cluster_out_of_range:
    return "cluster_out_of_range";
  case GlyphRunError::cluster_splits_grapheme:
    return "cluster_splits_grapheme";
  case GlyphRunError::nonmonotonic_clusters:
    return "nonmonotonic_clusters";
  case GlyphRunError::nonfinite_placement:
    return "nonfinite_placement";
  }
  return "unknown";
}

} // namespace gui_forms
