#include "gui_forms/controls/typography_specimen/typography_specimen.hpp"

#include "gui_forms/visual_inspection.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace gui_forms {
namespace {

template <typename Resolver>
std::vector<std::string> wrap_lines(std::string_view text, double width,
                                    bool wrap, Resolver&& resolve) {
    if (!wrap || width <= 8.0) return {std::string(text)};
    std::vector<std::string> lines;
    std::string line;
    std::size_t cursor{};
    while (cursor < text.size()) {
        while (cursor < text.size() &&
               std::isspace(static_cast<unsigned char>(text[cursor])) != 0) {
            ++cursor;
        }
        if (cursor >= text.size()) break;
        std::size_t end = cursor;
        while (end < text.size() &&
               std::isspace(static_cast<unsigned char>(text[end])) == 0) {
            ++end;
        }
        const std::string_view word = text.substr(cursor, end - cursor);
        std::string candidate = line;
        if (!candidate.empty()) candidate.push_back(' ');
        candidate.append(word);
        if (!line.empty() && resolve(candidate).logical_size.width > width) {
            lines.push_back(std::move(line));
            line.assign(word);
        } else {
            line = std::move(candidate);
        }
        cursor = end;
    }
    if (!line.empty()) lines.push_back(std::move(line));
    if (lines.empty()) lines.emplace_back();
    return lines;
}

} // namespace

TypographySpecimen::TypographySpecimen(StableId stable_id, std::string text)
    : Control(std::move(stable_id)), text_(std::move(text)) {}

void TypographySpecimen::set_text(std::string text) {
    require_mutable();
    if (text_ == text) return;
    text_ = std::move(text);
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics |
               Dirty::accessibility);
}

void TypographySpecimen::set_font(FontSpec font) {
    require_mutable();
    if (!valid_font_spec(font)) {
        throw std::invalid_argument("TypographySpecimen font is invalid");
    }
    if (font_ == font) return;
    font_ = font;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TypographySpecimen::set_caption(std::string caption) {
    require_mutable();
    if (caption_ == caption) return;
    caption_ = std::move(caption);
    invalidate(Dirty::paint | Dirty::semantics);
}

void TypographySpecimen::set_diagnostics_visible(bool visible) {
    require_mutable();
    if (diagnostics_visible_ == visible) return;
    diagnostics_visible_ = visible;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

void TypographySpecimen::set_wrap_text(bool wrap) {
    require_mutable();
    if (wrap_text_ == wrap) return;
    wrap_text_ = wrap;
    invalidate(Dirty::measure | Dirty::paint | Dirty::semantics);
}

ResolvedTextLayout TypographySpecimen::resolved_layout() const {
    return resolve_text_layout_utf8(text_, effective_font(font_));
}

Size TypographySpecimen::measure(Size available) {
    const FontSpec effective = effective_font(font_);
    const auto resolve = [this, effective](std::string_view line) {
        return resolve_text_layout_utf8(line, effective);
    };
    const std::vector<std::string> lines = wrap_lines(
        text_, std::max(0.0, available.width - 20.0), wrap_text_, resolve);
    double content_width{};
    double content_height{};
    for (const std::string& line : lines) {
        const ResolvedTextLayout resolved = resolve(line);
        content_width = std::max(content_width, resolved.logical_size.width);
        content_height += std::max(effective.size, resolved.logical_size.height);
    }
    const double scale = effective_text_scale();
    const double caption_height = caption_.empty() ? 0.0 : 8.0 * scale * 1.2 + 6.0;
    const double diagnostics = diagnostics_visible_
        ? 8.0 * scale * 1.2 + 8.0 : 0.0;
    return {std::min(available.width,
                     std::max(160.0, content_width + 24.0)),
            std::min(available.height,
                     std::max(28.0, content_height + caption_height +
                                        diagnostics + 8.0))};
}

void TypographySpecimen::on_paint(Painter& painter, Rect) {
    const Rect bounds = client_rectangle();
    const FontSpec effective = effective_font(font_);
    const auto resolve = [&painter, effective](std::string_view line) {
        return painter.resolve_text_layout_utf8(line, effective);
    };
    const std::vector<std::string> lines = wrap_lines(
        text_, std::max(0.0, bounds.width - 20.0), wrap_text_, resolve);
    std::vector<ResolvedTextLayout> resolved_lines;
    resolved_lines.reserve(lines.size());
    for (const std::string& line : lines) resolved_lines.push_back(resolve(line));
    const ResolvedTextLayout& resolved = resolved_lines.front();
    const Color ink = Color::rgba(27, 43, 57);
    const Color muted = Color::rgba(83, 103, 118);
    const Color baseline_ink = Color::rgba(64, 145, 196, 145);
    const Color fault = Color::rgba(157, 63, 55);
    const FontSpec caption_font =
        effective_font({FontRole::control, 8.0, 700, false, 0.24});

    painter.fill_rect(bounds, Color::rgba(249, 251, 252));
    painter.stroke_rect({0.0, 0.0, std::max(0.0, bounds.width - 1.0),
                         std::max(0.0, bounds.height - 1.0)},
                        Color::rgba(172, 184, 191), 1.0);
    if (!caption_.empty()) {
        painter.draw_text_utf8(
            {10.0, snap_text_baseline(caption_font.size + 3.0)}, caption_,
            caption_font, muted);
    }
    double logical_baseline = caption_font.size + 8.0 + resolved.ascent;
    double baseline = snap_text_baseline(logical_baseline);
    for (std::size_t index = 0U; index < lines.size(); ++index) {
        if (index != 0U) {
            logical_baseline += std::max(
                effective.size, resolved_lines[index - 1U].logical_size.height);
            baseline = snap_text_baseline(logical_baseline);
        }
        painter.draw_line({8.0, baseline}, {bounds.width - 8.0, baseline},
                          baseline_ink, 1.0);
        painter.draw_text_utf8(
            {10.0, baseline}, lines[index], effective,
            resolved_lines[index].status == TextResolutionStatus::exact
                ? ink : fault);
    }

    if (diagnostics_visible_) {
        std::ostringstream details;
        details << text_resolution_status_name(resolved.status) << ' ';
        if (resolved.primary_family.empty()) details << "no resolved face";
        else details << resolved.primary_family;
        details << ' ' << resolved.runs.size() << "r";
        if (resolved.missing_clusters != 0U) {
            details << " !" << resolved.missing_clusters;
        }
        details << " A" << std::fixed << std::setprecision(0)
                << resolved.ascent << "/D"
                << resolved.descent << " B" << std::setprecision(1)
                << logical_baseline << ">" << baseline
                << " @" << (window() != nullptr ? (*window()).scale() : 1.0)
                << "×";
        painter.draw_text_utf8(
            {10.0, snap_text_baseline(bounds.height - 7.0)}, details.str(),
            effective_font({FontRole::monospace, 8.0, 400, false}), muted);
    }
}

bool TypographySpecimen::hit_test_local(Point) const { return false; }

SemanticDescriptor TypographySpecimen::semantic_descriptor() const {
    const ResolvedTextLayout resolved = resolved_layout();
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::static_text;
    descriptor.name = caption_.empty() ? text_ : caption_ + ": " + text_;
    descriptor.value = std::string(text_resolution_status_name(resolved.status)) +
        "; " + (resolved.primary_family.empty() ? "no resolved face"
                                                  : resolved.primary_family);
    descriptor.exposed = true;
    return descriptor;
}

} // namespace gui_forms
