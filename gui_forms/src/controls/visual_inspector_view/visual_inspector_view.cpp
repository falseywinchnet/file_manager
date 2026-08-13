#include "gui_forms/controls/visual_inspector_view/visual_inspector_view.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace gui_forms {
namespace {

constexpr Color inspector_ink = Color::rgba(225, 234, 241);
constexpr Color inspector_muted = Color::rgba(157, 174, 187);
constexpr Color inspector_blue = Color::rgba(64, 170, 232);
constexpr Color inspector_gold = Color::rgba(247, 194, 72);
constexpr Color inspector_magenta = Color::rgba(238, 102, 184);
constexpr Color inspector_panel = Color::rgba(30, 39, 47);
constexpr Color inspector_panel_light = Color::rgba(44, 55, 64);

std::string rect_text(Rect value) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(1) << value.x << ", " << value.y
           << "  " << value.width << " × " << value.height;
    return output.str();
}

std::string state_text(const VisualControlInspection& target) {
    std::ostringstream output;
    output << (target.state.visible ? "visible" : "hidden") << " · "
           << (target.state.enabled ? "enabled" : "disabled");
    if (target.state.focused) output << " · focused";
    if (target.state.hovered) output << " · hovered";
    if (target.state.pressed) output << " · pressed";
    if (target.state.pointer_captured) output << " · captured";
    output << " · " << visual_status_name(target.state.visual_status);
    return output.str();
}

std::string representative_font_text(const VisualControlInspection& target) {
    const VisualPaintOperationSnapshot* representative = nullptr;
    for (const VisualPaintOperationSnapshot& operation :
         target.display_operations) {
        if (operation.operation != VisualPaintOperation::draw_text) continue;
        if (representative == nullptr ||
            operation.font.size > representative->font.size) {
            representative = &operation;
        }
    }
    if (representative == nullptr) return "no committed text draw";
    const VisualPaintOperationSnapshot& operation = *representative;
    std::ostringstream output;
    output << font_role_name(operation.font.role) << " " << std::fixed
           << std::setprecision(1) << operation.font.size << "/"
           << operation.font.weight;
    if (operation.font.italic) output << "i";
    if (operation.font.letter_spacing != 0.0) {
        output << " +" << operation.font.letter_spacing << "ls";
    }
    if (operation.resolved_text) {
        const ResolvedTextLayout& resolved = *operation.resolved_text;
        output << " · " << text_resolution_status_name(resolved.status);
        if (!resolved.primary_family.empty()) {
            output << " · " << resolved.primary_family;
        }
        output << " · " << resolved.runs.size() << "r";
        if (resolved.missing_clusters != 0U) {
            output << " · !" << resolved.missing_clusters;
        }
    } else {
        output << " · unavailable";
    }
    return output.str();
}

std::string operation_summary(const VisualControlInspection& target) {
    std::size_t clips{};
    std::size_t fills{};
    std::size_t gradients{};
    std::size_t shadows{};
    std::size_t text{};
    for (const VisualPaintOperationSnapshot& operation :
         target.display_operations) {
        switch (operation.operation) {
        case VisualPaintOperation::clip_rect:
        case VisualPaintOperation::clip_rounded_rect: ++clips; break;
        case VisualPaintOperation::fill_rect:
        case VisualPaintOperation::fill_rounded_rect: ++fills; break;
        case VisualPaintOperation::fill_linear_gradient:
        case VisualPaintOperation::fill_linear_gradient_spread:
        case VisualPaintOperation::fill_radial_gradient: ++gradients; break;
        case VisualPaintOperation::draw_box_shadow:
        case VisualPaintOperation::draw_inset_box_shadow: ++shadows; break;
        case VisualPaintOperation::draw_text: ++text; break;
        default: break;
        }
    }
    std::ostringstream output;
    output << target.total_display_operations << " ops · " << clips
           << " clips · " << fills << " fills · " << gradients
           << " gradients · " << shadows << " shadows · " << text << " text";
    return output.str();
}

std::string material_summary(const VisualControlInspection& target) {
    if (!target.authored_material) {
        return "none authored; resolved paint is recorded below";
    }
    std::ostringstream output;
    output << (*target.authored_material).fills.size() << " layers · "
           << (*target.authored_material).shadows.size() << " shadows · "
           << (*target.authored_material).keylines.size() << " keylines · radius "
           << (*target.authored_material).corner_radius;
    return output.str();
}

VisualInspectionOptions view_options() {
    VisualInspectionOptions options;
    options.maximum_controls = 16'384U;
    options.maximum_operations_per_control = 512U;
    options.include_display_operations = true;
    options.include_text = false;
    return options;
}

VisualInspectionOptions overlay_options() {
    VisualInspectionOptions options;
    options.maximum_controls = 16'384U;
    options.include_display_operations = false;
    return options;
}

} // namespace

VisualInspectorView::VisualInspectorView(StableId stable_id,
                                         std::string target_stable_id)
    : Control(std::move(stable_id)),
      target_stable_id_(std::move(target_stable_id)) {
    set_accessible_name("Visual state inspector");
}

void VisualInspectorView::set_target_stable_id(std::string stable_id) {
    require_mutable();
    if (target_stable_id_ == stable_id) return;
    target_stable_id_ = std::move(stable_id);
    invalidate(Dirty::paint | Dirty::semantics | Dirty::accessibility);
}

void VisualInspectorView::refresh() { invalidate(Dirty::paint | Dirty::semantics); }

void VisualInspectorView::on_frame(FrameTime) { refresh(); }

void VisualInspectorView::on_paint(Painter& painter, Rect) {
    const Rect bounds = client_rectangle();
    painter.fill_rect(bounds, inspector_panel);
    painter.fill_rect({0.0, 0.0, bounds.width, 44.0}, inspector_panel_light);
    painter.fill_rect({0.0, 0.0, 5.0, bounds.height}, inspector_blue);

    const FontSpec title = effective_font({FontRole::control, 14.0, 700, false});
    const FontSpec section = effective_font({FontRole::control, 10.0, 700, false});
    const FontSpec body = effective_font({FontRole::monospace, 10.0, 400, false});
    painter.draw_text_utf8({18.0, 27.0}, "VISUAL STATE INSPECTOR", title,
                           inspector_ink);

    Window* owner = window();
    if (owner == nullptr) {
        painter.draw_text_utf8({18.0, 68.0}, "Detached", body, inspector_muted);
        return;
    }
    const VisualInspectionSnapshot snapshot =
        (*owner).visual_inspection_snapshot(view_options());
    const VisualControlInspection* target = snapshot.find(target_stable_id_);
    if (target == nullptr) {
        painter.draw_text_utf8({18.0, 68.0}, "Target not found: " + target_stable_id_,
                               body, inspector_gold);
        return;
    }

    struct Row final { std::string label; std::string value; Color color; };
    const std::vector<Row> rows{
        {"IDENTITY", target->stable_id + "  #" +
             std::to_string(target->runtime_id.value), inspector_ink},
        {"STATE", state_text(*target), inspector_ink},
        {"REQUESTED", rect_text(target->layout.requested_bounds), inspector_muted},
        {"ARRANGED", rect_text(target->layout.arranged_bounds), inspector_blue},
        {"ABSOLUTE", rect_text(target->layout.absolute_bounds), inspector_blue},
        {"VISUAL", rect_text(target->layout.visual_bounds), inspector_magenta},
        {"EFFECTIVE CLIP", rect_text(target->layout.effective_clip), inspector_gold},
        {"VIEWPORT", rect_text(target->layout.child_viewport), inspector_muted},
        {"MATERIAL", material_summary(*target), inspector_ink},
        {"RESOLVED FONT", representative_font_text(*target), inspector_ink},
        {"DISPLAY", operation_summary(*target), inspector_ink},
        {"CHUNK", target->display_chunk
             ? (target->display_chunk_current ? "current generation " :
                                                "STALE generation ") +
                   std::to_string((*target->display_chunk).generation)
             : "not painted", target->display_chunk_current ? inspector_blue :
                                                               inspector_gold},
        {"THEME", target->theme_id +
             (target->theme_override ? " · local override" : " · inherited"),
             inspector_muted},
        {"WINDOW", std::to_string(snapshot.total_controls) + " controls · " +
             std::to_string(snapshot.pending_damage[0].size() +
                            snapshot.pending_damage[1].size() +
                            snapshot.pending_damage[2].size()) +
             " pending damage rects", inspector_muted},
    };

    double y = 64.0;
    for (const Row& row : rows) {
        painter.draw_text_utf8({18.0, y}, row.label, section, inspector_muted);
        painter.draw_text_utf8({145.0, y}, row.value, body, row.color);
        y += 30.0;
        painter.draw_line({18.0, y - 10.0}, {bounds.width - 18.0, y - 10.0},
                          Color::rgba(73, 86, 96), 1.0);
    }
    painter.draw_text_utf8(
        {18.0, bounds.height - 20.0},
        "Blue arranged · magenta visual · gold effective clip",
        effective_font({FontRole::content, 9.0, 400, false}), inspector_muted);
}

std::string VisualInspectorView::accessible_value() const {
    Window* owner = window();
    if (owner == nullptr) return "Detached";
    const VisualInspectionSnapshot snapshot =
        (*owner).visual_inspection_snapshot(view_options());
    const VisualControlInspection* target = snapshot.find(target_stable_id_);
    if (target == nullptr) return "Target not found: " + target_stable_id_;
    return target->stable_id + "; " + state_text(*target) + "; arranged " +
           rect_text(target->layout.arranged_bounds) + "; effective clip " +
           rect_text(target->layout.effective_clip) + "; " +
           operation_summary(*target);
}

SemanticDescriptor VisualInspectorView::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.role = SemanticRole::group;
    descriptor.name = "Visual state inspector";
    descriptor.value = accessible_value();
    descriptor.exposed = true;
    return descriptor;
}

VisualInspectorOverlay::VisualInspectorOverlay(StableId stable_id,
                                               std::string target_stable_id)
    : Control(std::move(stable_id)),
      target_stable_id_(std::move(target_stable_id)) {
    set_hit_test_transparent(true);
    set_paint_plane(PaintPlane::overlay);
}

void VisualInspectorOverlay::set_target_stable_id(std::string stable_id) {
    require_mutable();
    if (target_stable_id_ == stable_id) return;
    target_stable_id_ = std::move(stable_id);
    invalidate(Dirty::paint);
}

void VisualInspectorOverlay::refresh() { invalidate(Dirty::paint); }

void VisualInspectorOverlay::on_frame(FrameTime) { refresh(); }

void VisualInspectorOverlay::on_paint(Painter& painter, Rect) {
    Window* owner = window();
    if (owner == nullptr || target_stable_id_.empty()) return;
    const VisualInspectionSnapshot snapshot =
        (*owner).visual_inspection_snapshot(overlay_options());
    const VisualControlInspection* target = snapshot.find(target_stable_id_);
    if (target == nullptr || target->runtime_id == runtime_id()) return;

    Rect arranged = rectangle_from_window(target->layout.absolute_bounds);
    Rect visual = rectangle_from_window(target->layout.visual_bounds);
    Rect clip = rectangle_from_window(target->layout.effective_clip);
    painter.stroke_rect(visual, inspector_magenta, 2.0);
    painter.stroke_rect(arranged, inspector_blue, 2.0);
    if (!clip.empty()) painter.stroke_rect(clip, inspector_gold, 1.0);
}

bool VisualInspectorOverlay::hit_test_local(Point) const { return false; }

SemanticDescriptor VisualInspectorOverlay::semantic_descriptor() const {
    SemanticDescriptor descriptor;
    descriptor.exposed = false;
    return descriptor;
}

} // namespace gui_forms
