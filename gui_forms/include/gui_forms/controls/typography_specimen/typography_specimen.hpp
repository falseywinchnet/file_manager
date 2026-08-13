#pragma once

#include "gui_forms/control.hpp"

#include <string>

namespace gui_forms {

// Renderer-neutral diagnostic control for typography review. It draws one
// committed text run with its logical line box, resolved baseline, and exact
// provider status. It is a review instrument, not a File Manager control.
class TypographySpecimen final : public Control {
public:
    explicit TypographySpecimen(StableId stable_id, std::string text = {});

    [[nodiscard]] const std::string& text() const noexcept { return text_; }
    void set_text(std::string text);
    [[nodiscard]] FontSpec font() const noexcept { return font_; }
    void set_font(FontSpec font);
    [[nodiscard]] const std::string& caption() const noexcept { return caption_; }
    void set_caption(std::string caption);
    [[nodiscard]] bool diagnostics_visible() const noexcept {
        return diagnostics_visible_;
    }
    void set_diagnostics_visible(bool visible);
    [[nodiscard]] bool wrap_text() const noexcept { return wrap_text_; }
    void set_wrap_text(bool wrap);
    [[nodiscard]] ResolvedTextLayout resolved_layout() const;

    [[nodiscard]] Size measure(Size available) override;
    void on_paint(Painter& painter, Rect local_damage) override;
    [[nodiscard]] bool hit_test_local(Point local_point) const override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    std::string text_;
    std::string caption_;
    FontSpec font_{FontRole::content, 12.0, 400, false};
    bool diagnostics_visible_{true};
    bool wrap_text_{};
};

} // namespace gui_forms
