#include "gui_forms/controls/metrics_view/metrics_view.hpp"

#include "gui_forms/window.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {

MetricsView::MetricsView(StableId stable_id, std::string title)
    : Control(std::move(stable_id)), title_(std::move(title)) {
    set_accessible_name(title_);
}

void MetricsView::set_title(std::string title) {
    require_mutable();
    if (title_ == title) return;
    title_ = std::move(title);
    set_accessible_name(title_);
    invalidate(Dirty::paint | Dirty::semantics);
}

void MetricsView::set_style(BasicControlStyle style) {
    require_mutable();
    if (style_ == style) return;
    style_ = style;
    invalidate(Dirty::paint);
}

void MetricsView::set_accent_width(double width) {
    require_mutable();
    if (!std::isfinite(width) || width < 1.0 || width > 32.0) {
        throw std::invalid_argument("metrics accent width must be within [1, 32]");
    }
    if (accent_width_ == width) return;
    accent_width_ = width;
    invalidate(Dirty::paint);
}

std::string MetricsView::metrics_text() const {
    if (window() == nullptr) return "Detached";
    const MetricsSnapshot metrics = window()->metrics_snapshot();
    std::ostringstream text;
    text << metrics.control_count << " controls · "
         << metrics.paint_passes << " paints · "
         << metrics.display_chunks_reused << " chunks reused · "
         << metrics.input_events << " inputs · "
         << metrics.active_surface_count << " active surfaces · "
         << metrics.focus_scope_depth << " focus scopes · damage "
         << static_cast<unsigned long long>(std::round(metrics.painted_damage_area))
         << " px";
    return text.str();
}

void MetricsView::on_paint(Painter& painter, Rect) {
    const Rect bounds{0.0, 0.0, committed_arranged_bounds().width,
                      committed_arranged_bounds().height};
    const double s = effective_text_scale();
    painter.fill_rect(bounds, style_.text);
    painter.fill_rect({0.0, 0.0, accent_width_, bounds.height}, style_.accent);
    painter.draw_text_utf8({16.0 * s, 24.0 * s}, title_,
                           effective_font({FontRole::control, 12.0, 700, false}),
                           style_.face_light);
    const std::string metrics = metrics_text();
    const std::size_t split = metrics.find(" · ", metrics.size() / 2U);
    const std::string first = split == std::string::npos
        ? metrics : metrics.substr(0U, split);
    const std::string second = split == std::string::npos
        ? std::string{} : metrics.substr(split + 3U);
    painter.draw_text_utf8({16.0 * s, 48.0 * s}, first,
                           effective_font({FontRole::content, 11.0, 400, false}),
                           style_.face);
    if (!second.empty()) {
        painter.draw_text_utf8({16.0 * s, 68.0 * s}, second,
                               effective_font({FontRole::content, 11.0, 400, false}),
                               style_.face);
    }
}

bool MetricsView::hit_test_local(Point) const { return false; }

SemanticDescriptor MetricsView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = title_;
    descriptor.value = metrics_text();
    descriptor.exposed = true;
    return descriptor;
}

} // namespace gui_forms
