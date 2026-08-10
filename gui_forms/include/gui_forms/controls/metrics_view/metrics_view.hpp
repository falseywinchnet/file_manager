#pragma once

#include "gui_forms/basic_controls.hpp"

#include <string>

namespace gui_forms {

// Renderer-neutral view over the structured Window metrics snapshot.
class MetricsView final : public Control {
public:
    explicit MetricsView(StableId stable_id,
                         std::string title = "Runtime metrics");

    [[nodiscard]] const std::string& title() const noexcept { return title_; }
    void set_title(std::string title);
    [[nodiscard]] const BasicControlStyle& style() const noexcept {
        return style_;
    }
    void set_style(BasicControlStyle style);
    [[nodiscard]] double accent_width() const noexcept { return accent_width_; }
    void set_accent_width(double width);

    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    [[nodiscard]] std::string metrics_text() const;
    std::string title_;
    BasicControlStyle style_;
    double accent_width_{5.0};
};

} // namespace gui_forms
